# TI AMC23C10 Interface for 400VAC Phase-to-Phase Zero-Crossing Detection

## Overview

This document describes the interface circuit for detecting zero-crossings of **400VAC 3-phase, 50Hz (EU standard)** voltage using the Texas Instruments **AMC23C10 precision isolated comparator**. The AMC23C10 provides reinforced galvanic isolation between the high-voltage measurement side and the low-voltage microcontroller side.

**System Specifications:**
- **Voltage**: 400VAC phase-to-phase (EU 3-phase standard)
- **Frequency**: 50Hz (EU) - also compatible with 60Hz (US) systems
- **Phases**: 3-phase (L1, L2, L3)
- **Application**: Industrial motor control with triac phase angle control

**Important**: The AMC23C10 is an **isolated comparator**, not a delta-sigma modulator. It is designed for fast zero-crossing detection and threshold monitoring, outputting a digital HIGH/LOW signal when the input crosses the threshold. For analog voltage measurement applications, consider the AMC1304/AMC1305 isolated delta-sigma modulator family instead.

## AMC23C10 Key Specifications

- **Device Type**: Fast response, reinforced isolated comparator with dual output
- **Input Threshold**: ±6mV (maximum trip threshold error)
- **Input Supply Range (VDD1)**: 3.0V to 27V (high-voltage side)
- **Output Supply Range (VDD2)**: 2.7V to 5.5V (low-voltage side)
- **Isolation**: Reinforced isolation up to 7000V peak (per DIN EN IEC 60747-17), 5000VRMS for 1 minute (UL1577)
- **Working Voltage**: 1000VRMS continuous
- **Output**: Dual digital outputs (open-drain and push-pull)
- **Response Time**: 500ns typical propagation delay
- **CMTI**: 100 V/ns (minimum) for push-pull output
- **Temperature Range**: -40°C to +125°C
- **Package**: 8-pin wide-body SOIC (DWV)

## Application: Phase-to-Phase Zero-Crossing Detection

The AMC23C10 is ideal for detecting zero-crossings in 400VAC 3-phase systems (L1-L2, L2-L3, or L3-L1) common in EU industrial applications. Zero-crossing detection is essential for:
- Synchronized triac firing in motor control
- Power factor correction
- Soft-start circuits
- Phase-locked loops
- Solid-state relay timing

### Input Configuration

The AMC23C10 comparator trips when the differential input voltage crosses near zero (±6mV threshold). For zero-crossing detection:

- **Phase-to-Phase Voltage (RMS)**: 400VAC (EU 3-phase standard)
- **AC Frequency**: 50Hz (EU standard) or 60Hz (US standard)
- **Peak Voltage**: 400V × √2 = 565.7V
- **Zero-Crossing**: Occurs twice per AC cycle (positive and negative transitions)
- **Output Frequency**: 100 Hz for 50Hz AC (EU), 120 Hz for 60Hz AC (US)

### Voltage Divider Design for Zero-Crossing

For zero-crossing detection, we need to scale the 400VAC signal so the zero-crossing point corresponds to near 0V at the AMC23C10 input (within ±6mV threshold). The voltage divider should be balanced:

**Design Goals:**
- Scale 565.7V peak to approximately ±10mV at AMC23C10 input (comfortable margin)
- Scaling ratio: 565.7V / 0.01V = 56,570:1
- Use symmetric resistor divider for accurate zero-crossing

**Voltage Divider:**
- **R1, R2 (High Side, each leg)**: 28.2 MΩ (3 × 9.4 MΩ in series, or 6 × 4.7 MΩ)
- **R3, R4 (Low Side, each leg)**: 1.0 kΩ (precision, matched pair)
- **Actual Ratio**: 28,200 kΩ / 1 kΩ = 28,200:1 per leg
- **Differential scaling**: Maintains ratio across both inputs
- **Scaled Peak Voltage**: 565.7V / 28,200 = ±20mV (safe margin above ±6mV threshold)

## Complete Zero-Crossing Detection Circuit

```
                         400VAC Phase-to-Phase
                          (e.g., L1 - L2)
                                  │
                          ┌───────┴───────┐
                          │               │
                      L1 (Hot 1)      L2 (Hot 2)
                          │               │
                          │               │
                    ┌─────┴─────┐   ┌─────┴─────┐
                    │           │   │           │
                    │ Protection Circuit        │
                    │                           │
                 ┌──▼──┐                     ┌──▼──┐
                 │ F1  │  Fuse 250mA         │ F2  │  Fuse 250mA
                 │250mA│  Slow-blow          │250mA│  Slow-blow
                 └──┬──┘                     └──┬──┘
                    │                           │
                 ┌──▼──┐                     ┌──▼──┐
                 │MOV1 │  Metal Oxide        │MOV2 │  Metal Oxide
                 │575V │  Varistor           │575V │  Varistor
                 └──┬──┘  (Surge Protect)    └──┬──┘  (Surge Protect)
                    │                           │
                    ├────────┬────┬─────────────┤
                    │        │    │             │
                    │      ┌─▼─┐  │             │
                    │      │GDT│  │  Gas Discharge Tube
                    │      │600V  │  (Overvoltage Protect)
                    │      └─┬─┘  │
                    │        │    │
                    │       GND   │
                    │       (PE)  │
                    │             │
                    │  Differential Voltage Divider
                    │  (Balanced for Zero-Crossing)
                    │             │
       L1 Side:     │             │     L2 Side:
                 ┌──▼──┐       ┌──▼──┐
                 │ R1A │       │ R1B │  9.4MΩ each, 1/2W
                 │9.4MΩ│       │9.4MΩ│  1% tolerance
                 └──┬──┘       └──┬──┘  High voltage rated
                    │             │
                 ┌──▼──┐       ┌──▼──┐
                 │ R2A │       │ R2B │  9.4MΩ each, 1/2W
                 │9.4MΩ│       │9.4MΩ│  1% tolerance
                 └──┬──┘       └──┬──┘
                    │             │
                 ┌──▼──┐       ┌──▼──┐
                 │ R3A │       │ R3B │  9.4MΩ each, 1/2W
                 │9.4MΩ│       │9.4MΩ│  1% tolerance
                 └──┬──┘       └──┬──┘
                    │             │
                    ├──┬────┬─────┤
                    │  │    │     │
                 ┌──▼──▼─┐ ┌▼─▼──┐
                 │  C1  │ │ C2  │  100nF, 1kV X7R
                 │ 100nF│ │100nF│  (EMI filtering)
                 └──┬───┘ └──┬──┘
                    │        │
                 ┌──▼──┐  ┌──▼──┐
                 │ R4A │  │ R4B │  1.0kΩ each, 1/4W
                 │ 1kΩ │  │ 1kΩ │  1% tolerance (matched)
                 └──┬──┘  └──┬──┘
                    │        │
                    │        │
              VIN+ ─┤        ├─ VIN-
                    │        │
                ┌───▼────────▼───┐
                │   AMC23C10     │
                │  (8-pin SOIC)  │
                │                │
                │ 1: VDD1    VDD2: 8  ◄─── +5V (MCU Side)
                │ 2: VIN+    GND2: 7  ◄─── GND (MCU Side)
                │ 3: VIN-    OUT1: 6  ────► Digital Out (Open-drain)
                │ 4: GND1    OUT2: 5  ────► Digital Out (Push-pull)
                │                │
                └────────────────┘
                      │
                   +5V_ISO (Isolated)
                   GND_ISO


         Low Voltage Side (Microcontroller Interface):
         ──────────────────────────────────────────────

                        +5V (MCU)
                         │
                      ┌──┴──┐
                      │ 100nF│  Decoupling
                      └──┬──┘
                         │
         VDD2 ──────────┤
         (Pin 8)         │
                         │
         GND2 ─────┬─────┴──── GND (MCU)
         (Pin 7)   │
                ┌──┴──┐
                │ 100nF│  Decoupling
                └─────┘

         OUT2 ──────────┬────────► PD4 (AVR Zero-Cross Input)
         (Pin 5)        │          (Configure as input, both edges)
         Push-Pull   ┌──▼──┐
                     │ 10kΩ│       Optional pull-up
                     └──┬──┘       (if using open-drain OUT1)
                        │
                       GND

         Alternative: Use OUT1 (Pin 6, Open-Drain) with external pull-up
```

## High Voltage Power Supply (Isolated)

The AMC23C10 requires an isolated power supply on the high-voltage side (VDD1). The device accepts 3V to 27V on VDD1, making it flexible for various isolated supply options.

```
         Primary Side (MCU)         Secondary Side (HV)
         ──────────────────         ───────────────────
              +5V                        +5V_ISO
               │                            │
           ┌───▼────┐                   ┌───▼────┐
           │  100nF │                   │  100nF │
           └───┬────┘                   └───┬────┘
               │                            │
           ┌───▼────────┐              ┌───▼────┐
           │  DC-DC     │══════════════│  10µF  │
           │  Isolated  │  Isolation   └───┬────┘
           │  5Vin      │   Barrier        │
           │  5Vout     │                  │
           │  1W, 5kV   │              VDD1 (Pin 1)
           └───┬────────┘              AMC23C10
               │                          │
              GND                     GND1 (Pin 4)
           (MCU Side)                     │
                                      ┌───▼────┐
                                      │  100nF │
                                      └───┬────┘
                                          │
                                       GND_ISO

    Recommended DC-DC Converters:
    • Murata MGJ2D051505SC (5V→5V, 1W, 5.2kV isolation)
    • RECOM R0.25S-0505 (5V→5V, 250mW, 3kV isolation)
    • Mornsun B0505S-1WR3 (5V→5V, 1W, 3kV isolation)
```

## Component List

### Protection Components
| Part | Value/Rating | Description | Quantity |
|------|-------------|-------------|----------|
| F1, F2 | 250mA, 600V | Slow-blow fuse | 2 |
| MOV1, MOV2 | 575VAC | Metal oxide varistor (surge protection) | 2 |
| GDT | 600V | Gas discharge tube (overvoltage protection) | 1 |

### Voltage Divider Resistors (Balanced Differential)
| Part | Value | Power | Tolerance | Voltage Rating | Quantity |
|------|-------|-------|-----------|----------------|----------|
| R1A, R1B | 9.4MΩ | 1/2W | 1% | >1kV | 2 |
| R2A, R2B | 9.4MΩ | 1/2W | 1% | >1kV | 2 |
| R3A, R3B | 9.4MΩ | 1/2W | 1% | >1kV | 2 |
| R4A, R4B | 1.0kΩ | 1/4W | 1% (matched) | Standard | 2 |

**Note**: Use high-voltage thick film resistors such as:
- Vishay CRHV series (2512 package, 2kV rating)
- Ohmite SM series high voltage resistors
- Stackpol RMCF series (high voltage)
- Or multiple 4.7MΩ resistors in series (6 per leg = 28.2MΩ)

### Filter Capacitors
| Part | Value | Voltage | Type | Quantity |
|------|-------|---------|------|----------|
| C1, C2 | 100nF | 1kV | Ceramic X7R (HV side) | 2 |
| C3, C4 | 100nF | 50V | Ceramic X7R (LV side) | 2 |
| C5 | 10µF | 16V | Ceramic or Tantalum (isolated supply) | 1 |

### Main Components
| Part | Part Number | Description | Quantity |
|------|-------------|-------------|----------|
| U1 | AMC23C10DWVR | Isolated comparator (8-pin SOIC) | 1 |
| PS1 | MGJ2D051505SC | Isolated 5V to 5V DC-DC converter (1W) | 1 |
| - | - | 10kΩ pull-up (if using OUT1 open-drain) | 1 |

## Power Dissipation Calculations

### Voltage Divider Power Dissipation

At maximum 400VAC RMS (565.7V peak):

**Total Resistance per leg**: R_total = 28,200kΩ + 1kΩ ≈ 28,201kΩ

**RMS Current per leg**: I_rms = 400V / 28,201kΩ = 0.0142 mA (14.2 µA)

**Power per high-voltage resistor** (for six 9.4MΩ resistors per leg):
- P = I² × R = (0.0142mA)² × 9.4MΩ = 1.89 mW

**Power in R4A and R4B** (1kΩ each):
- P = (0.0142mA)² × 1kΩ = 0.2 µW (negligible)

**Total power dissipation**: ~11.3 mW per leg, ~23 mW total

All resistors operate well within their power ratings (using 1/2W resistors with <2mW dissipation each).

### Temperature Considerations

- Maximum ambient temperature: 85°C
- Self-heating is minimal (<2mW per resistor)
- Use resistors with low temperature coefficient (<100ppm/°C) for stable zero-crossing detection
- Resistor matching between legs is critical for accurate zero-crossing (use 1% tolerance or better)

## PCB Layout Recommendations

### Isolation Requirements
1. **Clearance Distance**: Minimum 8mm between high-voltage and low-voltage sections
2. **Creepage Distance**: Minimum 8mm along PCB surface
3. **Isolation Barrier**: Use a routed slot in PCB for enhanced isolation
4. **Guard Ring**: Place a grounded guard ring around the isolated section

### High Voltage Section Layout
```
   ┌────────────────────────────────────────┐
   │  High Voltage Section                  │
   │  ┌──────────────────────────┐          │
   │  │  Fuses & MOV Protection  │          │
   │  └────────────┬─────────────┘          │
   │               │                         │
   │  ┌────────────▼─────────────┐          │
   │  │  Voltage Divider Chain   │          │
   │  │  (Vertical layout)        │          │
   │  │   R1A    R1B              │          │
   │  │   R2A    R2B              │          │
   │  │   R3A    R3B              │          │
   │  └────────────┬─────────────┘          │
   │               │                         │
   │  ┌────────────▼─────────────┐          │
   │  │  AMC23C10 + Filter       │          │
   │  │  + Isolated Power        │          │
   │  └──────────────────────────┘          │
   │                                         │
   └─────────────────────────────────────────┘
          ║ Isolation Barrier ║
   ┌─────────────────────────────────────────┐
   │  Low Voltage Section (MCU Side)         │
   │  ┌──────────────────────────┐           │
   │  │  SPI Interface to AVR    │           │
   │  │  Decoupling Caps         │           │
   │  └──────────────────────────┘           │
   └─────────────────────────────────────────┘
```

### Trace Guidelines
- **High Voltage Traces**: Minimum 2mm width, 3mm spacing
- **Low Voltage Traces**: Standard 0.5mm width
- **Ground Planes**: Separate ground planes for HV and LV sides
- **Via Prohibition**: No vias in the isolation zone

## Integration with AVR16EB32

### Digital Input Configuration for Zero-Crossing

The AMC23C10 outputs a digital signal that transitions HIGH/LOW when the input crosses the zero threshold. Connect to the existing zero-cross detection pin:

```c
/* AMC23C10 Zero-Crossing Configuration */
#define AMC23C10_ZC_PORT     PORTD
#define AMC23C10_ZC_PIN      4  /* PD4 - Zero-cross detection */

/* Initialize AMC23C10 zero-crossing input */
void amc23c10_init(void) {
    /* Configure pin as input with pull-up */
    AMC23C10_ZC_PORT.DIRCLR = (1 << AMC23C10_ZC_PIN);
    
    /* Configure for both edges interrupt (detect both + and - zero crossings) */
    #if AMC23C10_ZC_PIN == 4
        AMC23C10_ZC_PORT.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
    #else
        #error "AMC23C10_ZC_PIN must be on PORTD pin 4"
    #endif
}

/* Zero-crossing detection ISR - Already exists in triac.c */
ISR(PORTD_PORT_vect) {
    /* Check if it's the zero-cross pin (AMC23C10 output) */
    if (PORTD.INTFLAGS & PORT_INT4_bm) {
        /* Clear interrupt flag */
        PORTD.INTFLAGS = PORT_INT4_bm;
        
        /* Reset timer and state machine for triac firing */
        TCB0.CNT = 0;
        triac_state = 1;
        
        /* Use pre-calculated delay values from triac_set_power() */
        TCB0.CCMP = triac1_delay_ticks;
        
        /* Enable timer */
        TCB0.CTRLA |= TCB_ENABLE_bm;
    }
}

/* Read current zero-crossing state (optional, for monitoring) */
uint8_t amc23c10_read_state(void) {
    return (AMC23C10_ZC_PORT.IN & (1 << AMC23C10_ZC_PIN)) ? 1 : 0;
}
```

### Pin Assignment Summary

The AMC23C10 integrates seamlessly with the existing AVR16EB32 triac control system:

| AMC23C10 Pin | AVR16EB32 Pin | Function |
|--------------|---------------|----------|
| OUT2 (Pin 5) | PD4 | Zero-crossing detection (digital input) |
| VDD2 (Pin 8) | +5V | Power supply (MCU side) |
| GND2 (Pin 7) | GND | Ground (MCU side) |

**Note**: The existing `src/triac.c` code already handles zero-crossing detection on PD4. The AMC23C10 provides a clean, isolated zero-crossing signal that replaces simpler zero-cross detector circuits.

### AC Frequency Configuration

For 50Hz EU systems, ensure `include/config.h` is configured correctly:

```c
/* AC Frequency - UPDATE FOR EU 50Hz SYSTEMS */
#define AC_FREQ_HZ              50   /* 50 Hz AC (EU standard) */
#define AC_HALF_PERIOD_US       10000 /* Half period in microseconds (10ms for 50Hz) */
```

For 60Hz US systems, use:
```c
#define AC_FREQ_HZ              60   /* 60 Hz AC (US standard) */
#define AC_HALF_PERIOD_US       8333 /* Half period in microseconds (8.33ms for 60Hz) */
```

## Safety Considerations

### ⚠️ HIGH VOLTAGE WARNING

This circuit operates with **400VAC phase-to-phase voltage** which is **LETHAL**!

### Mandatory Safety Measures

1. **Isolation Testing**:
   - Test isolation resistance (>100MΩ required)
   - Test isolation voltage at 1000VDC for 1 minute
   - Use hipot tester if available

2. **Physical Barriers**:
   - Enclose high-voltage section in plastic housing
   - Use warning labels on all HV sections
   - Provide safety interlocks on access panels

3. **Fusing**:
   - Primary fuses must be rated for 600V minimum
   - Use slow-blow type to prevent nuisance trips
   - Size fuses for maximum expected fault current

4. **Grounding**:
   - Connect chassis to protective earth (PE)
   - Ensure low-impedance ground path
   - Test ground continuity regularly

5. **Professional Review**:
   - Have circuit reviewed by certified electrical engineer
   - Comply with local electrical codes
   - Follow IEC 61010 (Safety Requirements for Electrical Equipment)

## Testing Procedure

### Stage 1: Bench Testing (No High Voltage)
1. **Power Supply Test**:
   - Connect isolated DC-DC converter
   - Verify +5V_ISO on VDD1 (Pin 1)
   - Verify +5V on VDD2 (Pin 8)
   - Check both grounds are isolated from each other

2. **Digital Output Test**:
   - With no AC input, check OUT2 (Pin 5) state
   - Should be stable HIGH or LOW
   - Verify signal reaches AVR PD4 pin

### Stage 2: Low Voltage Testing (12VAC)
1. **Setup**:
   - Use 12VAC transformer (isolated from mains)
   - Apply 12VAC across L1 and L2 inputs
   - Expected input to AMC23C10: ~0.6mV peak (scaled down)

2. **Verification**:
   - Monitor OUT2 (Pin 5) with oscilloscope
   - Should see square wave at 2× AC frequency (100Hz for 50Hz EU, 120Hz for 60Hz US)
   - Verify transitions occur at zero-crossings
   - Check propagation delay (~500ns typical)

3. **AVR Integration**:
   - Run existing triac control code
   - Verify PORTD_PORT_vect ISR triggers on each zero-crossing
   - Confirm triac firing timing is correct

### Stage 3: Medium Voltage Testing (120VAC)
1. **Safety Setup**:
   - Use isolation transformer
   - Ensure proper grounding
   - Keep hands clear of HV section

2. **Testing**:
   - Apply 120VAC phase-to-phase
   - Expected input to AMC23C10: ~4.25mV peak
   - Verify zero-crossing detection remains accurate
   - Monitor for noise or false triggers
   - Check resistor temperatures (should be barely warm)

### Stage 4: Full Voltage Testing (400VAC)
1. **Safety Precautions**:
   - Wear insulated gloves and face shield
   - Have emergency disconnect switch ready
   - Ensure proper ventilation
   - Work with a partner (never alone)

2. **Testing**:
   - Apply 400VAC phase-to-phase
   - Expected input to AMC23C10: ~20mV peak (well within ±6mV threshold margin)
   - Verify zero-crossing detection at full voltage
   - Monitor for 30 minutes:
     - Check component temperatures
     - Verify no corona discharge (audible hissing)
     - Check for any arcing
     - Confirm stable operation

3. **Frequency Verification**:
   - For 50Hz AC (EU): Zero-crossing frequency should be 100Hz
   - For 60Hz AC (US): Zero-crossing frequency should be 120Hz
   - Use oscilloscope or frequency counter on PD4

### Verification Checklist
- [ ] Isolation barrier tested (>100MΩ resistance)
- [ ] Zero-crossing timing accurate (±100µs)
- [ ] No false triggers under load
- [ ] Stable operation at full voltage
- [ ] Triac firing synchronized with zero-crossings
- [ ] Temperature rise acceptable (<10°C above ambient)
- [ ] No electromagnetic interference issues

## Troubleshooting

| Symptom | Possible Cause | Solution |
|---------|---------------|----------|
| No zero-crossing pulses | Open fuse or bad connection | Check fuses F1, F2; verify continuity |
| No output transitions | AMC23C10 not powered | Check VDD1 (+5V_ISO) and VDD2 (+5V) |
| Continuous HIGH or LOW | Broken voltage divider | Test each resistor with multimeter |
| Erratic triggering | EMI/noise coupling | Add ferrite beads, improve grounding |
| False triggers | Input exceeds threshold | Check resistor values, verify scaling calculation |
| Phase shift in detection | RC time constant too high | Reduce C1, C2 values or remove if EMI permits |
| Output doesn't reach AVR | Bad connection or damaged pin | Check continuity from OUT2 to PD4 |
| Blown fuse on power-up | Surge/inrush current | Verify MOV and GDT are functioning |
| Drift over time | Temperature coefficient | Use precision resistors (<50ppm/°C) |
| Loss of isolation | Damaged AMC23C10 | Test isolation, replace IC if compromised |

## Alternative Configurations

### For 230VAC Phase-to-Phase Systems
If measuring 230VAC phase-to-phase (325V peak):
- Use same circuit design
- Scaled voltage at AMC23C10 input: 325V / 28,201 = ±11.5mV
- Still well within the ±6mV threshold for reliable zero-crossing
- Provides good margin of safety

### For Three-Phase Zero-Crossing Detection
For complete 3-phase motor control, use three AMC23C10 circuits:

1. **AMC23C10 #1**: L1-L2 phase-to-phase (0° reference)
2. **AMC23C10 #2**: L2-L3 phase-to-phase (120° shifted)
3. **AMC23C10 #3**: L3-L1 phase-to-phase (240° shifted)

Connect outputs to three different AVR pins (e.g., PD4, PD5, PD6) for independent phase monitoring.

### Using Open-Drain Output (OUT1)
If using the open-drain output instead of push-pull:

```
         OUT1 ───────┬────────► PD4 (AVR)
         (Pin 6)     │
                  ┌──▼──┐
                  │ 10kΩ│  Pull-up resistor to +5V
                  └──┬──┘
                     │
                    +5V
```

Benefits of open-drain:
- Can be wire-OR'd with other signals
- Flexible logic levels with pull-up to different voltages
- Slightly lower CMTI (75 V/ns vs 100 V/ns for push-pull)

### For Voltage Measurement (Not Just Zero-Crossing)
If you need actual voltage measurement (RMS, peak, etc.), consider using:
- **AMC1304M25**: ±250mV input, 16-bit resolution, isolated delta-sigma modulator
- **AMC1305M25**: Similar to AMC1304 with CMOS output
- **AMC0306M25**: ±50mV input for lower voltage/higher sensitivity applications

These provide digital bitstream output for precise voltage reconstruction, suitable for power quality monitoring, energy metering, etc.

## Comparison: AMC23C10 vs. Traditional Zero-Cross Detectors

### AMC23C10 Advantages:
- **Superior isolation**: 7000V peak reinforced isolation
- **High CMTI**: 100 V/ns immunity to transients
- **Fast response**: 500ns propagation delay
- **Precise threshold**: ±6mV (vs. ±100mV for discrete circuits)
- **Compact**: Single 8-pin IC replaces multiple components
- **No transformer required**: Direct resistive divider connection
- **Temperature stable**: -40°C to +125°C operation

### Traditional Zero-Cross Detector (H11A1/4N25 + Transformer):
- Lower cost (~$1 vs ~$3)
- Simpler power supply (no isolated DC-DC needed)
- Lower precision (±100mV to ±500mV typical)
- Bulkier (requires transformer)
- Lower CMTI (<10 V/ns typical)

**Recommendation**: Use AMC23C10 for professional/industrial applications requiring high precision, reliability, and isolation. Use traditional circuits for hobby/low-cost projects.

## Design Considerations for 3-Phase Systems

### Balanced Load Monitoring
When monitoring all three phase-to-phase voltages:
- L1-L2, L2-L3, L3-L1 should be 120° apart
- Any phase imbalance will be visible in zero-crossing timing
- Can detect phase loss or severe imbalance

### Synchronization for Triac Firing
For synchronized 3-phase motor control:
1. Use one AMC23C10 as master reference (e.g., L1-L2)
2. Calculate firing angles for all three triacs from this reference
3. All three phases will be properly synchronized
4. Maintains balanced motor operation

### Fault Detection
The AMC23C10 can help detect:
- **Phase loss**: No zero-crossing pulses on one phase
- **Phase imbalance**: Irregular zero-crossing intervals
- **Voltage sag**: Timing remains correct, but can add voltage measurement
- **Harmonics**: May cause multiple false crossings (filter if necessary)

## References

1. **AMC23C10 Datasheet**: Texas Instruments, SBAS946 - "Fast Response, Reinforced Isolated Comparator With Dual Output"
   - https://www.ti.com/lit/ds/symlink/amc23c10.pdf

2. **Application Note**: TI SBOA328 - "Isolated Voltage Sensing for Three-Phase Applications"

3. **IEC 61010**: Safety Requirements for Measurement, Control, and Laboratory Equipment

4. **IEC 60664-1**: Insulation coordination for equipment within low-voltage systems

5. **High Voltage Resistor Selection**: Vishay Technical Note on Voltage Dividers
   - https://www.vishay.com/resistors/high-voltage/

6. **PCB Design for High Voltage**: IPC-2221 standards for creepage and clearance

7. **Isolated DC-DC Converters**: Application guide for isolation barriers in industrial applications

8. **Related TI Products**:
   - AMC1304/AMC1305: For precision voltage measurement (delta-sigma modulators)
   - AMC1300: Basic isolated amplifier
   - AMC3330: Precision isolated amplifier with integrated ADC

## Appendix: Resistor Value Selection

### Standard Value Alternatives

If 9.4MΩ resistors are unavailable, use series combinations:

**Option 1**: Use 4.7MΩ resistors (6 per leg)
- 6 × 4.7MΩ = 28.2MΩ per leg ✓ (exact match)
- Easier to source
- Same power dissipation per resistor (~1mW)

**Option 2**: Use 10MΩ resistors (3 per leg, but slightly different scaling)
- 3 × 10MΩ = 30MΩ per leg
- Scaled voltage: 565.7V / 30,001 = ±18.9mV (still adequate)
- Acceptable for zero-crossing detection

**Option 3**: Use 5.6MΩ resistors (5 per leg)
- 5 × 5.6MΩ = 28MΩ per leg
- Very close to ideal: 565.7V / 28,001 = ±20.2mV
- Good compromise

### Critical Requirements
- **Voltage rating**: Each resistor must handle 1/3 to 1/6 of total voltage
  - For 565.7V peak: Each 9.4MΩ resistor sees ~188V peak
  - Use >500V rated resistors (1kV preferred)
- **Tolerance**: Use 1% or better for balanced divider
- **Temperature coefficient**: <100ppm/°C preferred
- **Power rating**: 1/2W provides good safety margin
- **Series connection**: Solder multiple resistors in series for higher voltage rating

## Appendix: Timing Analysis

### Zero-Crossing Detection Timing

For 50Hz AC (EU Standard), 400VAC phase-to-phase:
- **Period**: 20ms (one complete cycle)
- **Half-period**: 10ms
- **Zero-crossings**: 2 per cycle = 100 Hz
- **Time between crossings**: 10ms

For 60Hz AC (US Standard), 400VAC phase-to-phase:
- **Period**: 16.67ms (one complete cycle)
- **Half-period**: 8.33ms
- **Zero-crossings**: 2 per cycle = 120 Hz
- **Time between crossings**: 8.33ms

### AMC23C10 Response Time
- **Propagation delay**: 500ns typical (max 800ns)
- **Jitter**: <50ns typical
- **Total timing error**: <1µs (negligible for triac control at 50/60Hz)

### Triac Firing Window
- **Minimum firing angle**: ~0.5ms after zero-crossing (from config.h)
- **Maximum firing angle**: 10ms (near end of half-cycle for 50Hz EU) or 8.3ms (for 60Hz US)
- **Resolution**: 10MHz timer = 100ns per tick = 0.0018° resolution at 50Hz

## Appendix: Waveform Examples

### Expected Oscilloscope Traces

**Channel 1: AC Input (L1-L2)** - 400VAC RMS, 50Hz (EU Standard)
```
    +565V ─────────╱╲─────────╱╲─────────
                  ╱  ╲       ╱  ╲
                 ╱    ╲     ╱    ╲
    0V   ───────┼──────┼───┼──────┼──── Zero crossings
                 ╲    ╱     ╲    ╱
                  ╲  ╱       ╲  ╱
    -565V ─────────╲╱─────────╲╱─────────
                   
                |←─ 10ms ──→|  (50Hz half-period)
```

**Channel 2: AMC23C10 OUT2 (Digital)**
```
    +5V  ─┐     ┌─────┐     ┌─────┐     
          │     │     │     │     │     
          │     │     │     │     │     
    0V   ─┘─────┘     └─────┘     └─────
          ↑           ↑           ↑
       Zero-cross  Zero-cross  Zero-cross
       (positive)  (negative)  (positive)
       
       |←───── 100 Hz (50Hz AC) ──────→|
```

**Note**: For 60Hz systems (US), the half-period is 8.33ms and output frequency is 120Hz.

**Timing**: OUT2 transitions occur precisely at AC input zero-crossings with ~500ns delay.

## Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0 | 2025-12-07 | Initial release - 400VAC phase-to-phase interface |

---

**Document Status**: Design Specification
**Author**: AVR16EB32_triac Project
**Classification**: Technical Reference
