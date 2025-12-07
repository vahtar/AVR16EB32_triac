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

### Option 1: MID400 Optocoupler (Recommended for Phase-to-Phase Detection)
```
Phase L1 ────[ 10kΩ ]────┬────┐
                         │    │
                         │  ┌─▼──┐
                         │  │LED+│ Pin 1 (Anode)
                         │  │    │
                         │  │MID │
                         │  │400 │
                         │  │    │
                         │  │LED-│ Pin 2 (Cathode)
                         │  └─┬──┘
                         │    │
Phase L2 ────[ 10kΩ ]────┴────┘
                         
             [ 10kΩ ]
                │
         VCC ───┘
                │
         Pin 4 ─┴────────► PD4 (AVR)
         (Collector)
                
         Pin 5 ──────────► GND
         (Emitter)
```
**Note:** Pin numbers reference DIP-6 package. Check datasheet for your specific package type.

**MID400 Advantages:**
- Designed specifically for AC input and zero-crossing detection
- Can directly sense phase-to-phase voltage (ideal for 3-phase systems)
- Better noise immunity and more reliable zero-crossing detection
- Simplified circuit with fewer components
- Wide AC input voltage range (100-600V AC)
- Built-in current limiting

### Option 2: Standard Optocoupler (4N25/H11A1)
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

## Zero-Cross Detector Wiring Guide

### MID400 Wiring (3-Phase System)
For 3-phase motor control, the MID400 can directly sense phase-to-phase voltage:

**Pin Connections (DIP-6 Package):**
- Pin 1 (LED Anode): Connect to Phase L1 through 10kΩ, 1/2W resistor (high voltage rating)
- Pin 2 (LED Cathode): Connect to Phase L2 through 10kΩ, 1/2W resistor (high voltage rating)
- Pin 3: No connection (NC)
- Pin 4 (Phototransistor Collector): Connect to AVR PD4 and VCC through 10kΩ, 1/4W pull-up resistor
- Pin 5 (Phototransistor Emitter): Connect to GND
- Pin 6: No connection (NC)

**Resistor Calculations for 400V 3-Phase (Phase-to-Phase = ~400V):**
- Current through LED: I = V / (2 × R) = 400V / (2 × 10kΩ) ≈ 20mA (typical for optocouplers)
- Power per resistor: P = V² / (4 × R) = 400² / (4 × 10kΩ) = 4W worst case
- **Recommended:** Use 2W or higher rated resistors with voltage rating >400V for safety margin

**Important Safety Notes:**
- Use 1/2W minimum, 2W recommended resistors for AC line connections (Pins 1-2)
- Resistor voltage rating must exceed peak AC voltage (400V × 1.414 = 565V peak)
- For different voltage systems, adjust resistor values: R(kΩ) ≈ V(AC) / 20mA
- Always verify actual voltage levels and adjust component ratings accordingly

**Note:** Different package types may have different pin arrangements. Always verify with the MID400 datasheet for your specific package and voltage rating.

**Advantages for 3-Phase:**
- Detects zero-crossing between any two phases (L1-L2, L2-L3, or L1-L3)
- Provides reliable synchronization for all three triacs
- Simpler circuit than detecting from neutral
- Wide input voltage range (typically 100-600V AC - verify with specific MID400 datasheet)
- Only one MID400 needed for 3-phase system (monitors one phase pair)

### Standard Optocoupler Wiring (Single Phase to Neutral)
For simpler applications or single-phase systems, use 4N25 or H11A1 with bridge rectifier as shown in diagram above.

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
- 1x Dual-axis 10kΩ joystick module (only single axis used for 3-phase control)
- 3x BT136 or BTA16 triacs (600V, 16A) - one per phase
- 3x MOC3021 optocoupler triacs - one per phase
- 1x MID400 AC input optocoupler (zero-cross, recommended) OR 1x 4N25/H11A1 optocoupler
- 1x 7805 voltage regulator
- 1x Bridge rectifier (1A)
- 1x 12V transformer

### Resistors
- 3x 330Ω, 1/4W (LED current limit for MOC3021 - one per phase)
- 2x 10kΩ, 2W (MID400 AC input resistors - one MID400 unit monitors one phase pair) OR 2x 10kΩ, 1/4W (4N25/H11A1 voltage divider)
- 1x 10kΩ, 1/4W (pull-up resistor for zero-crossing detector output)
- 1x 4.7kΩ, 1/4W (zero-crossing detector - only needed for 4N25/H11A1 option)
- 3x 100Ω, 2W (snubber circuits - one per triac, high power rating)

### Capacitors
- 2x 100nF (decoupling)
- 2x 10µF electrolytic (power supply)
- 3x 47nF (snubber circuits - one per triac)

### Safety Components
- 3x Fuses (one per phase, appropriate for load)
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
