# Technical Context

## Technology Stack
- **Framework:** JUCE 7.x (C++)
- **Language:** C++17 or later
- **Build System:** CMake
- **Plugin Formats:** VST3, AU, Standalone

## Development Environment
- **OS:** macOS (darwin)
- **Shell:** zsh
- **Compiler:** Clang (Xcode Command Line Tools)

## Dependencies
- JUCE Framework (will be downloaded via CMake)
- CMake 3.15+
- Xcode Command Line Tools

## Project Structure
```
apt-filter/
├── CMakeLists.txt
├── Source/
│   ├── PluginProcessor.h
│   ├── PluginProcessor.cpp
│   ├── PluginEditor.h
│   ├── PluginEditor.cpp
│   ├── CustomLookAndFeel.h
│   └── CustomLookAndFeel.cpp
└── memory-bank/
    ├── projectbrief.md
    ├── productContext.md
    ├── activeContext.md
    ├── systemPatterns.md
    ├── techContext.md
    └── progress.md
```

## Key Technical Decisions

### Audio Processing
1. Use JUCE's IIR filter classes for DSP (ProcessorDuplicator)
2. Implement smooth parameter changes to avoid clicks (SmoothedValue, 50ms)
3. Use atomic parameters for thread-safe audio processing (APVTS)
4. Crossfade wet/dry for smooth bypass transitions (±0.1 fade zone)
5. True bypass at center position (no DSP processing)

### UI/UX
1. Custom LookAndFeel class for vintage knob design
2. Timer-based repaint for smooth glow animation (30 Hz)
3. Snap-to-center behavior for better user experience (±0.05)
4. Multi-layer glow rendering (85 total layers)
5. Color interpolation for heat effect (orange→yellow→white)

## Build Commands
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
