#include <avr/io.h>
#include "adc.h"
#include "config.h"

void adc_init(void) {
    /* Configure ADC reference voltage (VDD) */
    VREF.ADC0REF = VREF_REFSEL_VDD_gc;
    
    /* Configure ADC0 */
    ADC0.CTRLA = ADC_ENABLE_bm;           /* Enable ADC */
    ADC0.CTRLB = ADC_PRESC_DIV16_gc;      /* CLK_PER divided by 16 */
    ADC0.CTRLC = ADC_REFSEL_VDD_gc |      /* VDD reference */
                 ADC_SAMPCAP_bm;          /* Sample capacitance selection */
    ADC0.CTRLE = 64;                      /* Sample duration = 64 CLK_PER cycles */
    /* Free running mode disabled (default) */
}

uint16_t adc_read(uint8_t channel) {
    /* Select ADC channel */
    ADC0.MUXPOS = channel;
    
    /* Start single 10-bit conversion */
    ADC0.COMMAND = ADC_MODE_SINGLE_10BIT_gc | ADC_START_IMMEDIATE_gc;
    
    /* Wait for conversion to complete */
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));
    
    /* Clear interrupt flag */
    ADC0.INTFLAGS = ADC_RESRDY_bm;
    
    /* Return result */
    return ADC0.RESULT;
}

uint16_t adc_read_joystick(void) {
    return adc_read(ADC_CHANNEL_JOYSTICK);
}
