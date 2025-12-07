#include <avr/io.h>
#include <avr/interrupt.h>
#include "triac.h"
#include "config.h"

/* Triac power levels (0-100%) */
static volatile uint8_t triac1_power = 0;
static volatile uint8_t triac2_power = 0;

/* State machine for triac control */
static volatile uint8_t triac_state = 0;

/* Store calculated delays for triac 2 */
static volatile uint16_t triac2_delay_ticks = 0;

void triac_init(void) {
    /* Configure triac control pins as outputs */
    PORTB.DIRSET = (1 << TRIAC_1_PIN) | (1 << TRIAC_2_PIN);
    PORTB.OUTCLR = (1 << TRIAC_1_PIN) | (1 << TRIAC_2_PIN);
    
    /* Configure zero-cross detection pin as input with pull-up */
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
    
    /* Update appropriate triac */
    if (triac_num == 0) {
        triac1_power = power_level;
    } else if (triac_num == 1) {
        triac2_power = power_level;
    }
}

void triac_update_from_joystick(uint16_t x_value, uint16_t y_value) {
    uint8_t power1 = 0;
    uint8_t power2 = 0;
    
    /* Convert X-axis to power level for Triac 1 */
    /* Center is 512, deadzone of 50 around center */
    if (x_value > (JOYSTICK_CENTER + JOYSTICK_DEADZONE)) {
        /* Right direction - proportional power */
        power1 = ((uint32_t)(x_value - JOYSTICK_CENTER - JOYSTICK_DEADZONE) * 100) / 
                 (1023 - JOYSTICK_CENTER - JOYSTICK_DEADZONE);
    } else if (x_value < (JOYSTICK_CENTER - JOYSTICK_DEADZONE)) {
        /* Left direction - could control differently or same triac in reverse */
        power1 = ((uint32_t)(JOYSTICK_CENTER - JOYSTICK_DEADZONE - x_value) * 100) / 
                 (JOYSTICK_CENTER - JOYSTICK_DEADZONE);
    }
    
    /* Convert Y-axis to power level for Triac 2 */
    if (y_value > (JOYSTICK_CENTER + JOYSTICK_DEADZONE)) {
        /* Up direction - proportional power */
        power2 = ((uint32_t)(y_value - JOYSTICK_CENTER - JOYSTICK_DEADZONE) * 100) / 
                 (1023 - JOYSTICK_CENTER - JOYSTICK_DEADZONE);
    } else if (y_value < (JOYSTICK_CENTER - JOYSTICK_DEADZONE)) {
        /* Down direction */
        power2 = ((uint32_t)(JOYSTICK_CENTER - JOYSTICK_DEADZONE - y_value) * 100) / 
                 (JOYSTICK_CENTER - JOYSTICK_DEADZONE);
    }
    
    /* Update triacs */
    triac_set_power(0, power1);
    triac_set_power(1, power2);
}

void triac_emergency_stop(void) {
    /* Turn off both triacs immediately */
    PORTB.OUTCLR = (1 << TRIAC_1_PIN) | (1 << TRIAC_2_PIN);
    triac1_power = 0;
    triac2_power = 0;
    TCB0.CTRLA &= ~TCB_ENABLE_bm;  /* Stop timer */
}

/* Timer capture ISR - fires at calculated phase angle delay */
ISR(TCB0_INT_vect) {
    /* Clear interrupt flag */
    TCB0.INTFLAGS = TCB_CAPT_bm;
    
    if (triac_state == 1) {
        /* State 1: Fire triac 1 */
        if (triac1_power > 0) {
            PORTB.OUTSET = (1 << TRIAC_1_PIN);
        }
        
        /* Set timer for pulse width */
        TCB0.CCMP = TRIAC_PULSE_WIDTH_US * 10;  /* Convert to 10MHz timer ticks */
        triac_state = 2;
        
    } else if (triac_state == 2) {
        /* State 2: End triac 1 pulse */
        PORTB.OUTCLR = (1 << TRIAC_1_PIN);
        
        /* Set timer to fire triac 2 at its calculated phase angle */
        /* Update compare value then reset counter for clean timing */
        TCB0.CCMP = triac2_delay_ticks;
        TCB0.CNT = 0;  /* Reset counter after updating compare */
        triac_state = 3;
        
    } else if (triac_state == 3) {
        /* State 3: Fire triac 2 */
        if (triac2_power > 0) {
            PORTB.OUTSET = (1 << TRIAC_2_PIN);
        }
        
        /* Set timer for pulse width */
        TCB0.CCMP = TRIAC_PULSE_WIDTH_US * 10;  /* Convert to 10MHz timer ticks */
        triac_state = 4;
        
    } else if (triac_state == 4) {
        /* State 4: End triac 2 pulse */
        PORTB.OUTCLR = (1 << TRIAC_2_PIN);
        
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
        
        /* Calculate phase angle delay for triac 1 */
        /* Phase angle: 0% power = full delay, 100% power = minimum delay */
        uint32_t delay_us = AC_HALF_PERIOD_US - 
                           ((uint32_t)triac1_power * (AC_HALF_PERIOD_US - MIN_FIRING_DELAY_US)) / 100;
        
        /* Convert to timer ticks (10 MHz = 0.1us per tick) */
        TCB0.CCMP = delay_us * 10;
        
        /* Calculate phase angle delay for triac 2 and store for later */
        uint32_t delay2_us = AC_HALF_PERIOD_US - 
                            ((uint32_t)triac2_power * (AC_HALF_PERIOD_US - MIN_FIRING_DELAY_US)) / 100;
        triac2_delay_ticks = delay2_us * 10;
        
        /* Enable timer */
        TCB0.CTRLA |= TCB_ENABLE_bm;
    }
}

