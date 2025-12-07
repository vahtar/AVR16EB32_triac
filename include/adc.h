#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/**
 * @brief Initialize the ADC for reading potentiometer values
 * 
 * Configures ADC for 10-bit resolution on AVR16EB32
 */
void adc_init(void);

/**
 * @brief Read ADC value from specified channel
 * 
 * @param channel ADC channel number (0-7 for AVR16EB32)
 * @return uint16_t ADC reading (0-1023 for 10-bit)
 */
uint16_t adc_read(uint8_t channel);

/**
 * @brief Read joystick X-axis position
 * 
 * @return uint16_t ADC value (0-1023)
 */
uint16_t adc_read_joystick_x(void);

/**
 * @brief Read joystick Y-axis position
 * 
 * @return uint16_t ADC value (0-1023)
 */
uint16_t adc_read_joystick_y(void);

#endif /* ADC_H */
