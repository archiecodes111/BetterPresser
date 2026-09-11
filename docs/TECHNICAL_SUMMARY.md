# BetterPresser: Technical Summary & DSP Specification

**Developer:** Archie DSP  
**Course:** Audio Software Development  
**Target Plugin Standard:** VST3 / Standalone (JUCE 8 / C++17)  
**Verification Standard:** Tracktion PluginVal (Strictness Level 10)  

---

## 1. Executive Summary

**BetterPresser** is a high-performance digital dynamic range compressor developed in C++ using the JUCE framework. The system is engineered to provide transparent, low-distortion feed-forward compression with continuous second-order polynomial soft-knee transitions, switchable Peak/RMS sidechain detection, dynamic auto-makeup compensation, and zero-allocation real-time audio thread execution. The plugin includes a resolution-independent procedural vector user interface featuring an illuminated analog VU meter with second-order physics-based needle ballistics and a live scrolling compression graph.

---

## 2. DSP Mathematics & Signal Processing Architecture

### Signal Flow:
`Stereo In` &rarr; `Input Gain` &rarr; `Split` &rarr; [`Sidechain 2nd HPF (20Hz - 500Hz)` &rarr; `Detector (Peak/RMS)` &rarr; `Polynomial Gain Computer` &rarr; `Ballistics (Attack/Release in dB)`] &rarr; `Gain Multiplier` &rarr; `Auto/Manual Makeup` &rarr; `Dry/Wet Crossfade` &rarr; `Output Gain` &rarr; `Stereo Out`.

---

### 2.1. Continuous Polynomial Soft-Knee Gain Computer
To eliminate the discontinuous first derivative present in traditional piecewise hard-knee compressors (which introduces high-frequency harmonic distortion artifacts), BetterPresser utilizes a quadratic polynomial interpolation within the knee boundary *W*:

Let *T* be the threshold in dB, *R* the compression ratio, and *W* the knee width in dB. The static output characteristic *y*<sub>G</sub>(*x*<sub>G</sub>) for input level *x*<sub>G</sub> (both in dB) is defined as:

1. **Linear Uncompressed Region** (for *x*<sub>G</sub> &le; *T* &minus; *W*/2):  
   `y_G(x_G) = x_G`

2. **Polynomial Soft-Knee Transition Region** (for *T* &minus; *W*/2 &lt; *x*<sub>G</sub> &le; *T* + *W*/2):  
   `y_G(x_G) = x_G + ((1/R - 1) * (x_G - T + W/2)^2) / (2 * W)`

3. **Linear Compressed Region** (for *x*<sub>G</sub> &gt; *T* + *W*/2):  
   `y_G(x_G) = T + (x_G - T) / R`

**Target Gain Reduction:**  
`GR_target(x_G) = y_G(x_G) - x_G  <= 0 dB`

---

### 2.2. Dual Detection Path (Peak & RMS)
1. **Peak Mode:** Evaluates instantaneous stereo peak amplitude:  
   `x_lin[n] = max(|s_L[n]|, |s_R[n]|)`

2. **True RMS Mode:** Running leaky integrator with &tau;<sub>rms</sub> = 30 ms (&alpha;<sub>rms</sub> = exp(&minus;1 / (&tau;<sub>rms</sub> &middot; f<sub>s</sub>))):  
   `MS[n] = alpha_rms * MS[n-1] + (1 - alpha_rms) * 0.5 * (s_L[n]^2 + s_R[n]^2)`  
   `x_lin[n] = sqrt(max(10^-10, MS[n]))`

---

### 2.3. Decibel-Domain Envelope Ballistics
Applying attack and release filtering in the decibel domain ensures level-independent, constant-rate recovery (in dB/sec):

- **Attack Phase** (when `GR_target[n] < Env_dB[n-1]`):  
  `Env_dB[n] = alpha_att * Env_dB[n-1] + (1 - alpha_att) * GR_target[n]`

- **Release Phase** (when `GR_target[n] >= Env_dB[n-1]`):  
  `Env_dB[n] = alpha_rel * Env_dB[n-1] + (1 - alpha_rel) * GR_target[n]`

Where the discrete time constants are:  
`alpha_att = exp(-1 / (tau_att * f_s))`  
`alpha_rel = exp(-1 / (tau_rel * f_s))`

---

### 2.4. Automatic Makeup Gain Formula
Dynamic mathematical makeup gain compensation:  
`G_auto(dB) = -0.55 * T * (1 - 1/R)   (for T <= 0 dB)`

---

## 3. Real-Time Thread Safety & Software Architecture

- **Zero-Allocation Audio Thread:** All audio buffers, filters, and circular FIFOs are pre-allocated during `prepareToPlay()`. `processBlock()` performs zero heap allocations (`malloc`/`free`/`new`/`delete`).
- **Lock-Free Atomic Metering:** Thread-safe atomic variables (`std::atomic<float>`, `std::memory_order_relaxed`) convey real-time metering data to the 60 Hz GUI thread without thread contention.
- **APVTS Parameter Tree:** Full parameter state management via `AudioProcessorValueTreeState` with XML binary streaming in `getStateInformation()` and `setStateInformation()`.

---

## 4. UI Engine & Physics-Based Needle Ballistics

### 4.1. Second-Order Mass-Spring-Damper Needle Model
The analog VU meter needle position &theta;(t) is driven by an ANSI C16.5 standard 2nd-order differential equation (&zeta; = 0.68, &omega;<sub>n</sub> = 22.0 rad/s):

`d^2 theta / dt^2 + 2 * zeta * omega_n * (d theta / dt) + omega_n^2 * (theta - theta_target) = 0`

Discretized using semi-implicit Euler integration at 60 Hz.

### 4.2. Gaussian-Smoothed Waveform Visualizer
Subsampled at ~6.0 ms intervals (264 samples/point @ 44.1 kHz) and smoothed via a 3-point Gaussian moving average filter:  
`y_smooth[p] = 0.25 * y_raw[p-1] + 0.50 * y_raw[p] + 0.25 * y_raw[p+1]`

---

## 5. Verification & Testing Conformance

| # | Test Description | Input Condition | Measured Output | Status |
| :-: | :--- | :--- | :--- | :-: |
| **01** | Unity Ratio (1:1) | &minus;60 dB to +12 dB range | 0.000000 dB | **PASS** |
| **02** | Hard-Knee Math | Thresh &minus;20 dB, Ratio 4:1, In &minus;8 dB | &minus;9.000000 dB (Exact) | **PASS** |
| **03** | Soft-Knee Continuity | Knee 10 dB, 100 sub-dB points | Continuous C<sup>1</sup> descent | **PASS** |
| **04** | Peak vs. RMS Detection | 1-sample 0 dBFS spike | Peak: &minus;3.0 dB / RMS: 0.0 dB | **PASS** |
| **05** | Ballistics Time Constants | Burst tone + 1s silence | Att: &minus;14.7 dB, Rel: &minus;0.0006 dB | **PASS** |
| **06** | Auto-Makeup Formula | Thresh &minus;20 dB, Ratio 4:1 | +8.250000 dB | **PASS** |
| **07** | Dry/Wet Bit Transparency | Mix = 0% on heavy compression | Max diff = 0.000000 | **PASS** |
| **08** | Sidechain HPF Rejection | 40 Hz sine tone, HPF @ 300 Hz | Attenuated by &gt;16.8 dB | **PASS** |
| **09** | APVTS State XML Recall | 15 mutated parameters | 100% match (&Delta; &lt; 10<sup>&minus;5</sup>) | **PASS** |
| **10** | Real-Time NaN Safety | &plusmn;100.0f (+40 dBFS) @ 96 kHz | Zero NaNs / zero infinities | **PASS** |
| **11** | **Tracktion PluginVal** | **Strictness Level 10** | **0 Errors / 0 Crashes** | **PASS** |

---

## 6. Build Artifacts & Project Structure

- **VST3 Binary:** `build/BetterPresser_artefacts/Release/VST3/BetterPresser.vst3`
- **Standalone Binary:** `build/BetterPresser_artefacts/Release/Standalone/BetterPresser.exe`
- **Unit Test Runner:** `build/Release/BetterPresserTests.exe`
- **Automated Test Script:** `scripts/run_tests.ps1`
- **User Guide PDF:** `docs/User_Guide.pdf`
- **Technical Summary PDF:** `docs/Technical_Summary.pdf`
- **Conformance Test Report:** `TEST_REPORT.txt` &bull; `TEST_REPORT.md`
