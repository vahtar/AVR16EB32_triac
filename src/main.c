#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "config.h"
#include "adc.h"
#include "triac.h"

int main(void) {
    /* Initialize peripherals */
    adc_init();
    triac_init();
    
    /* Enable global interrupts */
    sei();
    
    /* Main loop */
    while (1) {
        /* Read joystick position */
        uint16_t x_pos = adc_read_joystick_x();
        uint16_t y_pos = adc_read_joystick_y();
        
        /* Update triac control based on joystick */
        triac_update_from_joystick(x_pos, y_pos);
        
        /* Small delay to avoid excessive ADC reads */
        _delay_ms(10);
    }
    
    return 0;
}
