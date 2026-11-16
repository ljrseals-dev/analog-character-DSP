# Signal Flow Reference

## Channel Strip (console path)

```
Input
  → Input Drive + Channel Tolerance trim
  → Pre-EQ (4-band musical)
  → FET Compressor (1176-style + transformers) + Parallel blend
  → Post-EQ
  → Saturation (ADAA-1, oversampled)
  → Drift / Hum / Noise
  → Output Trim
  → Output
```

Key points:
- Parallel compression dry path is delayed by 1 sample to match ADAA latency.
- Only the saturation stage runs inside the oversampler.
- Channel tolerance is a static 48-entry table (deterministic per index).

## Master Tape

```
Input
  → Calibration gain
  → Head bump + HF loss (speed dependent)
  → Oversampled tape saturation (formula + bias dependent)
  → Tape glue (slow RMS-style compression)
  → Stereo-linked wow / flutter / drift delay modulation
  → Hiss + wear noise
  → Output
```

Key points:
- 15 IPS = more head bump and character, earlier HF roll-off.
- 30 IPS = cleaner, extended top end.
- Lower bias = more distortion and HF emphasis.
- Wow/flutter is forced stereo-linked for mono compatibility.
