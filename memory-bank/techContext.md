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
│   └── PluginEditor.cpp
└── memory-bank/
```

## Key Technical Decisions
1. Use JUCE's IIR filter classes for DSP
2. Implement smooth parameter changes to avoid clicks
3. Use atomic parameters for thread-safe audio processing
4. Separate filter logic into dedicated DSP class

## Build Commands
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
