# Project Brief: APT Filter Plugin

## Overview
A single-knob dual filter audio plugin that combines Low-Pass Filter (LPF) and High-Pass Filter (HPF) in one bipolar control, inspired by Pioneer DJ mixer filter behavior.

## Core Requirements
1. Single knob control (-1.0 to +1.0 range)
2. Left turn: Low-Pass Filter (removes highs, keeps lows)
3. Right turn: High-Pass Filter (removes lows, keeps highs)
4. Center position: Bypass (clean signal)
5. Smooth, continuous frequency response

## Technical Specifications
- **LPF Range (left):** 20 kHz → 200 Hz
- **HPF Range (right):** 20 Hz → 1 kHz
- **Center:** No filtering (dry signal)
- **Curve:** Smooth and continuous (linear or slightly exponential)

## Target Platforms
- VST3
- AU (Audio Unit)
- Standalone application

## Technology Stack
- JUCE Framework (C++)
- Cross-platform (macOS, Windows, Linux)

## Success Criteria
- Single knob controls both filters intuitively
- No audio artifacts or clicks during parameter changes
- Low CPU usage
- Professional sound quality
