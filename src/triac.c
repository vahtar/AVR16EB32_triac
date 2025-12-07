#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "triac.h"
#include "config.h"

/* Triac power levels (0-100%) */
static volatile uint8_t triac1_power = 0;
static volatile uint8_t triac2_power = 0;

/* Zero-cross detection flag */
static volatile uint8_t zero_cross_detected = 0;

void triac_init(void) {
    /* Configure triac control pins as outputs */
    PORTB.DIRSET = (1 << TRIAC_1_PIN) | (1 << TRIAC_2_PIN);
    PORTB.OUTCLR = (1 << TRIAC_1_PIN) | (1 << TRIAC_2_PIN);
    
    /* Configure zero-cross detection pin as input with pull-up */
    PORTD.DIRCLR = (1 << ZERO_CROSS_PIN);
    PORTD.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
    
    /* Enable Timer/Counter A (TCA0) for phase angle control */
    /* Set up for periodic interrupt at AC frequency */
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc;  /* CLK_PER/64 */
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;         /* Enable overflow interrupt */
    
    /* Calculate period for zero-cross timing */
    /* At 20 MHz with DIV64: 20000000/64 = 312500 Hz */
    /* For 60 Hz AC half-period (8333 us): 312500 * 0.008333 = 2604 */
    TCA0.SINGLE.PER = 2604;
    
    /* Enable interrupts for zero-cross detection */
    PORTD.INTFLAGS = PORT_INT4_bm;  /* Clear any pending interrupt */
    
    /* Start timer */
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
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
}

/* Timer overflow ISR - triggers triac based on phase angle */
ISR(TCA0_OVF_vect) {
    /* Clear interrupt flag */
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
    
    /* Calculate delay based on power level (0-100%) */
    /* Phase angle: 0% = full delay (8333us), 100% = min delay (~500us) */
    
    /* Triac 1 control */
    if (triac1_power > 0) {
        uint16_t delay_us = AC_HALF_PERIOD_US - 
                           ((uint32_t)triac1_power * (AC_HALF_PERIOD_US - 500)) / 100;
        
        /* Wait for calculated phase angle */
        for (uint16_t i = 0; i < delay_us / 10; i++) {
            _delay_us(10);
        }
        
        /* Trigger triac with short pulse (10-100 us) */
        PORTB.OUTSET = (1 << TRIAC_1_PIN);
        _delay_us(50);
        PORTB.OUTCLR = (1 << TRIAC_1_PIN);
    }
    
    /* Triac 2 control */
    if (triac2_power > 0) {
        uint16_t delay_us = AC_HALF_PERIOD_US - 
                           ((uint32_t)triac2_power * (AC_HALF_PERIOD_US - 500)) / 100;
        
        /* Wait for calculated phase angle */
        for (uint16_t i = 0; i < delay_us / 10; i++) {
            _delay_us(10);
        }
        
        /* Trigger triac with short pulse */
        PORTB.OUTSET = (1 << TRIAC_2_PIN);
        _delay_us(50);
        PORTB.OUTCLR = (1 << TRIAC_2_PIN);
    }
}

/* Zero-cross detection ISR */
ISR(PORTD_PORT_vect) {
    /* Check if it's the zero-cross pin */
    if (PORTD.INTFLAGS & PORT_INT4_bm) {
        /* Clear interrupt flag */
        PORTD.INTFLAGS = PORT_INT4_bm;
        
        /* Set flag for main loop */
        zero_cross_detected = 1;
        
        /* Reset timer for phase angle control */
        TCA0.SINGLE.CNT = 0;
    }
}
