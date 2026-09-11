# BetterPresser 🎛️

A transparent, high-precision 64-bit digital audio compressor plugin built with **JUCE (C++17)** and **CMake**, featuring an analog-style UI inspired by classic hardware dynamics processors.

![BetterPresser Interface Preview](docs/assets/preview.png)

---

## ✨ Features

- **Transparent DSP Dynamics Engine**: Clean feed-forward gain computer with continuous second-order polynomial soft-knee transitions.
- **Dual Detection Modes**:
  - **Peak Mode**: Instantaneous peak tracking for aggressive transient control.
  - **RMS Mode**: Continuous root-mean-square energy integration ($\tau = 30\text{ ms}$) for smooth, transparent vocal and master bus leveling.
- **Dual-Mode Visualizer Display**:
  - **Analog VU Meter**: Illuminated vintage dial with authentic 2nd-order mass-spring-damper physics needle simulation (ANSI C16.5 standard).
  - **Live Scrolling Waveform Graph**: Gaussian-smoothed oscilloscope view showing real-time Input, Output, and Gain Reduction curves (~3.3s history window).
- **Sidechain High-Pass Filter**: 2nd-order State-Variable HPF ($20\text{ Hz} - 500\text{ Hz}$) with dedicated **Sidechain Listen** audition mode.
- **Parallel Compression**: Sample-exact Dry/Wet crossfade for instant parallel processing.
- **Real-Time Thread Safety**: Zero heap allocations on the audio rendering thread; lock-free atomic metering pipeline.
- **Quality Certified**: 10/10 automated DSP unit test suite and **Tracktion PluginVal** validated at Strictness Level 10 (0 errors).

---

## 🎛️ Parameters

| Parameter | Range | Default | Description |
| :--- | :--- | :--- | :--- |
| **Detection Mode** | Peak / RMS | `Peak` | Instantaneous peak vs. energy-averaged RMS detection |
| **Threshold** | $-60.0\text{ dB}$ to $0.0\text{ dB}$ | `-20.0 dB` | Compression onset level |
| **Ratio** | $1.0:1$ to $30.0:1$ | `4.0:1` | Compression slope |
| **Attack** | $0.1\text{ ms}$ to $200.0\text{ ms}$ | `15.0 ms` | Onset response time |
| **Release** | $5.0\text{ ms}$ to $2000.0\text{ ms}$ | `100.0 ms` | Recovery time |
| **Auto Release** | Off / On | `Off` | Program-dependent adaptive release timing |
| **Knee** | $0.0\text{ dB}$ to $24.0\text{ dB}$ | `6.0 dB` | Polynomial soft-knee transition width |
| **SC HPF** | $20\text{ Hz}$ to $500\text{ Hz}$ | `20 Hz (Off)` | Sidechain detection high-pass filter |
| **SC Listen** | Off / On | `Off` | Sidechain monitor mode |
| **Make Up** | $-24.0\text{ dB}$ to $+24.0\text{ dB}$ | `0.0 dB` | Manual makeup gain compensation |
| **Auto Gain** | Off / On | `Off` | Dynamic mathematical makeup gain compensation |
| **Mix** | $0.0\%$ to $100.0\%$ | `100.0%` | Dry/Wet parallel crossfade |
| **Input / Output** | $-24.0\text{ dB}$ to $+24.0\text{ dB}$ | `0.0 dB` | Input and Output trim levels |

---

## 🚀 Building & Testing

### Prerequisites
- Visual Studio Build Tools (C++17) / Clang / GCC
- CMake 3.23+
- Git

### Build Commands
```bash
# 1. Configure CMake
cmake -B build

# 2. Build Release targets (VST3, Standalone & Unit Tests)
cmake --build build --config Release

# 3. Run Automated Tests & PluginVal
powershell -ExecutionPolicy Bypass -File ./scripts/run_tests.ps1
```

---

## 📁 Deliverables & Documentation

- 📄 **User Guide (PDF)**: [`docs/User_Guide.pdf`](docs/User_Guide.pdf)
- 📄 **Technical Summary & DSP Spec (PDF)**: [`docs/Technical_Summary.pdf`](docs/Technical_Summary.pdf)
- 📄 **Conformance Test Report**: [`TEST_REPORT.txt`](TEST_REPORT.txt) / [`TEST_REPORT.md`](TEST_REPORT.md)

---

## 📜 License & Credits

Developed by **Archie DSP** for Audio Software Development. Built with [JUCE](https://juce.com/).
