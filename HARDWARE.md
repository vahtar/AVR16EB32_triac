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
                      ┌──────────────┐
Phase L1 ────[ 10kΩ ]─┤              │
                      │    MID400    │
                      │              │ Pin 1 (LED+)
                      │              │ Pin 2 (LED-)
                      │              │
Phase L2 ────[ 10kΩ ]─┤              │
                      └──────────────┘
                            │
                            │ Pin 4 (Collector)
                     [ 10kΩ ]
                            │
                      VCC ──┘
                            │
                     ───────┴────────► PD4 (AVR)
                            
                     Pin 5 ──────────► GND
                     (Emitter)
```
**Circuit:** Two 10kΩ resistors in series between Phase L1 and Phase L2, with MID400 LED between them.

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
- Pin 1 (LED Anode): Connect to Phase L1 through 10kΩ, 5W resistor (high voltage rating >600V)
- Pin 2 (LED Cathode): Connect to Phase L2 through 10kΩ, 5W resistor (high voltage rating >600V)
- Pin 3: No connection (NC)
- Pin 4 (Phototransistor Collector): Connect to AVR PD4 and VCC through 10kΩ, 1/4W pull-up resistor
- Pin 5 (Phototransistor Emitter): Connect to GND
- Pin 6: No connection (NC)

**Resistor Calculations for 400V 3-Phase (Phase-to-Phase = ~400V RMS):**
- Current through LED: I = V_RMS / (R1 + R2) = 400V / (10kΩ + 10kΩ) = 20mA nominal
  - Note: Actual current may vary ±5-10% due to resistor tolerances and voltage variations
- Voltage drop per resistor: V = 400V / 2 = 200V RMS per resistor
- Power per resistor (RMS): P = V² / R = (200V)² / 10kΩ = 4W continuous
- Power per resistor (instantaneous peak): P_peak = (200V × √2)² / 10kΩ ≈ 8W at AC peak
- **Recommended:** Use 5W or higher rated resistors for adequate safety margin

**Important Safety Notes:**
- Use 5W minimum rated resistors for AC line connections (Pins 1-2) to handle power dissipation
- Resistor voltage rating must exceed peak AC voltage: 400V_RMS × √2 ≈ 565V peak
- For different voltage systems, adjust total series resistance: R_total(Ω) = V_RMS(AC) / I_LED(A)
  - Each resistor: R = R_total / 2 (for two resistors in series)
  - Typical I_LED = 10-30mA (verify with MID400 datasheet for your specific part number)
  - Example using 20mA: For 230V system, R_total = 230V / 0.020A = 11.5kΩ, use 2× 5.6kΩ or 2× 6.8kΩ
- Always verify MID400 recommended operating current and adjust resistors accordingly
- Account for voltage variations and resistor tolerances when selecting components

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
- 2x 10kΩ, 5W, >600V (MID400 AC input resistors - one MID400 unit monitors one phase pair) OR 2x 10kΩ, 1/4W (4N25/H11A1 voltage divider)
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
