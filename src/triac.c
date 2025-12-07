#include <avr/io.h>
#include <avr/interrupt.h>
#include "triac.h"
#include "config.h"

/* Triac power levels (0-100%) */
static volatile uint8_t triac1_power = 0;
static volatile uint8_t triac2_power = 0;

/* State machine for triac control */
static volatile uint8_t triac_state = 0;

void triac_init(void) {
    /* Configure triac control pins as outputs */
    PORTB.DIRSET = (1 << TRIAC_1_PIN) | (1 << TRIAC_2_PIN);
    PORTB.OUTCLR = (1 << TRIAC_1_PIN) | (1 << TRIAC_2_PIN);
    
    /* Configure zero-cross detection pin as input with pull-up */
    PORTD.DIRCLR = (1 << ZERO_CROSS_PIN);
    PORTD.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
    
    /* Enable Timer/Counter B (TCB0) for phase angle control */
    /* TCB0 will be used for precise timing after zero-cross */
    TCB0.CTRLA = TCB_CLKSEL_DIV2_gc;  /* CLK_PER/2 = 10 MHz */
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;  /* Periodic interrupt mode */
    TCB0.INTCTRL = TCB_CAPT_bm;       /* Enable capture interrupt */
    
    /* Enable interrupts for zero-cross detection */
    PORTD.INTFLAGS = PORT_INT4_bm;  /* Clear any pending interrupt */
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
        
        /* Set timer for pulse width (50us) */
        TCB0.CCMP = 500;  /* 50us at 10 MHz */
        triac_state = 2;
        
    } else if (triac_state == 2) {
        /* State 2: End triac 1 pulse */
        PORTB.OUTCLR = (1 << TRIAC_1_PIN);
        
        /* Calculate delay for triac 2 */
        uint32_t delay_us = AC_HALF_PERIOD_US - 
                           ((uint32_t)triac2_power * (AC_HALF_PERIOD_US - 500)) / 100;
        
        /* Set timer for triac 2 phase angle from start of cycle */
        /* We need to calculate time from now to triac 2 firing */
        /* For simplicity, fire triac 2 slightly after triac 1 */
        TCB0.CCMP = 100;  /* Small delay before checking triac 2 */
        triac_state = 3;
        
    } else if (triac_state == 3) {
        /* State 3: Fire triac 2 */
        if (triac2_power > 0) {
            PORTB.OUTSET = (1 << TRIAC_2_PIN);
        }
        
        /* Set timer for pulse width (50us) */
        TCB0.CCMP = 500;  /* 50us at 10 MHz */
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
        /* Phase angle: 0% = full delay (~8333us), 100% = min delay (~500us) */
        uint32_t delay_us = AC_HALF_PERIOD_US - 
                           ((uint32_t)triac1_power * (AC_HALF_PERIOD_US - 500)) / 100;
        
        /* Convert to timer ticks (10 MHz = 0.1us per tick) */
        TCB0.CCMP = delay_us * 10;
        
        /* Enable timer */
        TCB0.CTRLA |= TCB_ENABLE_bm;
    }
}

