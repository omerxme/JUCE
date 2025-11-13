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
1. **Smooth Parameter Changes:** Use smoothed values to prevent audio clicks
2. **Thread Safety:** Audio thread never allocates memory
3. **Filter State:** Maintain separate filter states per channel
4. **Bypass Logic:** When knob is at center, bypass filter processing entirely
