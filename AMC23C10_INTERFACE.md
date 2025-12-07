# TI AMC23C10 Interface for 400VAC Phase-to-Phase Voltage Sensing

## Overview

This document describes the interface circuit for measuring 400VAC phase-to-phase voltage using the Texas Instruments AMC23C10 precision isolated delta-sigma modulator. The AMC23C10 provides galvanic isolation between the high-voltage measurement side and the low-voltage microcontroller side.

## AMC23C10 Key Specifications

- **Input Range**: ±250mV differential
- **Isolation**: Reinforced isolation up to 5kVRMS
- **Supply Voltage**: 3.0V to 5.5V (both sides)
- **Output**: Digital bitstream (clock and data)
- **Accuracy**: ±0.5% gain error typical
- **Bandwidth**: 100kHz typical

## Voltage Scaling Calculation

### Input Specifications
- **Phase-to-Phase Voltage (RMS)**: 400VAC
- **Peak Voltage**: 400V × √2 = 565.7V
- **Target AMC23C10 Input**: ±200mV (using 80% of full scale for margin)
- **Scaling Ratio**: 565.7V / 0.2V = 2828.5:1

### Voltage Divider Design
To achieve a scaling ratio of approximately 2830:1, we use high-precision, high-voltage resistors:

- **R1 (High Side)**: 2.82 MΩ (3 × 940kΩ in series)
- **R2 (Low Side)**: 1.0 kΩ
- **Actual Ratio**: 2821:1
- **Scaled Peak Voltage**: 565.7V / 2821 = 200.5mV ✓

## Complete Interface Circuit

```
                         400VAC Phase-to-Phase
                              (L1 - L2)
                                  │
                                  │
                          ┌───────┴───────┐
                          │               │
                          │  L1 (Hot 1)   │  L2 (Hot 2)
                          │               │
                          └───────┬───────┘
                                  │
                        ┌─────────┴─────────┐
                        │                   │
                        │    Protection     │
                        │                   │
                     ┌──▼──┐             ┌──▼──┐
                     │ F1  │             │ F2  │  500mA Slow-blow Fuses
                     │500mA│             │500mA│
                     └──┬──┘             └──┬──┘
                        │                   │
                     ┌──▼──┐             ┌──▼──┐
                     │MOV1 │             │MOV2 │  Metal Oxide Varistors
                     │575V │             │575V │  (Surge Protection)
                     └──┬──┘             └──┬──┘
                        │                   │
                        ├───────────┬───────┤
                        │           │       │
                        │         ┌─┴─┐     │
                        │         │GDT│     │  Gas Discharge Tube
                        │         │600V     │  (Overvoltage Protection)
                        │         └─┬─┘     │
                        │           │       │
                        │           GND     │
                        │          (PE)     │
                        │                   │
                        │  Voltage Divider  │
                        │                   │
                     ┌──▼──┐             ┌──▼──┐
                     │ R1A │             │ R1B │  940kΩ each, 1/2W
                     │940kΩ│             │940kΩ│  1% tolerance
                     └──┬──┘             └──┬──┘  High voltage rated
                        │                   │
                     ┌──▼──┐             ┌──▼──┐
                     │ R2A │             │ R2B │  940kΩ each, 1/2W
                     │940kΩ│             │940kΩ│  1% tolerance
                     └──┬──┘             └──┬──┘
                        │                   │
                     ┌──▼──┐             ┌──▼──┐
                     │ R3A │             │ R3B │  940kΩ each, 1/2W
                     │940kΩ│             │940kΩ│  1% tolerance
                     └──┬──┘             └──┬──┘
                        │                   │
                        ├──────┬────┬───────┤
                        │      │    │       │
                     ┌──▼──┐ ┌─▼─┐ ┌▼──┐ ┌──▼──┐
                     │ C1  │ │R4A│ │R4B│ │ C2  │
                     │ 10nF│ │1kΩ│ │1kΩ│ │ 10nF│  Low-pass filter
                     └──┬──┘ └─┬─┘ └┬──┘ └──┬──┘  fc ≈ 16kHz
                        │      │    │       │
                        │      ├────┤       │
                        │      │    │       │
                        │   ┌──▼────▼──┐    │
                        │   │  AMC23C10│    │
                        │   │          │    │
                        │   │  VIN+  │◄────┤
                        ├───┤►       │     │
                        │   │  VIN-  │◄────┘
                        │   │          │
                        │   │  VDD1    │◄──── +5V (Isolated HV Side)
                        │   │          │
                        │   │  GND1    │◄──── GND (Isolated HV Side)
                        │   │          │
                        │   │─────────═│──────── Isolation Barrier
                        │   │          │
                        │   │  VDD2    │◄──── +5V (MCU Side)
                        │   │          │
                        │   │  GND2    │◄──── GND (MCU Side)
                        │   │          │
                        │   │  CLKIN   │◄──── Clock Input (20 MHz from AVR)
                        │   │  DOUT    ├────► Data Output (to AVR)
                        │   └──────────┘
                        │
                       GND (Isolated HV Side)


         Low Voltage Side (Microcontroller):
         ───────────────────────────────────

                        +5V
                         │
                      ┌──┴──┐
                      │ 100nF│  Decoupling capacitor
                      └──┬──┘
                         │
         VDD2 ──────────┤
                         │
         GND2 ─────┬─────┴──── GND (MCU Side)
                   │
                ┌──┴──┐
                │ 100nF│  Decoupling capacitor
                └─────┘

         AVR16EB32 Connections:
         ─────────────────────

         PA5 (SPI CLK)  ───────► CLKIN   (AMC23C10 Clock Input)
         PA6 (SPI MISO) ◄─────── DOUT    (AMC23C10 Data Output)
         PA7 (Chip Sel) ───────► CS      (Optional, for multi-device)

```

## High Voltage Power Supply (Isolated)

The AMC23C10 requires an isolated 5V supply on the high-voltage measurement side. Use a small isolated DC-DC converter:

```
         Primary Side               Secondary Side
         ────────────               ──────────────
              +5V                        +5V_ISO
               │                            │
           ┌───▼────┐                   ┌───▼────┐
           │  100nF │                   │  100nF │
           └───┬────┘                   └───┬────┘
               │                            │
           ┌───▼────┐                   ┌───▼────┐
           │ DC-DC  │═══════════════════│ 10µF  │
           │ Conv.  │   Isolation       └───┬────┘
           │ 5Vin   │                       │
           │ 5Vout  │                       │
           │ 1W     │                    VDD1 (AMC23C10)
           └───┬────┘
               │                          GND1 (AMC23C10)
              GND                            │
                                         ┌───▼────┐
                                         │  100nF │
                                         └───┬────┘
                                             │
                                            GND_ISO

    Recommended: Murata MGJ2D051505SC (5V to 5V, 1W, 5.2kV isolation)
                 or RECOM R0.25S-0505 (5V to 5V, 250mW, 3kV isolation)
```

## Component List

### Protection Components
| Part | Value/Rating | Description | Quantity |
|------|-------------|-------------|----------|
| F1, F2 | 500mA, 600V | Slow-blow fuse | 2 |
| MOV1, MOV2 | 575VAC | Metal oxide varistor | 2 |
| GDT | 600V | Gas discharge tube | 1 |

### Voltage Divider Resistors
| Part | Value | Power | Tolerance | Voltage Rating | Quantity |
|------|-------|-------|-----------|----------------|----------|
| R1A, R1B | 940kΩ | 1/2W | 1% | >1kV | 2 |
| R2A, R2B | 940kΩ | 1/2W | 1% | >1kV | 2 |
| R3A, R3B | 940kΩ | 1/2W | 1% | >1kV | 2 |
| R4A, R4B | 1.0kΩ | 1/4W | 1% | Standard | 2 |

**Note**: Use high-voltage thick film resistors such as:
- Vishay CRHV series (2512 package, 2kV rating)
- Ohmite SM series high voltage resistors
- Stackpole RMCF series (high voltage)

### Filter Capacitors
| Part | Value | Voltage | Type | Quantity |
|------|-------|---------|------|----------|
| C1, C2 | 10nF | 1kV | Ceramic X7R | 2 |
| C3, C4 | 100nF | 50V | Ceramic X7R | 4 |
| C5 | 10µF | 16V | Ceramic or Tantalum | 1 |

### Main Components
| Part | Part Number | Description | Quantity |
|------|-------------|-------------|----------|
| U1 | AMC23C10 | Isolated delta-sigma modulator | 1 |
| PS1 | MGJ2D051505SC | Isolated 5V to 5V DC-DC converter | 1 |

## Power Dissipation Calculations

### Voltage Divider Power Dissipation

At maximum 400VAC RMS (565.7V peak):

**Total Resistance**: R_total = 2820kΩ + 1kΩ ≈ 2821kΩ

**RMS Current**: I_rms = 400V / 2821kΩ = 0.142 mA

**Power per resistor** (for six 940kΩ resistors):
- P = I² × R = (0.142mA)² × 940kΩ = 18.9 mW

**Power in R4A and R4B** (1kΩ each):
- P = (0.142mA)² × 1kΩ = 0.02 mW

All resistors operate well within their power ratings (using 1/2W resistors).

### Temperature Considerations

- Maximum ambient temperature: 85°C
- Self-heating is minimal (<20mW per resistor)
- Use resistors with low temperature coefficient (<100ppm/°C)

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

### SPI Interface Configuration

The AMC23C10 outputs a digital bitstream that can be read via SPI:

```c
/* AMC23C10 SPI Configuration */
#define AMC23C10_CLK_PIN     5  /* PA5 - SPI Clock */
#define AMC23C10_DATA_PIN    6  /* PA6 - SPI MISO (Data) */
#define AMC23C10_CS_PIN      7  /* PA7 - Chip Select (optional) */

/* SPI Clock Frequency: 20 MHz (max for AMC23C10) */
#define AMC23C10_SPI_CLK     20000000UL

void amc23c10_init(void) {
    /* Configure SPI pins */
    PORTA.DIRSET = (1 << AMC23C10_CLK_PIN);    /* CLK as output */
    PORTA.DIRCLR = (1 << AMC23C10_DATA_PIN);   /* DATA as input */
    
    /* Configure SPI peripheral */
    SPI0.CTRLA = SPI_ENABLE_bm | SPI_MASTER_bm;
    SPI0.CTRLB = SPI_MODE_0_gc | SPI_SSD_bm;
    
    /* Set clock to 20 MHz (F_CPU/1) */
    SPI0.CTRLA |= SPI_CLK2X_bm;  /* Enable 2x clock */
}

int16_t amc23c10_read_voltage(void) {
    uint16_t data = 0;
    
    /* Read 16-bit data from AMC23C10 */
    data = SPI0.DATA;
    while (!(SPI0.INTFLAGS & SPI_IF_bm));
    data = (data << 8) | SPI0.DATA;
    
    /* Convert to signed 16-bit value */
    int16_t voltage_raw = (int16_t)data;
    
    /* Scale back to actual voltage */
    /* voltage_raw represents ±200mV, which is 565.7V peak at input */
    /* Scale factor: 565.7V / 200mV = 2828.5 */
    int32_t voltage_mv = ((int32_t)voltage_raw * 2829) / 1000;
    
    return (int16_t)voltage_mv;  /* Return voltage in millivolts */
}
```

### Pin Assignments Update

Add to `include/config.h`:

```c
/* AMC23C10 Voltage Sensor Configuration */
#define AMC23C10_ENABLED        1
#define AMC23C10_CLK_PIN        5  /* PA5 - SPI Clock */
#define AMC23C10_DATA_PIN       6  /* PA6 - SPI MISO */
#define AMC23C10_SAMPLE_RATE    1000  /* 1 kHz sampling */
```

## Calibration Procedure

### Initial Calibration
1. **Zero Offset Calibration**:
   - With no AC voltage applied (circuit powered down)
   - Read AMC23C10 output
   - Store zero offset value

2. **Gain Calibration**:
   - Apply known AC voltage (use calibrated meter)
   - Read AMC23C10 output
   - Calculate and store gain correction factor

### Runtime Calibration
```c
/* Calibration constants (stored in EEPROM) */
int16_t amc23c10_zero_offset = 0;
float amc23c10_gain_factor = 1.0;

int16_t amc23c10_calibrated_read(void) {
    int16_t raw = amc23c10_read_voltage();
    
    /* Apply zero offset correction */
    raw -= amc23c10_zero_offset;
    
    /* Apply gain correction */
    float calibrated = (float)raw * amc23c10_gain_factor;
    
    return (int16_t)calibrated;
}
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

### Stage 1: Low Voltage Testing
1. Power the circuit with isolated 12VAC
2. Verify voltage divider output: expect ~4.25mV peak
3. Check AMC23C10 digital output with oscilloscope
4. Verify SPI communication with microcontroller

### Stage 2: Medium Voltage Testing
1. Use isolation transformer (for safety)
2. Apply 120VAC (single phase)
3. Verify scaled output: expect ~60mV peak
4. Check for any thermal issues in resistors
5. Monitor isolation integrity

### Stage 3: Full Voltage Testing
1. Use proper safety equipment (insulated gloves, face shield)
2. Have emergency disconnect readily accessible
3. Apply 400VAC phase-to-phase
4. Verify output: expect ~200mV peak
5. Monitor temperature rise over 30 minutes
6. Check for any corona discharge or arcing

### Verification Measurements
- Use calibrated multimeter on AC input
- Compare AMC23C10 readings with reference meter
- Calculate error percentage (should be <1%)
- Test at multiple voltage levels (50%, 75%, 100%)

## Troubleshooting

| Symptom | Possible Cause | Solution |
|---------|---------------|----------|
| No output reading | Open fuse | Check and replace fuse |
| Incorrect scaling | Wrong resistor values | Verify resistor values with multimeter |
| Noisy readings | EMI interference | Add ferrite beads, improve shielding |
| Drift over time | Temperature effects | Use low-TC resistors, add thermal compensation |
| Loss of isolation | Damaged components | Test isolation, replace AMC23C10 or DC-DC |
| Blown fuse | Surge event | Check MOV and GDT, verify they triggered |

## Alternative Configurations

### For 230VAC Phase-to-Phase Systems
If measuring 230VAC phase-to-phase (325V peak):
- Use same circuit
- Full-scale reading will be at ~115mV
- Provides extra margin of safety

### For Current Sensing
The AMC23C10 can also be used with a shunt resistor for current sensing:
- Use 10mΩ to 100mΩ shunt resistor
- Input range ±250mV allows up to 25A (with 10mΩ shunt)
- Maintain proper isolation between shunt and microcontroller

## References

1. **AMC23C10 Datasheet**: Texas Instruments, SBAS646
2. **IEC 61010**: Safety Requirements for Measurement, Control, and Laboratory Equipment
3. **IEC 60664-1**: Insulation coordination for equipment within low-voltage systems
4. **High Voltage Resistor Selection**: Application note on voltage dividers
5. **PCB Design for High Voltage**: IPC-2221 standards

## Revision History

| Version | Date | Description |
|---------|------|-------------|
| 1.0 | 2025-12-07 | Initial release - 400VAC phase-to-phase interface |

---

**Document Status**: Design Specification
**Author**: AVR16EB32_triac Project
**Classification**: Technical Reference
