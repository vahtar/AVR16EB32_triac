# Usage Guide

## Quick Start

1. **Hardware Setup**
   - Connect the joystick potentiometers to PA0 (X-axis) and PA1 (Y-axis)
   - Wire the triacs through optocouplers to PB2 and PB3
   - Connect zero-cross detector to PD4
   - Ensure proper power supply (5V for MCU, isolated AC for loads)

2. **Build and Flash**
   ```bash
   make
   make flash  # Requires avrdude configuration
   ```

3. **Operation**
   - Power on the system
   - Move joystick to control loads:
     - X-axis (left/right) → Triac 1
     - Y-axis (up/down) → Triac 2
   - Center position = loads off (with deadzone)

## Understanding the Code

### ADC Module (`src/adc.c`)
The ADC module initializes the AVR16EB32's ADC peripheral for 10-bit resolution:
- Uses VDD as reference voltage
- Reads joystick position (0-1023 range)
- X-axis on PA0, Y-axis on PA1

### Triac Control Module (`src/triac.c`)
Implements phase angle control for triacs:
- Zero-cross detection synchronizes with AC waveform
- Timer generates precise phase delays
- Power level (0-100%) determines firing angle
- Shorter delay = more power, longer delay = less power

### Main Application (`src/main.c`)
Simple control loop:
1. Read joystick position (X and Y)
2. Calculate power levels based on position
3. Update triac control
4. Repeat with 10ms delay

## Customization

### Adjusting AC Frequency
If using 50Hz AC (Europe, Asia) instead of 60Hz (Americas):

Edit `include/config.h`:
```c
#define AC_FREQ_HZ              50   /* Change from 60 to 50 */
#define AC_HALF_PERIOD_US       10000 /* Change from 8333 to 10000 */
```

### Changing Pin Assignments
Edit pin definitions in `include/config.h`:
```c
#define ADC_CHANNEL_JOYSTICK_X  0  /* Change ADC channel */
#define ADC_CHANNEL_JOYSTICK_Y  1
#define TRIAC_1_PIN             2  /* Change output pins */
#define TRIAC_2_PIN             3
```

### Adjusting Deadzone
Make the center deadzone larger or smaller:
```c
#define JOYSTICK_DEADZONE       50   /* Increase for larger deadzone */
```

### Modifying Power Range
Limit maximum power output:
```c
#define MAX_POWER_LEVEL         80   /* Limit to 80% instead of 100% */
```

## Control Modes

### Current Mode: Independent Dual Control
- X-axis controls Triac 1 (0-100% based on position)
- Y-axis controls Triac 2 (0-100% based on position)
- Both directions from center use same triac

### Alternative Mode: Directional Control
To implement bidirectional control (e.g., for motors):

Modify `triac_update_from_joystick()` in `src/triac.c`:
```c
void triac_update_from_joystick(uint16_t x_value, uint16_t y_value) {
    // X-axis: Left uses Triac 1, Right uses Triac 2
    if (x_value > (JOYSTICK_CENTER + JOYSTICK_DEADZONE)) {
        // Right - Triac 2 forward
        uint8_t power = ((uint32_t)(x_value - JOYSTICK_CENTER - JOYSTICK_DEADZONE) * 100) / 
                        (1023 - JOYSTICK_CENTER - JOYSTICK_DEADZONE);
        triac_set_power(0, 0);
        triac_set_power(1, power);
    } else if (x_value < (JOYSTICK_CENTER - JOYSTICK_DEADZONE)) {
        // Left - Triac 1 reverse
        uint8_t power = ((uint32_t)(JOYSTICK_CENTER - JOYSTICK_DEADZONE - x_value) * 100) / 
                        (JOYSTICK_CENTER - JOYSTICK_DEADZONE);
        triac_set_power(0, power);
        triac_set_power(1, 0);
    } else {
        // Center - both off
        triac_set_power(0, 0);
        triac_set_power(1, 0);
    }
}
```

## Debugging

### Serial Output (Optional)
Add UART debugging to monitor values:

1. Add UART initialization in `main()`:
```c
#include <stdio.h>

// In main(), before main loop:
// uart_init();  // Implement UART initialization
```

2. Print joystick values:
```c
printf("X: %d, Y: %d\r\n", x_pos, y_pos);
```

### LED Indicators (Optional)
Add status LEDs to monitor operation:

```c
// In main():
PORTC.DIRSET = (1 << 0) | (1 << 1);  // PC0, PC1 as outputs

// In loop, indicate power levels:
if (triac1_power > 50) PORTC.OUTSET = (1 << 0);
else PORTC.OUTCLR = (1 << 0);
```

## Performance Optimization

### ADC Sampling Rate
Current: ~100Hz (10ms delay)
For faster response:
```c
_delay_ms(5);  // Change to 5ms = 200Hz
```

### Phase Angle Precision
Current implementation uses delay loops. For better precision:
- Use hardware timers for delay calculation
- Implement lookup table for delay values
- Use interrupt-driven phase angle control

## Safety Features

### Implementing Soft Start
Add gradual power ramp-up:

```c
static uint8_t current_power[2] = {0, 0};
#define RAMP_RATE 5  // Power increase per cycle

void triac_update_with_ramp(uint8_t triac_num, uint8_t target_power) {
    if (current_power[triac_num] < target_power) {
        current_power[triac_num] += RAMP_RATE;
        if (current_power[triac_num] > target_power)
            current_power[triac_num] = target_power;
    } else if (current_power[triac_num] > target_power) {
        current_power[triac_num] -= RAMP_RATE;
        if (current_power[triac_num] < target_power)
            current_power[triac_num] = target_power;
    }
    triac_set_power(triac_num, current_power[triac_num]);
}
```

### Over-Current Protection
Add current sensing and emergency shutdown:

```c
#define MAX_CURRENT_ADC 900  // Define safe maximum

void check_overcurrent(void) {
    uint16_t current = adc_read(2);  // Assume PA2 for current sensor
    if (current > MAX_CURRENT_ADC) {
        triac_emergency_stop();
        // Set error flag, disable further operation
    }
}
```

## Troubleshooting Common Issues

### Issue: Triacs don't trigger
**Solution**: Check optocoupler LED current (should be 10-20mA)

### Issue: Erratic zero-cross detection  
**Solution**: Add RC filter to zero-cross input, check grounding

### Issue: MCU resets during triac firing
**Solution**: Add more decoupling capacitors, improve power supply filtering

### Issue: Power control is inverted
**Solution**: Swap phase angle calculation in `triac.c`

## Advanced Topics

### PID Control
Implement closed-loop control with feedback:
- Add current/voltage sensors
- Implement PID algorithm
- Adjust triac power based on error

### Multiple Triacs
Expand to more channels:
- Add more pins in `config.h`
- Increase array sizes in `triac.c`
- Implement channel selection logic

### Communication Interface
Add external control:
- UART/SPI/I2C for remote control
- Accept power commands from PC
- Report status and measurements

## Support

For issues or questions:
1. Check hardware connections against HARDWARE.md
2. Verify component ratings and specifications
3. Review safety considerations in README.md
4. Test with low voltage/current first
