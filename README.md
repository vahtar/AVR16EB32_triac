# AVR16EB32 Triac Control with Joystick

This project implements a triac control system using a 10k potentiometer joystick with the AVR16EB32 microcontroller. The system allows bidirectional control of two AC loads through triacs based on joystick position.

## Features

- Dual-axis joystick control (X and Y)
- Independent control of two triacs
- Phase angle control for smooth power variation
- Zero-cross detection for synchronized switching
- Deadzone around center position
- 0-100% power control range
- Emergency stop functionality

## Hardware Requirements

- **Microcontroller**: AVR16EB32
- **Joystick**: Dual-axis analog joystick with 10kΩ potentiometers
- **Triacs**: Two triacs (e.g., BT136, BTA16) for AC load control
- **Optocouplers**: MOC3021 or similar for isolation
- **Zero-cross detector**: AC line zero-crossing detection circuit
- **Power supply**: 5V for microcontroller, isolated AC supply for triacs

## Pin Connections

### ADC Inputs (Joystick)
- **PA0** - Joystick X-axis (horizontal)
- **PA1** - Joystick Y-axis (vertical)
- **GND** - Common ground for potentiometers
- **VCC** - 5V supply for potentiometers

### Digital Outputs (Triac Control)
- **PB2** - Triac 1 control (via optocoupler)
- **PB3** - Triac 2 control (via optocoupler)

### Digital Input (Zero-Cross Detection)
- **PD4** - Zero-cross detection signal

## Circuit Description

### Joystick Interface
```
VCC ----+----[ 10kΩ Pot X ]----+---- GND
        |                      |
        +----> PA0             |
                               |
VCC ----+----[ 10kΩ Pot Y ]----+---- GND
        |                      |
        +----> PA1             |
```

### Triac Control (per channel)
```
PB2/PB3 --> 330Ω --> LED (MOC3021) --> Triac Gate
                     |                    |
                    GND              AC Load
```

### Zero-Cross Detector
```
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
- **X-axis** (horizontal): Controls Triac 1 power
  - Right: Increase power (0-100%)
  - Left: Alternative control mode
  - Center: 0% power (with deadzone)

- **Y-axis** (vertical): Controls Triac 2 power
  - Up: Increase power (0-100%)
  - Down: Alternative control mode
  - Center: 0% power (with deadzone)

### Power Control
- Power is controlled using phase angle control
- Zero-cross detection ensures synchronized switching
- Power range: 0-100% (fully off to fully on)
- Update rate: ~100Hz (10ms per cycle)

## Configuration

Edit `include/config.h` to adjust:
- CPU frequency (`F_CPU`)
- ADC channel assignments
- Triac pin assignments
- AC frequency (50Hz or 60Hz)
- Joystick deadzone
- Power level limits

## Safety Considerations

⚠️ **WARNING**: This project involves AC mains voltage which can be lethal!

- Always use proper isolation (optocouplers)
- Ensure all AC connections are properly insulated
- Use appropriate fusing and circuit protection
- Test with low voltage/current loads first
- Never touch AC circuits while powered
- Follow local electrical codes and regulations
- Consider professional review of circuit design

## License

This project is provided as-is for educational purposes.