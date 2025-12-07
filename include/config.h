#ifndef CONFIG_H
#define CONFIG_H

/* F_CPU must be defined before including delay.h */
#ifndef F_CPU
#define F_CPU 20000000UL  /* AVR16EB32 default 20 MHz */
#endif

/* ADC Configuration */
#define ADC_CHANNEL_JOYSTICK_X  0  /* PA0 - Joystick X-axis */
#define ADC_CHANNEL_JOYSTICK_Y  1  /* PA1 - Joystick Y-axis */
#define ADC_VREF_VDD            0  /* Use VDD as reference */

/* Triac Configuration */
#define TRIAC_1_PIN             2  /* PB2 - Triac 1 control */
#define TRIAC_2_PIN             3  /* PB3 - Triac 2 control */
#define ZERO_CROSS_PIN          4  /* PD4 - Zero cross detection */

/* Power Control Parameters */
#define MIN_POWER_LEVEL         0
#define MAX_POWER_LEVEL         100
#define JOYSTICK_CENTER         512  /* Center position (10-bit ADC) */
#define JOYSTICK_DEADZONE       50   /* Deadzone around center */

/* AC Frequency */
#define AC_FREQ_HZ              60   /* 60 Hz AC (change to 50 for Europe) */
#define AC_HALF_PERIOD_US       8333 /* Half period in microseconds (1/60/2 * 1000000) */

#endif /* CONFIG_H */
