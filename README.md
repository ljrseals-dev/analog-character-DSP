# Analog Character DSP

Framework-agnostic C++17 DSP modules for adding analog console and tape character to digital audio.

This repository contains complete, compilable implementations of:

- Kaiser-window polyphase oversampler
- 1176-style FET compressor with input/output transformer modelling
- ADAA-1 tape saturation (multiple formulations)
- Musical 4-band console EQ
- Channel tolerance (TMT-style) variation
- Analog drift / hum / noise
- Stereo-linked tape transport (wow / flutter / drift)

All nonlinear stages use first-order antiderivative anti-aliasing (ADAA-1) and can be wrapped in high-quality oversampling.

---

## Manual: Tiny Boost vs Overpowered Character

### 1. The Tiny Boost (subtle, musical enhancement)

Goal: make the signal feel slightly more alive, present, and “analog” without anyone noticing processing.

**Recommended starting points**

| Parameter              | Value          | Why |
|------------------------|----------------|-----|
| `inputDrive`           | +1.5 … +3 dB   | Gentle push into the nonlinear stages |
| `satDrive`             | 8 … 18         | Just enough harmonics to add density |
| `satType`              | 0 or 1         | FET-ish or mild tape |
| `compThreshold`        | −12 … −8 dB    | Light compression only on peaks |
| `compRatio`            | 4              | Softest 1176 ratio |
| `compMix`              | 25 … 40 %      | Parallel compression for glue without pumping |
| `compAttack`           | 50–100 µs      | Slightly slower than fastest setting |
| `tape bias`            | 55–70          | Higher bias = cleaner, less distortion |
| `tape drive`           | +1 … +3 dB     | Tiny amount of tape compression |
| `driftAmt` / `humAmt`  | 5–12           | Barely audible movement and 60 Hz warmth |
| `noiseAmt` / `hiss`    | 3–8            | Just enough floor to feel “live” |
| `oversample`           | 4× or 8×       | Clean enough for subtle work |

**Signal-flow tips for tiny boost**

1. Start with the channel strip only.
2. Set input drive so the FET compressor just starts working on the loudest transients (gain reduction meter flickers 0.5–1.5 dB).
3. Add a very small amount of post-EQ saturation.
4. On the master bus, use the tape emulator at 30 IPS with high bias and low drive. This adds glue and a gentle low-end bump without obvious saturation.
5. Leave wow/flutter near zero. Any audible pitch modulation destroys the “subtle” illusion.

**Result**: the track feels denser, slightly warmer, and more three-dimensional while remaining clean and modern.

---

### 2. Making It Sound Overpowered

Goal: aggressive, saturated, larger-than-life, “this console is on fire” character. Useful for rock, hip-hop, electronic, or any mix that needs attitude.

**Aggressive parameter zones**

| Parameter              | Aggressive Range     | Effect |
|------------------------|----------------------|--------|
| `inputDrive`           | +9 … +18 dB          | Hard push into FET + saturation |
| `satDrive`             | 45 … 90              | Heavy harmonic generation |
| `satType`              | 2 or 3               | Triode or fuzz-like behaviour |
| `compThreshold`        | −22 … −28 dB         | Deep into the compressor |
| `compRatio`            | 12 or 20             | Hard limiting character |
| `compAttack`           | 20–40 µs             | Fastest attack = more “bite” |
| `compMix`              | 70–100 %             | Heavy parallel or full wet |
| `tape bias`            | 20–40                | Low bias = more distortion + HF |
| `tape drive`           | +8 … +15 dB          | Heavy tape compression + saturation |
| `tape speed`           | 15 IPS               | More head bump and character |
| `wow` / `flutter`      | 15–40                | Audible movement and “broken” vibe |
| `driftAmt`             | 25–50                | Slow gain wandering |
| `humAmt`               | 20–40                | Strong 60 Hz + harmonics |
| `noiseAmt` / `hiss`    | 25–60                | Obvious noise floor / grit |
| `wear`                 | 40–80                | Extra HF loss + hiss |

**Techniques for maximum overpowered character**

1. **Drive stacking**  
   High input drive → aggressive FET → high post-EQ satDrive → low-bias tape. Each stage adds its own harmonic signature. The cumulative effect is far more complex than a single saturator.

2. **Parallel path extremes**  
   Run one instance of the channel strip almost clean (tiny boost settings) and another heavily overdriven. Blend them. This gives both clarity and aggression at the same time.

3. **Tape as the final destroyer**  
   After the channel strip, slam the master tape emulator:
   - 15 IPS
   - Bias ~30
   - Drive +12 dB or higher
   - Compression macro 60–80
   - Moderate wow + flutter for movement
   - Hiss and wear for texture

4. **Asymmetry and bias**  
   The FET and tape models both contain asymmetric bias terms. Pushing the signal hard makes the even-order harmonics more prominent, which the ear often interprets as “warmth” or “power”.

5. **Controlled noise**  
   A small amount of modelled hum and noise makes the saturation feel like real hardware under stress instead of a clean digital clipper. Too much becomes a noise generator — keep it musical.

6. **Oversampling still matters**  
   Even when going aggressive, keep at least 4× (preferably 8× or 16×) oversampling on the nonlinear stages. Aliasing turns “powerful” into “harsh and digital”.

**Quick “overpowered” preset recipe**

```
Channel Strip:
  inputDrive     = +12 dB
  satDrive       = 65
  satType        = 2 (triode)
  compThreshold  = −24 dB
  compRatio      = 12
  compAttack     = 25 µs
  compMix        = 85 %
  driftAmt       = 30
  humAmt         = 25
  noiseAmt       = 20
  oversample     = 8×

Master Tape:
  formula        = 456 or GP9
  speed          = 15 IPS
  bias           = 30
  drive          = +10 dB
  compression    = 70
  wow            = 25
  flutter        = 15
  hiss           = 35
  wear           = 50
  oversample     = 16×
```

---

### 3. Practical Workflow Recommendations

1. **Always start clean**  
   Get the mix balanced with neutral plugins first. Only then add the analog character stages.

2. **Gain staging is everything**  
   The models respond dramatically to input level. A 3 dB change in input drive can move you from “subtle” to “destroyed”.

3. **Use the tolerance channels**  
   Assign different `channelIndex` values (0–47) to different tracks. The tiny mismatches in gain, bias and threshold create a more realistic console feel.

4. **A/B with true bypass**  
   Because the plugins add level and density, match loudness when comparing. Otherwise the louder version always “wins”.

5. **CPU vs quality**  
   16× oversampling is expensive. For tracking or many instances, drop to 4×. For the final mix bus, use the highest factor you can afford.

---

### 4. File Layout

```
analog-character-dsp/
├── README.md                 ← this manual
├── LICENSE
├── CMakeLists.txt
├── dsp/
│   ├── KaiserFIR.h
│   ├── Utils.h
│   ├── AnalogImperfections.h
│   ├── FETCompressor.h
│   ├── TapeShaper.h
│   ├── Biquad.h
│   ├── TapeEQ.h
│   ├── TapeTransport.h
│   ├── ChannelStripDSP.h
│   └── TapeDSP.h
└── docs/
    └── signal-flow.md
```

---

### 5. Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

The DSP core has zero dependencies beyond the C++17 standard library. JUCE (or any other plugin framework) is only needed if you want to turn the modules into actual VST3/AU plugins.

---

### License

MIT. Use freely in commercial or non-commercial projects.
