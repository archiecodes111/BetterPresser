# BetterPresser: User Guide & Operations Manual

**Author:** Archie DSP  
**Product:** BetterPresser (VST3 & Standalone Audio Plugin)  
**Version:** 0.0.1 (64-bit Audio DSP Engine)  
**Classification:** Academic Assignment Submission Deliverable  

---

## 1. Overview & Introduction

**BetterPresser** is a high-precision, 64-bit transparent digital audio compressor designed for modern mixing, mastering, and live sound workflows. Combining an intuitive vintage hardware console aesthetic inspired by classic analog processors with mathematical digital transparency, BetterPresser delivers clean dynamic control without unwanted harmonic distortion or coloration.

### Key Highlights
- **Transparent Dynamic Control**: Clean feed-forward gain computer with continuous second-order polynomial soft-knee.
- **Dual Detection Engines**: Instantaneous **Peak** detection for aggressive transient capture vs. continuous **RMS** energy integration for smooth vocal and bus leveling.
- **Dual-Mode Visualizer Display**:
  - **Analog VU Meter**: Illuminated vintage dial with physical 2nd-order mass-spring-damper needle ballistics.
  - **Live Scrolling Graph**: Smooth, eye-comfortable waveform visualizer displaying Input, Output, and Gain Reduction curves over a ~3.3-second history window.
- **Precision Sidechain High-Pass Filter**: Prevents sub-bass frequencies from causing unnatural compression pumping, with a dedicated **Sidechain Listen** monitor mode.
- **Parallel Compression**: Instant dry/wet blending for New York style parallel compression.
- **Real-Time Metering**: Vertical stereo Input and Output level meters with peak hold bars and clip warning indicators.

---

## 2. Installation & DAW Compatibility

### File Formats & Locations
- **VST3 Plugin Path (Windows)**:
  `C:\Program Files\Common Files\VST3\BetterPresser.vst3`
- **Standalone Application**:
  `build/BetterPresser_artefacts/Release/Standalone/BetterPresser.exe`

### Supported Digital Audio Workstations (DAWs)
BetterPresser complies with the VST3 standard and is compatible with:
- **Ableton Live** (10, 11, 12)
- **Cockos Reaper** (6.x, 7.x)
- **FL Studio** (20, 21, 24)
- **Apple Logic Pro** (via VST3 wrapper / Standalone)
- **Steinberg Cubase / Nuendo**
- **PreSonus Studio One** (5, 6)
- **Bitwig Studio**

---

## 3. Interface Tour & Control Layout

```text
+-------------------------------------------------------------------------------+
| BETTERPRESSER  •  CLEAN DYNAMICS ENGINE          [PEAK] [RMS]   [SC LISTEN]   |
+-------------------------------------------------------------------------------+
|        |                                                     |        |       |
|  [IN]  |     +-----------------------------------------+     |  [OUT] |       |
|  METER |     |        [ METER ]       [ GRAPH ]        |     |  METER |       |
|  -60dB |     |                                         |     |  -60dB |       |
|   to   |     |    ( ANALOG VU METER / LIVE GRAPH )     |     |   to   |       |
|  +6dB  |     |                                         |     |  +6dB  |       |
|        |     +-----------------------------------------+     |        |       |
|        |                                                     |        |       |
| [INPUT]|  [THRESHOLD]  [RATIO]   [MAKE UP]   [SC HPF]  [AUTO GAIN]   | [MIX]  |
|  GAIN  |  [  KNEE   ]  [ATTACK]  [RELEASE]   [ MIX  ]  [AUTO REL ]   |[OUTPUT]|
+-------------------------------------------------------------------------------+
```

### 1. Top Navigation Bar
- **Branding**: BetterPresser 64-bit precision engine identifier.
- **Detection Selector (`PEAK` / `RMS`)**: Toggles the sidechain envelope detector between instantaneous peak rectification and continuous root-mean-square energy integration.
- **`SC LISTEN` (Sidechain Listen)**: Toggles monitoring of the sidechain filtered audio directly, allowing you to audition the exact frequency content triggering the compressor.

### 2. Central Display Bezel
- **`Meter` Mode**: Displays an illuminated vintage VU Meter showing real-time Gain Reduction from $0\text{ dB}$ down to $-30\text{ dB}$, with a $+3\text{ dB}$ redline overload zone and physical needle inertia.
- **`Graph` Mode**: Displays a high-resolution scrolling oscilloscope visualizer showing incoming Dry audio (slate blue), Wet compressed audio (neon cyan), and downward Gain Reduction (amber fill).

### 3. Level Meters & Trim
- **Left Column**: High-resolution Input level meter ($-60\text{ dB}$ to $+6\text{ dB}$) + `INPUT` gain knob ($-24\text{ dB}$ to $+24\text{ dB}$).
- **Right Column**: High-resolution Output level meter ($-60\text{ dB}$ to $+6\text{ dB}$) + `OUTPUT` master gain knob ($-24\text{ dB}$ to $+24\text{ dB}$).

---

## 4. Parameter Guide & Technical Reference

| Control | Parameter ID | Range | Default | Description |
| :--- | :--- | :--- | :--- | :--- |
| **Detection Mode** | `detectionMode` | Peak / RMS | `Peak` | Selects instantaneous peak or energy-averaged RMS detection. |
| **Threshold** | `threshold` | $-60.0\text{ dB}$ to $0.0\text{ dB}$ | `-20.0 dB` | Sets the decibel level at which compression begins. |
| **Ratio** | `ratio` | $1.0:1$ to $30.0:1$ | `4.0:1` | Determines how much gain reduction is applied to signals exceeding the threshold. |
| **Attack** | `attack` | $0.1\text{ ms}$ to $200.0\text{ ms}$ | `15.0 ms` | Controls how rapidly gain reduction engages when a transient crosses the threshold. |
| **Release** | `release` | $5.0\text{ ms}$ to $2000.0\text{ ms}$ | `100.0 ms` | Controls how quickly the compressor returns to unity gain after the signal falls below threshold. |
| **Auto Release** | `autoRelease` | Off / On | `Off` | Adapts recovery speed dynamically (fast for short transients, smooth for sustained signals). |
| **Knee** | `knee` | $0.0\text{ dB}$ to $24.0\text{ dB}$ | `6.0 dB` | Sets the width of the polynomial transition zone around the threshold for transparent compression. |
| **SC HPF** | `sidechainHPF` | $20\text{ Hz}$ to $500\text{ Hz}$ | `20 Hz (Off)`| 2nd-order high-pass filter on the sidechain path to prevent bass/kick pumping. |
| **SC Listen** | `sidechainListen`| Off / On | `Off` | Routes the sidechain filtered signal to the main outputs for monitoring. |
| **Make Up** | `makeUpGain` | $-24.0\text{ dB}$ to $+24.0\text{ dB}$ | `0.0 dB` | Manual gain compensation applied to the compressed wet signal. |
| **Auto Gain** | `autoGain` | Off / On | `Off` | Automatically calculates and applies optimal makeup gain based on threshold and ratio. |
| **Mix** | `mix` | $0.0\%$ to $100.0\%$ | `100.0%` | Crossfades between unmodified dry input ($0\%$) and processed compressed audio ($100\%$). |
| **Input Gain** | `inputGain` | $-24.0\text{ dB}$ to $+24.0\text{ dB}$ | `0.0 dB` | Pre-compression input trim. |
| **Output Gain** | `outputGain` | $-24.0\text{ dB}$ to $+24.0\text{ dB}$ | `0.0 dB` | Post-processing master output level control. |

---

## 5. Practical Mixing Recipes & Presets

### 1. Punchy Snare & Drum Bus
- **Detection**: `Peak`
- **Threshold**: `-18.0 dB`
- **Ratio**: `4.0:1`
- **Attack**: `25.0 ms` (lets initial transient punch through)
- **Release**: `80.0 ms` (musical recovery between drum hits)
- **Knee**: `3.0 dB`
- **SC HPF**: `60.0 Hz` (ignores deep sub-thump)
- **Mix**: `100%`

### 2. Transparent Lead Vocal Leveling
- **Detection**: `RMS`
- **Threshold**: `-24.0 dB`
- **Ratio**: `2.5:1`
- **Attack**: `10.0 ms`
- **Release**: `150.0 ms`
- **Auto Release**: `On`
- **Knee**: `12.0 dB` (ultra-smooth transition)
- **Auto Gain**: `On`
- **SC HPF**: `100.0 Hz` (filters mic handling and plosives)

### 3. Master Bus Glue
- **Detection**: `RMS`
- **Threshold**: `-12.0 dB`
- **Ratio**: `1.5:1` to `2.0:1`
- **Attack**: `30.0 ms`
- **Release**: `100.0 ms`
- **Knee**: `6.0 dB`
- **SC HPF**: `85.0 Hz` (prevents kick from clamping the whole mix)
- **Mix**: `100%`
- **Target Gain Reduction**: $1.0\text{ dB}$ to $2.5\text{ dB}$ on VU Meter

### 4. Aggressive New York Parallel Compression
- **Detection**: `Peak`
- **Threshold**: `-32.0 dB`
- **Ratio**: `12.0:1`
- **Attack**: `0.5 ms` (crushes all peaks)
- **Release**: `40.0 ms`
- **Knee**: `0.0 dB` (hard knee)
- **Mix**: `35.0%` (blend crushed energetic body underneath pristine dry drums)

---

## 6. Keyboard & Mouse Shortcuts

- **Fine-Tune Adjustments**: Hold `Shift` while dragging any rotary knob for 10x parameter precision.
- **Reset to Factory Default**: `Double-Click` any knob to instantly return it to its default value.
- **Direct Value Entry**: Double-click labels or use host automation for precise numeric entry.
