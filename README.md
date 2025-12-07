# AVR16EB32 3-Phase Motor Control with Joystick

This project implements a 3-phase motor triac control system using a 10k potentiometer joystick with the AVR16EB32 microcontroller. The system allows smooth speed control of a 400V 3-phase motor through phase angle control.

## Features

- Single-axis joystick control for motor speed
- Synchronized control of three triacs (one per phase)
- Phase angle control for smooth power variation (0-100%)
- Zero-cross detection for synchronized switching
- Deadzone around center position
- Emergency stop functionality
- Suitable for 400V 3-phase motors

## Hardware Requirements

- **Microcontroller**: AVR16EB32
- **Joystick**: Single-axis analog joystick with 10kΩ potentiometer
- **Triacs**: Three triacs (e.g., BT136, BTA16) rated for 400V AC
- **Optocouplers**: MOC3021 or similar for isolation (3 units)
- **Zero-cross detector**: MID400 AC input optocoupler (recommended) or standard AC line zero-crossing detection circuit (4N25/H11A1)
- **Power supply**: 5V for microcontroller, isolated 400V 3-phase supply for motor

## Pin Connections

### ADC Input (Joystick)
- **PA0** - Joystick axis (single axis control)
- **GND** - Common ground for potentiometer
- **VCC** - 5V supply for potentiometer

### Digital Outputs (Triac Control)
- **PB2** - Triac 1 control (Phase L1, via optocoupler)
- **PB3** - Triac 2 control (Phase L2, via optocoupler)
- **PB4** - Triac 3 control (Phase L3, via optocoupler)

### Digital Input (Zero-Cross Detection)
- **PD4** - Zero-cross detection signal

## Circuit Description

### Joystick Interface
```
VCC ----+----[ 10kΩ Pot ]----+---- GND
        |                    |
        +----> PA0           |
```

### Triac Control (per phase)
```
PB2/PB3/PB4 --> 330Ω --> LED (MOC3021) --> Triac Gate
                          |                    |
                         GND            400V 3-Phase Motor
```

### Zero-Cross Detector
```
Option 1: MID400 (Recommended for 3-phase)
Phase L1-L2 --> Resistors --> MID400 Optocoupler --> PD4
                                      |
                                     GND

Option 2: Standard (4N25/H11A1)
AC Line --> Transformer --> Bridge Rectifier --> Optocoupler --> PD4
                                                        |
                                                       GND
```

## Building the Project

### Prerequisites
- avr-gcc compiler
- avr-libc
- avrdude (for programming)
- GNU Make

### Compilation
```bash
make
```

This will create:
- `build/triac_control.hex` - Flash file for programming
- `build/triac_control.elf` - ELF executable
- `build/triac_control.lss` - Extended listing
- `build/triac_control.map` - Linker map

### Programming
```bash
make flash
```

Note: Adjust the programmer type in Makefile if not using usbasp.

### Cleaning
```bash
make clean
```

## Operation

### Joystick Control
- **Single Axis**: Controls 3-phase motor speed (all three triacs synchronized)
  - Forward: Increase power (0-100%)
  - Backward: Decrease power
  - Center: 0% power (with deadzone)

### Power Control
- Power is controlled using phase angle control
- Zero-cross detection ensures synchronized switching
- All three phases controlled with same power level for balanced motor operation
- Power range: 0-100% (fully off to fully on)
- Update rate: ~100Hz (10ms per cycle)

## Configuration

Edit `include/config.h` to adjust:
- CPU frequency (`F_CPU`)
- ADC channel assignment
- Triac pin assignments (PB2, PB3, PB4)
- AC frequency (50Hz or 60Hz) 
- Joystick deadzone
- Power level limits

## Safety Considerations

⚠️ **WARNING**: This project involves 400V 3-phase AC which can be LETHAL!

- **HIGH VOLTAGE**: 400V 3-phase system requires professional installation
- Always use proper isolation (optocouplers rated for 400V)
- Ensure all AC connections are properly insulated
- Use appropriate fusing and circuit protection for motor current
- Test with low voltage/current loads first
- Never touch AC circuits while powered
- Follow local electrical codes and regulations
- **MANDATORY**: Professional electrician review for 3-phase installations
- Use proper motor protection (thermal overload, contactor)

## License

This project is provided as-is for educational purposes.