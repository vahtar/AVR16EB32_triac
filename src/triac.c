#include <avr/io.h>
#include <avr/interrupt.h>
#include "triac.h"
#include "config.h"

/* Triac power levels (0-100%) - 3-phase motor control */
static volatile uint8_t triac1_power = 0;
static volatile uint8_t triac2_power = 0;
static volatile uint8_t triac3_power = 0;

/* Pre-calculated phase angle delays in timer ticks */
static volatile uint16_t triac1_delay_ticks = 0;
static volatile uint16_t triac2_delay_ticks = 0;
static volatile uint16_t triac3_delay_ticks = 0;

/* State machine for triac control */
static volatile uint8_t triac_state = 0;

void triac_init(void) {
    /* Configure triac control pins as outputs (3 phases) */
    PORTB.DIRSET = (1 << TRIAC_1_PIN) | (1 << TRIAC_2_PIN) | (1 << TRIAC_3_PIN);
    PORTB.OUTCLR = (1 << TRIAC_1_PIN) | (1 << TRIAC_2_PIN) | (1 << TRIAC_3_PIN);
    
    /* Configure zero-cross detection pin as input with pull-up 
     * Compatible with:
     * - MID400 AC input optocoupler (recommended for phase-to-phase detection)
     * - Standard optocouplers (4N25, H11A1) with bridge rectifier circuit
     * 
     * MID400 advantages:
     * - Direct phase-to-phase sensing (ideal for 3-phase systems)
     * - Built-in zero-cross detection
     * - Better noise immunity
     * - Simplified circuit
     */
    PORTD.DIRCLR = (1 << ZERO_CROSS_PIN);
    /* Use macro to configure the correct pin's control register */
    #if ZERO_CROSS_PIN == 4
        PORTD.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
    #else
        #error "ZERO_CROSS_PIN must be on PORTD pin 4 for this implementation"
    #endif
    
    /* Enable Timer/Counter B (TCB0) for phase angle control */
    /* TCB0 will be used for precise timing after zero-cross */
    TCB0.CTRLA = TCB_CLKSEL_DIV2_gc;  /* CLK_PER/2 = 10 MHz */
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;  /* Periodic interrupt mode */
    TCB0.INTCTRL = TCB_CAPT_bm;       /* Enable capture interrupt */
}

void triac_set_power(uint8_t triac_num, uint8_t power_level) {
    /* Limit power level to 0-100 */
    if (power_level > MAX_POWER_LEVEL) {
        power_level = MAX_POWER_LEVEL;
    }
    
    /* Calculate phase angle delay in microseconds */
    uint32_t delay_us = AC_HALF_PERIOD_US - 
                       ((uint32_t)power_level * (AC_HALF_PERIOD_US - MIN_FIRING_DELAY_US)) / 100;
    
    /* Convert to timer ticks */
    uint16_t delay_ticks = delay_us * TIMER_TICKS_PER_US;
    
    /* Update appropriate triac with atomic operations */
    if (triac_num == 0) {
        triac1_power = power_level;
        triac1_delay_ticks = delay_ticks;
    } else if (triac_num == 1) {
        triac2_power = power_level;
        triac2_delay_ticks = delay_ticks;
    } else if (triac_num == 2) {
        triac3_power = power_level;
        triac3_delay_ticks = delay_ticks;
    }
}

void triac_update_from_joystick(uint16_t joystick_value) {
    uint8_t power = 0;
    
    /* Convert joystick position to power level */
    /* Center is 512, deadzone of 50 around center */
    if (joystick_value > (JOYSTICK_CENTER + JOYSTICK_DEADZONE)) {
        /* Forward direction - proportional power for 3-phase motor */
        power = ((uint32_t)(joystick_value - JOYSTICK_CENTER - JOYSTICK_DEADZONE) * 100) / 
                 (1023 - JOYSTICK_CENTER - JOYSTICK_DEADZONE);
    } else if (joystick_value < (JOYSTICK_CENTER - JOYSTICK_DEADZONE)) {
        /* Reverse direction - proportional power */
        power = ((uint32_t)(JOYSTICK_CENTER - JOYSTICK_DEADZONE - joystick_value) * 100) / 
                 (JOYSTICK_CENTER - JOYSTICK_DEADZONE);
    }
    
    /* Update all three triacs with same power level for synchronized 3-phase control */
    triac_set_power(0, power);
    triac_set_power(1, power);
    triac_set_power(2, power);
}

void triac_emergency_stop(void) {
    /* Turn off all three triacs immediately */
    PORTB.OUTCLR = (1 << TRIAC_1_PIN) | (1 << TRIAC_2_PIN) | (1 << TRIAC_3_PIN);
    triac1_power = 0;
    triac2_power = 0;
    triac3_power = 0;
    TCB0.CTRLA &= ~TCB_ENABLE_bm;  /* Stop timer */
}

/* Timer capture ISR - fires at calculated phase angle delay */
ISR(TCB0_INT_vect) {
    /* Clear interrupt flag */
    TCB0.INTFLAGS = TCB_CAPT_bm;
    
    if (triac_state == 1) {
        /* State 1: Fire triac 1 (Phase L1) */
        if (triac1_power > 0) {
            PORTB.OUTSET = (1 << TRIAC_1_PIN);
        }
        
        /* Set timer for pulse width */
        TCB0.CCMP = TRIAC_PULSE_WIDTH_US * TIMER_TICKS_PER_US;
        triac_state = 2;
        
    } else if (triac_state == 2) {
        /* State 2: End triac 1 pulse */
        PORTB.OUTCLR = (1 << TRIAC_1_PIN);
        
        /* Set timer to fire triac 2 at its calculated phase angle */
        /* Reset counter first, then update compare for predictable timing */
        TCB0.CNT = 0;
        TCB0.CCMP = triac2_delay_ticks;
        triac_state = 3;
        
    } else if (triac_state == 3) {
        /* State 3: Fire triac 2 (Phase L2) */
        if (triac2_power > 0) {
            PORTB.OUTSET = (1 << TRIAC_2_PIN);
        }
        
        /* Set timer for pulse width */
        TCB0.CCMP = TRIAC_PULSE_WIDTH_US * TIMER_TICKS_PER_US;
        triac_state = 4;
        
    } else if (triac_state == 4) {
        /* State 4: End triac 2 pulse */
        PORTB.OUTCLR = (1 << TRIAC_2_PIN);
        
        /* Set timer to fire triac 3 at its calculated phase angle */
        TCB0.CNT = 0;
        TCB0.CCMP = triac3_delay_ticks;
        triac_state = 5;
        
    } else if (triac_state == 5) {
        /* State 5: Fire triac 3 (Phase L3) */
        if (triac3_power > 0) {
            PORTB.OUTSET = (1 << TRIAC_3_PIN);
        }
        
        /* Set timer for pulse width */
        TCB0.CCMP = TRIAC_PULSE_WIDTH_US * TIMER_TICKS_PER_US;
        triac_state = 6;
        
    } else if (triac_state == 6) {
        /* State 6: End triac 3 pulse */
        PORTB.OUTCLR = (1 << TRIAC_3_PIN);
        
        /* Stop timer until next zero-cross */
        TCB0.CTRLA &= ~TCB_ENABLE_bm;
        triac_state = 0;
    }
}

/* Zero-cross detection ISR */
ISR(PORTD_PORT_vect) {
    /* Check if it's the zero-cross pin */
    if (PORTD.INTFLAGS & PORT_INT4_bm) {
        /* Clear interrupt flag */
        PORTD.INTFLAGS = PORT_INT4_bm;
        
        /* Reset timer and state machine */
        TCB0.CNT = 0;
        triac_state = 1;
        
        /* Use pre-calculated delay values from triac_set_power() */
        TCB0.CCMP = triac1_delay_ticks;
        
        /* Enable timer */
        TCB0.CTRLA |= TCB_ENABLE_bm;
    }
}

