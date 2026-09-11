# BetterPresser v0.0.1 &mdash; Initial Release 🎛️

**BetterPresser** is a transparent, high-precision 64-bit digital dynamic range compressor plugin built with **JUCE (C++17)** and **CMake**. It combines an analog-inspired console aesthetic with zero-coloration mathematical digital dynamics.

---

## ✨ What's New & Key Highlights

### ⚡ Clean DSP Dynamics Engine
- **Continuous 2nd-Order Polynomial Soft-Knee**: Smooth mathematical transition ($0.0\text{ dB} - 24.0\text{ dB}$) eliminating harmonic distortion and clicks.
- **Dual Detection Modes**:
  - **Peak Mode**: Instantaneous peak tracking for aggressive transient containment.
  - **RMS Mode**: Continuous energy integration ($\tau = 30\text{ ms}$) for smooth, transparent vocal and master bus leveling.
- **Sidechain High-Pass Filter**: 2nd-order State-Variable HPF ($20\text{ Hz} - 500\text{ Hz}$) with dedicated **SC Listen** monitor mode.
- **Auto-Makeup Gain**: Dynamic mathematical compensation for threshold/ratio loudness loss.
- **Dry/Wet Parallel Compression**: Sample-exact crossfading for instant New York style parallel processing.
- **Zero-Allocation Real-Time Audio**: Lock-free atomic metering pipeline with zero heap allocations on the audio thread.

### 🎛️ Vector UI & Visualizers
- **Brushed Aluminum Vector Knobs**: Crisp, resolution-independent vector rendering with glowing cyan active arc rings and fine-tuning (`Shift+Drag`).
- **Analog VU Meter**: Vintage illuminated dial with authentic 2nd-order mass-spring-damper physics needle simulation ($-30\text{ dB}$ to $+3\text{ dB}$).
- **Live Scrolling Waveform Visualizer**: Gaussian-smoothed oscilloscope view showing real-time Input, Output, and Gain Reduction curves (~3.3s time window).
- **Stereo Level Meters**: Vertical Input and Output dB meters with peak hold bars and clip warning LEDs.

---

## 🧪 Quality & Test Conformance
- **10/10 DSP Unit Test Suite**: All gain computing, soft-knee continuity, ballistics, and state recall tests passed.
- **Tracktion PluginVal Certified**: Validated at **Strictness Level 10** across multiple sample rates and buffer sizes with **0 errors and 0 warnings**.

---

## 📥 Installation Instructions

### VST3 (Windows 64-bit)
1. Download and extract **`BetterPresser-v0.0.1-VST3.zip`**.
2. Copy the **`BetterPresser.vst3`** folder to your system VST3 directory:
   ```text
   C:\Program Files\Common Files\VST3\
   ```
3. Rescan plugins inside your DAW (Reaper, Ableton, FL Studio, Cubase, Studio One, etc.).

### Standalone Application
- Simply run **`BetterPresser.exe`** directly &mdash; no DAW required!

---

## 📄 Documentation & Deliverables Included
- 📄 **User Guide (PDF)**: Detailed operations manual, parameter tables, and mixing recipes.
- 📄 **Technical Summary & DSP Specification (PDF)**: Mathematical equations, block diagrams, and thread safety analysis.
- 📄 **Test & Validation Conformance Report**: Complete 10/10 unit test breakdown and PluginVal score.
