#ifndef CONFIG_H
#define CONFIG_H

/* F_CPU must be defined before including delay.h */
#ifndef F_CPU
#define F_CPU 20000000UL  /* AVR16EB32 default 20 MHz */
#endif

/* ADC Configuration */
#define ADC_CHANNEL_JOYSTICK    0  /* PA0 - Joystick axis (single axis control) */
#define ADC_VREF_VDD            0  /* Use VDD as reference */

/* Triac Configuration - 3-Phase Motor Control */
#define TRIAC_1_PIN             2  /* PB2 - Triac 1 control (Phase L1) */
#define TRIAC_2_PIN             3  /* PB3 - Triac 2 control (Phase L2) */
#define TRIAC_3_PIN             4  /* PB4 - Triac 3 control (Phase L3) */
#define ZERO_CROSS_PIN          4  /* PD4 - Zero cross detection (different port than TRIAC_3_PIN) */

/* Power Control Parameters */
#define MIN_POWER_LEVEL         0
#define MAX_POWER_LEVEL         100
#define JOYSTICK_CENTER         512  /* Center position (10-bit ADC) */
#define JOYSTICK_DEADZONE       50   /* Deadzone around center */

/* Triac Timing Parameters */
#define MIN_FIRING_DELAY_US     500  /* Minimum delay for max power (microseconds) */
#define TRIAC_PULSE_WIDTH_US    50   /* Triac gate pulse width (microseconds) */
#define TIMER_TICKS_PER_US      10   /* Timer ticks per microsecond at 10MHz (CLK_PER/2) */

/* AC Frequency */
#define AC_FREQ_HZ              60   /* 60 Hz AC (change to 50 for Europe) */
#define AC_HALF_PERIOD_US       8333 /* Half period in microseconds (rounded from 8333.33) */

#endif /* CONFIG_H */
