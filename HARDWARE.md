# Circuit Diagram and Hardware Design

## Complete System Block Diagram

```
┌─────────────────┐
│  10kΩ Joystick  │
│   (X & Y axis)  │
└────────┬────────┘
         │ Analog
         ▼
┌─────────────────┐      ┌──────────────┐
│   AVR16EB32     │      │  Zero-Cross  │
│                 │◄─────┤   Detector   │◄──── AC Line
│  PA0 - Joy X    │      │              │
│  PA1 - Joy Y    │      └──────────────┘
│  PD4 - ZC Input │
│                 │
│  PB2 - Triac 1  ├──────►┌──────────────┐
│  PB3 - Triac 2  ├──────►│ Optocouplers │──► Triacs ──► AC Loads
└─────────────────┘       └──────────────┘
```

## Detailed Triac Control Circuit (Per Channel)

```
                    +5V
                     │
AVR Pin (PB2/3) ─────┤
                     │
                    ┌┴┐
                    │ │ 330Ω
                    └┬┘
                     │
        ┌────────────┼────────────┐
        │            │            │
        │    ┌───────▼───────┐    │
        │    │   MOC3021     │    │
        │    │  (Opto-Iso.)  │    │
        │    │               │    │
        │  1 │ LED      Triac│ 6  │
        │  2 │ Anode    Side │ 4  ├───────┐
        │    │               │    │       │
        │    └───────────────┘    │       │
        │            │            │       │
        └────────────┼────────────┘       │
                     │                    │
                    GND                   │
                                          │
                    AC Hot ───────────────┤
                                          │
                                     ┌────▼────┐
                                     │  Triac  │
                                     │ BT136/  │
                                     │ BTA16   │
                                     └────┬────┘
                                          │
                                      ┌───▼───┐
                                      │  Load │
                                      └───┬───┘
                                          │
                    AC Neutral ───────────┘
```

## Zero-Cross Detector Circuit

```
AC Line (Hot) ───┬─────┐
                 │     │
              ┌──▼──┐  │ 
              │ 10kΩ│  │
              └──┬──┘  │
                 │     │
AC Line (Neut)───┼─────┤
                 │     │
              ┌──▼──┐  │ 4.7kΩ
              │ 10kΩ│  │
              └──┬──┘  │
                 │     │
                 ├─────┘
                 │
             ┌───▼───┐
             │  4N25 │ Optocoupler
             │   or  │
             │  H11A1│
             └───┬───┘
                 │
                 ├───────► PD4 (AVR)
                 │
                GND
```

## Joystick Connection

```
              VCC (+5V)
                │
                ├──────────┐
                │          │
            ┌───▼───┐  ┌───▼───┐
            │ 10kΩ  │  │ 10kΩ  │
            │Pot-X  │  │Pot-Y  │
            └───┬───┘  └───┬───┘
                │          │
    PA0 ◄───────┤          ├──────► PA1
                │          │
                ├──────────┤
                │          │
               GND        GND
```

## Power Supply

```
AC 120/240V ──┐
              │
          ┌───▼────┐
          │ Trafo  │ (12V AC)
          └───┬────┘
              │
          ┌───▼────┐
          │ Bridge │
          │Rectif. │
          └───┬────┘
              │
          ┌───▼────┐
          │7805 Reg│
          └───┬────┘
              │
         +5V  ├──────► VCC (AVR + Joystick)
              │
             GND
```

## Component List

### Main Components
- 1x AVR16EB32 microcontroller
- 1x Dual-axis 10kΩ joystick module
- 2x BT136 or BTA16 triacs (600V, 16A)
- 2x MOC3021 optocoupler triacs
- 1x 4N25 or H11A1 optocoupler (zero-cross)
- 1x 7805 voltage regulator
- 1x Bridge rectifier (1A)
- 1x 12V transformer

### Resistors
- 2x 330Ω (LED current limit)
- 2x 10kΩ (zero-cross detector)
- 1x 4.7kΩ (zero-cross detector)
- 2x 100Ω (snubber circuits)

### Capacitors
- 2x 100nF (decoupling)
- 2x 10µF electrolytic (power supply)
- 2x 47nF (snubber circuits)

### Safety Components
- 2x Fuses (appropriate for load)
- Heat sinks for triacs (if high power)
- Isolation barriers between AC and DC sections

## PCB Design Considerations

1. **Isolation**: Maintain at least 5mm clearance between AC and DC traces
2. **Heat Management**: Provide adequate copper area for triac heat dissipation
3. **Ground Planes**: Separate analog and digital ground planes
4. **Decoupling**: Place 100nF caps close to MCU power pins
5. **AC Traces**: Make AC traces thicker (min 1mm width for 5A)
6. **Optocoupler Placement**: Mount close to triacs

## Testing Procedure

1. **DC Section Test** (Power Off)
   - Check 5V supply voltage
   - Verify joystick output varies 0-5V
   - Confirm MCU programming

2. **Signal Test** (Low Voltage)
   - Test with 12V AC instead of mains
   - Verify zero-cross detection pulses
   - Check triac trigger pulses with oscilloscope

3. **Load Test** (Mains Voltage)
   - Start with resistive load (lamp)
   - Gradually increase power
   - Monitor for overheating
   - Verify smooth power control

## Troubleshooting

| Symptom | Possible Cause | Solution |
|---------|---------------|----------|
| No triac triggering | No gate pulse | Check optocoupler connections |
| Erratic triggering | Missing zero-cross | Verify zero-cross detector |
| Triac stays on | Gate pulse too long | Reduce pulse width in code |
| Dim operation only | Wrong phase angle | Check power calculation |
| MCU resets | Noise from AC | Add more decoupling, improve grounding |

## Safety Testing

- Perform all tests with proper isolation
- Use isolation transformer during development
- Check for leakage current (<1mA)
- Verify emergency stop function
- Test under maximum load conditions
- Ensure proper fusing
