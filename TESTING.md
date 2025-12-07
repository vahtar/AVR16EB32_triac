# Testing and Validation

## Pre-Flight Checklist

Before applying mains voltage, verify:

- [ ] All connections match the circuit diagram
- [ ] Optocouplers are correctly oriented
- [ ] Triacs are properly heat-sinked
- [ ] All AC connections are insulated
- [ ] Fuses are in place
- [ ] Ground/earth connection is secure
- [ ] Low-voltage (5V) section is isolated from AC
- [ ] Emergency stop procedure is established

## Testing Phases

### Phase 1: DC System Test (No AC Power)

1. **Power Supply Test**
   ```
   Expected: 5V ±0.25V on VCC
   Measure at:
   - MCU VDD pin
   - Joystick VCC
   - Optocoupler LED side VCC
   ```

2. **Joystick Calibration**
   ```
   With DMM on PA0 and PA1:
   - Center position: ~2.5V
   - Full left/down: ~0V
   - Full right/up: ~5V
   ```

3. **MCU Programming**
   ```bash
   # Flash the firmware
   make flash
   
   # Verify flash
   avrdude -p avr16eb32 -c usbasp -U flash:v:build/triac_control.hex:i
   ```

### Phase 2: Low-Voltage AC Test (12V AC)

⚠️ Use a 12V AC transformer instead of mains voltage

1. **Zero-Cross Detection**
   ```
   With oscilloscope on PD4:
   - Frequency: 60Hz (or 50Hz)
   - Pulse width: 100-500µs
   - Regular pulses at zero-crossings
   ```

2. **Triac Trigger Pulses**
   ```
   With oscilloscope on PB2/PB3:
   - Move joystick from center
   - Observe pulses after zero-cross
   - Pulse width: ~50µs
   - Phase delay varies with joystick position
   ```

3. **Load Test (12V Lamp)**
   ```
   Connect 12V lamp to triac output
   - Center: Lamp off
   - Partial movement: Lamp dims
   - Full movement: Lamp bright
   ```

### Phase 3: Mains Voltage Test (With Extreme Caution)

⚠️ DANGER: Mains voltage can be lethal! Use isolation transformer.

1. **Initial Power-On**
   - System powered off
   - Joystick at center (loads should be off)
   - Turn on power
   - Verify no unexpected operation

2. **Gradual Load Test**
   ```
   Start with low-power resistive load (e.g., 40W lamp):
   - Slowly move joystick from center
   - Observe smooth brightness change
   - Check for flickering (indicates phase angle issues)
   - Verify both axes control independent loads
   ```

3. **Temperature Check**
   ```
   After 5 minutes of operation:
   - Triac temperature: Should be warm, not hot (< 70°C)
   - Optocoupler temperature: Should be cool (< 50°C)
   - Heat sink: Warm to touch
   - Add heat sink if components are too hot
   ```

## Validation Tests

### Test 1: Deadzone Verification

**Setup**: Oscilloscope on triac outputs

**Procedure**:
1. Center joystick
2. Move slightly (within deadzone)
3. Observe outputs

**Expected**: No triac pulses within ±50 counts from center

**Pass Criteria**: Loads remain off in deadzone

---

### Test 2: Linear Response

**Setup**: Power meter on load, voltmeter on joystick

**Procedure**:
1. Record joystick voltage and load power at intervals:
   - Center + 20%
   - Center + 40%
   - Center + 60%
   - Center + 80%
   - Center + 100%

**Expected**: Roughly linear power increase

**Pass Criteria**: Power varies smoothly with joystick position

---

### Test 3: Independent Channel Control

**Setup**: Two loads, two power meters

**Procedure**:
1. X-axis to 50%, Y-axis at center
2. X-axis at center, Y-axis to 50%
3. Both axes to 50%

**Expected**: 
- Step 1: Load 1 at 50%, Load 2 off
- Step 2: Load 1 off, Load 2 at 50%
- Step 3: Both loads at 50%

**Pass Criteria**: Channels operate independently

---

### Test 4: Zero-Cross Synchronization

**Setup**: Oscilloscope on AC line and triac trigger

**Procedure**:
1. Capture waveform with AC and trigger signals
2. Verify trigger occurs after zero-crossing
3. Measure phase delay

**Expected**: Trigger pulses synchronized with AC zero-crossings

**Pass Criteria**: No triggers before zero-crossing

---

### Test 5: Emergency Stop

**Setup**: Loads running at 50% power

**Procedure**:
1. Call emergency stop function
2. Observe loads

**Expected**: Immediate power-off

**Pass Criteria**: Loads turn off within one AC cycle (16.7ms)

---

### Test 6: Stability Test

**Setup**: Joystick at 75% position

**Procedure**:
1. Run continuously for 30 minutes
2. Monitor temperature every 5 minutes
3. Check for voltage drops or irregularities

**Expected**: Stable operation, no overheating

**Pass Criteria**: 
- No MCU resets
- Temperature stable (< 70°C)
- No flickering or instability

## Measurement Points

### With Multimeter

| Point | Expected Value | Notes |
|-------|---------------|-------|
| VCC | 5.0V ±0.25V | MCU power |
| PA0 (center) | 2.5V ±0.1V | Joystick X |
| PA1 (center) | 2.5V ±0.1V | Joystick Y |
| PB2 (idle) | 0V | Triac 1 gate |
| PB3 (idle) | 0V | Triac 2 gate |

### With Oscilloscope

| Signal | Expected | Notes |
|--------|----------|-------|
| PD4 (Zero-cross) | 60Hz pulses | 120 pulses/sec for 60Hz |
| PB2 (Trigger) | 50µs pulses | After zero-cross |
| AC Load Voltage | Chopped sine | Phase angle varies |

## Common Problems and Solutions

### Problem: No triac triggering

**Symptoms**: Loads never turn on

**Check**:
1. Optocoupler LED current (10-20mA)
2. MCU output on PB2/PB3
3. Gate current reaching triac (measured with scope)

**Solution**: Verify R1 resistor value (330Ω), check optocoupler orientation

---

### Problem: Loads always on

**Symptoms**: Cannot turn off loads

**Check**:
1. Triac gate leakage
2. Snubber circuit (if present)
3. Gate trigger pulse duration

**Solution**: Ensure pulse width is short (50µs), verify triac not shorted

---

### Problem: Flickering at low power

**Symptoms**: Lights flicker when dimmed

**Check**:
1. Zero-cross detection timing
2. Phase angle calculation accuracy
3. Load type (inductive loads may flicker)

**Solution**: 
- Use resistive loads for testing
- Add RC snubber across triac
- Calibrate zero-cross detection timing

---

### Problem: MCU resets randomly

**Symptoms**: System restarts during operation

**Check**:
1. Power supply noise
2. Decoupling capacitors (100nF close to MCU)
3. Ground loop issues

**Solution**:
- Add ferrite bead on VCC
- Improve grounding
- Use isolated power supplies

---

### Problem: Non-linear power control

**Symptoms**: Small movement gives large power change

**Check**:
1. Joystick calibration (should be linear 0-5V)
2. Deadzone setting
3. Phase angle calculation

**Solution**:
- Calibrate joystick
- Adjust `JOYSTICK_DEADZONE` in config.h
- Implement exponential or custom curve

## Calibration Procedure

### Joystick Calibration

1. **Measure Rest Position**
   ```
   With joystick at rest (no touch):
   PA0 voltage: _____ V (should be ~2.5V)
   PA1 voltage: _____ V (should be ~2.5V)
   ```

2. **Measure Full Deflection**
   ```
   Full right: PA0 = _____ V (should be ~5.0V)
   Full left:  PA0 = _____ V (should be ~0.0V)
   Full up:    PA1 = _____ V (should be ~5.0V)
   Full down:  PA1 = _____ V (should be ~0.0V)
   ```

3. **Adjust if Needed**
   If center is not ~2.5V, adjust potentiometer endpoints or use software calibration:
   
   ```c
   #define JOYSTICK_CENTER_X    520  // Adjust based on measurement
   #define JOYSTICK_CENTER_Y    508
   ```

### Phase Angle Calibration

1. **Measure Zero-Cross to Trigger Delay**
   Use oscilloscope:
   - Channel 1: Zero-cross (PD4)
   - Channel 2: Triac trigger (PB2)
   
2. **Calculate Actual Delay**
   For 50% power, delay should be ~4166µs (for 60Hz)
   
3. **Adjust if Needed**
   Modify delay calculation in `triac.c` if timing is off

## Performance Benchmarks

Expected performance metrics:

| Metric | Target | Acceptable Range |
|--------|--------|------------------|
| Response time | 20ms | 10-50ms |
| Power resolution | 1% | 0.5-2% |
| Zero-cross jitter | <100µs | <500µs |
| Temperature rise | 30°C | <50°C above ambient |
| Efficiency | 98% | >95% |

## Acceptance Criteria

System is ready for deployment when:

✓ All DC voltages are within specification
✓ Zero-cross detection is stable and accurate
✓ Both triac channels operate independently
✓ Power control is smooth and linear
✓ No overheating after 30-minute test
✓ Emergency stop functions correctly
✓ Deadzone prevents unintended activation
✓ No electromagnetic interference with nearby devices

## Sign-off

Tester: ___________________  Date: ___________

System ID: _________________

Test Results: ☐ PASS  ☐ FAIL  ☐ CONDITIONAL

Notes:
__________________________________________
__________________________________________
__________________________________________
