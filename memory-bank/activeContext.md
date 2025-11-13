# Active Context

## Current Focus
All issues resolved! Smooth bypass, snap-to-center, and beautiful glow effect working.

## Recent Changes
- ✅ Fixed glow rendering (30 layers, proper alpha blending)
- ✅ Added snap-to-center feature (±0.05 snap zone)
- ✅ Fixed volume jump with crossfade (0.1 fade zone)
- ✅ Smooth wet/dry mixing near bypass point
- ✅ Timer-based repaint for smooth glow animation (30 Hz)
- ✅ Wider bypass threshold (±0.05) for stability

## Next Steps
1. Test all features with real audio
2. Fine-tune snap zone if needed
3. Optional: Add frequency response graph
4. Optional: Add resonance control

## Active Decisions
- Snap-to-center: ±0.05 zone for easy centering
- Crossfade zone: ±0.1 for smooth transitions
- Bypass threshold: ±0.05 (wider than before)
- Glow: 30 layers with 0.12 alpha per layer
- UI refresh: 30 Hz for smooth animation

## Important Patterns
- Bipolar parameter: -1.0 (full LPF) to +1.0 (full HPF)
- Snap zone: Automatically snaps to 0.0 when within ±0.05
- Crossfade: Wet/dry mix when within ±0.1 of center
- Glow intensity: Directly tied to abs(filterValue)

## Current Learnings
- Crossfading prevents volume jumps at bypass transition
- Snap-to-center improves user experience
- Timer-based repaint keeps glow smooth
- Wider thresholds prevent rapid state changes
