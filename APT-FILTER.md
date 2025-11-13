# FILTER KNOB BEHAVIOR (Single Knob Dual Filter Control)

This document defines how a **single bipolar filter knob** should behave.  
The knob combines both a **Low-Pass Filter (LPF)** and a **High-Pass Filter (HPF)** in one control, similar to Pioneer DJ mixers.

---

## Behavior

- **Turn Left (Counterclockwise):**  
  → Apply a **Low-Pass Filter (LPF)**.  
  Gradually remove high frequencies, keeping the low end.

- **Turn Right (Clockwise):**  
  → Apply a **High-Pass Filter (HPF)**.  
  Gradually remove low frequencies, keeping the high end.

- **Center Position (0):**  
  → No effect.  
  The signal remains clean and unaffected (dry).

---

## Technical Notes

- The filter cutoff frequency changes symmetrically around the center point.  
- Suggested ranges:
  - **LPF (left side):** 20 kHz → 200 Hz  
  - **HPF (right side):** 20 Hz → 1 kHz  
- The transition curve should be **smooth and continuous** (linear or slightly exponential) to prevent jumps in frequency response.
- Only one parameter (`Filter`) controls both filters.

---

## Pseudo Implementation

```python
if knob < 0:
    apply LowPassFilter with cutoff = map(knob, -1.0 → 0.0, 200 Hz → 20000 Hz)
elif knob > 0:
    apply HighPassFilter with cutoff = map(knob, 0.0 → +1.0, 20 Hz → 1000 Hz)
else:
    bypass filter (dry signal)
```

---

## Summary

This creates a **one-knob dual filter control**:
- Left = warm, muffled low-pass sound  
- Right = bright, thin high-pass sound  
- Center = clean, unprocessed signal
