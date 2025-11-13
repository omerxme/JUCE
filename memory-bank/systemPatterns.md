# System Patterns

## Architecture Overview
The plugin follows JUCE's standard audio plugin architecture with clear separation of concerns.

## Component Structure

### 1. PluginProcessor (Audio Processing)
- Manages audio buffer processing
- Handles parameter state
- Implements filter DSP logic
- Thread-safe audio processing

### 2. PluginEditor (UI)
- Single rotary slider for filter control
- Visual feedback of current filter state
- Parameter binding to processor

### 3. Filter DSP Logic
- Bipolar parameter mapping (-1.0 to +1.0)
- Frequency calculation based on knob position
- IIR filter coefficient updates
- Smooth parameter interpolation

## Key Design Patterns

### Parameter Mapping
```
knob < 0: LPF mode
  cutoff = map(knob, -1.0 → 0.0, 200 Hz → 20000 Hz)

knob > 0: HPF mode
  cutoff = map(knob, 0.0 → +1.0, 20 Hz → 1000 Hz)

knob == 0: Bypass mode
  signal passes through unaffected
```

### Audio Processing Flow
```
Input → Parameter Check → Filter Selection → Apply Filter → Output
                ↓
         (LPF / HPF / Bypass)
```

## Critical Implementation Details

### Audio Processing
1. **Smooth Parameter Changes:** SmoothedValue with 50ms ramp time
2. **Thread Safety:** Audio thread never allocates memory
3. **Filter State:** Separate filter states per channel (leftFilter, rightFilter)
4. **Bypass Logic:** True bypass at center (±0.05 threshold)
5. **Crossfade:** Wet/dry mixing in ±0.1 zone around bypass
6. **Snap-to-Center:** UI automatically snaps to 0.0 within ±0.05

### Visual Effects
1. **Glow Rendering:** 3-layer system (outer 50, middle 20, inner 15)
2. **Color Temperature:** Orange→Gold→White based on intensity
3. **Falloff:** Quadratic for outer layers, linear for inner
4. **Animation:** 30 Hz timer for smooth updates
5. **Knob Design:** Vintage bakelite with brass ring and ribbed texture
