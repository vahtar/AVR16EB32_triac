#ifndef TRIAC_H
#define TRIAC_H

#include <stdint.h>

/**
 * @brief Initialize triac control system
 * 
 * Sets up timer for zero-cross detection and phase angle control
 */
void triac_init(void);

/**
 * @brief Set triac power level
 * 
 * @param triac_num Triac number (0, 1, or 2 for 3-phase control)
 * @param power_level Power level 0-100 (percentage)
 */
void triac_set_power(uint8_t triac_num, uint8_t power_level);

/**
 * @brief Update triac control based on joystick position
 * 
 * @param joystick_value Joystick ADC value (0-1023)
 */
void triac_update_from_joystick(uint16_t joystick_value);

/**
 * @brief Emergency stop - turn off all triacs
 */
void triac_emergency_stop(void);

#endif /* TRIAC_H */
