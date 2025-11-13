# APT Filter - Testing Guide

## 🚀 Quick Start

### Launch Standalone App
```bash
open "build/APTFilter_artefacts/Release/Standalone/APT Filter.app"
```

### Test in DAW
The plugin is automatically installed to:
- **AU:** `~/Library/Audio/Plug-Ins/Components/APT Filter.component`
- **VST3:** `~/Library/Audio/Plug-Ins/VST3/APT Filter.vst3`

---

## ✅ Feature Testing Checklist

### 1. Preset System
- [ ] Open preset dropdown at top
- [ ] Select "Warm Low Pass" - should set filter to -0.65, Q=0.707
- [ ] Select "Bright High Pass" - should set filter to 0.45, Q=0.707
- [ ] Select "Telephone" - should set filter to 0.75, Q=2.5
- [ ] Select "Radio" - should set filter to 0.55, Q=1.8
- [ ] Select "DJ Filter" - should set filter to -0.85, Q=3.5
- [ ] Select "Bypass" - should center knob at 0.0

**Expected:** Knob and resonance should update instantly, glow color should change

---

### 2. Frequency Response Graph
- [ ] Turn knob left (Low-Pass) - graph curve should slope down on right
- [ ] Turn knob right (High-Pass) - graph curve should slope down on left
- [ ] Center knob (Bypass) - graph should be flat (gray)
- [ ] Adjust resonance - graph should show peak at cutoff frequency

**Expected:** Graph updates in real-time, color matches glow (Blue/Orange/Gray)

---

### 3. Main Filter Knob
- [ ] Turn knob fully left - should show "LOW PASS" and blue glow
- [ ] Turn knob fully right - should show "HIGH PASS" and orange glow
- [ ] Center knob - should show "BYPASS" and no glow
- [ ] Snap-to-center works (auto-snaps within ±5%)

**Expected:** Smooth transitions, no clicks or pops

---

### 4. Frequency Display
- [ ] Turn left to -100% - should show "200 Hz"
- [ ] Turn left to -50% - should show around "10 kHz"
- [ ] Turn left to -1% - should show "20 kHz"
- [ ] Center at 0% - should show "BYPASS"
- [ ] Turn right to +1% - should show "20 Hz"
- [ ] Turn right to +50% - should show around "500 Hz"
- [ ] Turn right to +100% - should show "1 kHz"

**Expected:** Frequency updates in real-time, formatted as Hz or kHz

---

### 5. Filter Type Indicator
- [ ] Left position - shows "LOW PASS"
- [ ] Center position - shows "BYPASS"
- [ ] Right position - shows "HIGH PASS"

**Expected:** Text updates instantly, color changes with glow

---

### 6. Resonance Control
- [ ] Small knob at bottom right
- [ ] Turn from 0.5 (minimum) to 10.0 (maximum)
- [ ] Text box shows current Q value
- [ ] Double-click resets to 0.707 (Butterworth)

**Expected:** Smooth parameter changes, no audio artifacts

---

### 7. Glow Effect
- [ ] Turn knob left - blue glow appears and intensifies
- [ ] Turn knob right - orange glow appears and intensifies
- [ ] Center knob - glow fades out completely
- [ ] Glow is smooth and realistic (no harsh edges)

**Expected:** 
- Blue for Low-Pass (cool colors)
- Orange for High-Pass (warm colors)
- Smooth gradient falloff
- No flickering

---

### 8. Audio Processing
**Test with audio file or live input:**

- [ ] **Bypass (center):** Audio passes through unchanged
- [ ] **Low-Pass (left):** High frequencies are removed
- [ ] **High-Pass (right):** Low frequencies are removed
- [ ] **Resonance:** Creates peak at cutoff frequency
- [ ] **No clicks/pops** when moving knob
- [ ] **No volume jumps** at bypass transition

**Expected:** Smooth, musical filtering with no artifacts

---

## 🎵 Audio Test Scenarios

### Test 1: White Noise
1. Generate white noise in DAW
2. Turn knob left (LP) - should hear highs roll off
3. Turn knob right (HP) - should hear lows roll off
4. Adjust resonance - should hear peak at cutoff

**Expected:** Clear frequency response, no artifacts

---

### Test 2: Music Track
1. Load full mix
2. Try "Warm Low Pass" preset - should sound smooth
3. Try "Bright High Pass" preset - should sound clean
4. Try "Telephone" preset - should sound lo-fi
5. Try "Radio" preset - should sound vintage

**Expected:** Musical results, no distortion

---

### Test 3: Automation
1. Automate filter knob in DAW
2. Sweep from left to right slowly
3. Sweep from left to right quickly
4. Automate resonance knob

**Expected:** Smooth sweeps, no clicks or pops

---

### Test 4: Extreme Settings
1. Set resonance to 10.0 (maximum)
2. Sweep filter knob
3. Should hear resonant peak

**Expected:** Stable, no instability or distortion

---

## 🐛 Known Issues to Check

### Audio
- [ ] No clicks when moving knob quickly
- [ ] No volume jumps at bypass
- [ ] No distortion at high resonance
- [ ] Smooth parameter automation

### Visual
- [ ] Glow renders smoothly (no flickering)
- [ ] Graph updates in real-time
- [ ] Frequency display is accurate
- [ ] Preset selector works correctly

### Performance
- [ ] CPU usage is reasonable
- [ ] No UI lag when moving knobs
- [ ] No audio dropouts

---

## 📊 Performance Benchmarks

### Expected CPU Usage
- **Idle:** < 1%
- **Active (moving knobs):** < 5%
- **With graph updates:** < 8%

### Expected Latency
- **Processing:** < 1ms
- **UI response:** < 16ms (60 FPS)

---

## 🎯 Visual Inspection

### UI Elements to Check
- [ ] Preset dropdown at top (styled correctly)
- [ ] "PRESETS" label above dropdown
- [ ] "APT-FILTER" title centered
- [ ] Main knob centered with glow
- [ ] Frequency graph behind knob
- [ ] Frequency display below knob
- [ ] Filter type indicator above knob
- [ ] Resonance knob bottom right
- [ ] "RES" label above resonance knob
- [ ] "LP" label bottom left
- [ ] "HP" label bottom right
- [ ] Logo at bottom center
- [ ] Corner screws (4 corners)

### Color Scheme
- [ ] Background: Dark gradient (brown/black)
- [ ] Text: Beige/tan (#d4c5a9)
- [ ] LP Glow: Blue (#4da6ff → #a3d5ff)
- [ ] HP Glow: Orange (#ff8c00 → #ffd700)
- [ ] Knob: Brass ring, dark body

---

## 🚀 Advanced Testing

### DAW Integration
Test in multiple DAWs:
- [ ] Logic Pro
- [ ] Ableton Live
- [ ] FL Studio
- [ ] Reaper
- [ ] Pro Tools

### Automation
- [ ] Parameter automation works
- [ ] Preset changes are recorded
- [ ] Undo/redo works in DAW

### Session Recall
- [ ] Save project with plugin
- [ ] Close and reopen
- [ ] Settings are recalled correctly

---

## 📝 Bug Report Template

If you find issues, report with:

```
**Issue:** [Brief description]
**Steps to Reproduce:**
1. [Step 1]
2. [Step 2]
3. [Step 3]

**Expected:** [What should happen]
**Actual:** [What actually happens]

**Environment:**
- DAW: [Name and version]
- Format: [VST3/AU/Standalone]
- macOS: [Version]
- Audio Interface: [Name]
- Sample Rate: [e.g., 44.1 kHz]
- Buffer Size: [e.g., 512 samples]
```

---

## ✅ Sign-Off Checklist

Before considering testing complete:

### Audio Quality
- [ ] No clicks or pops
- [ ] No volume jumps
- [ ] Smooth parameter changes
- [ ] Musical sound quality

### Visual Quality
- [ ] Glow effect looks realistic
- [ ] Graph updates smoothly
- [ ] All text is readable
- [ ] Colors are correct

### Functionality
- [ ] All presets work
- [ ] All knobs respond
- [ ] Graph is accurate
- [ ] Frequency display is correct

### Performance
- [ ] CPU usage acceptable
- [ ] No UI lag
- [ ] No audio dropouts

### Compatibility
- [ ] Works in target DAWs
- [ ] Automation works
- [ ] Session recall works

---

**Happy Testing!** 🎉

If everything checks out, the plugin is ready for use!
