#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <cmath>

namespace betterpresser
{

enum class DetectionMode
{
    Peak = 0,
    RMS = 1
};

struct VisualizerSample
{
    float inputLevel = 0.0f;
    float outputLevel = 0.0f;
    float gainReductionDb = 0.0f;
};

class CleanCompressor
{
public:
    CleanCompressor();
    ~CleanCompressor() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();

    void setParameters(float thresholdDb,
                       float ratio,
                       float attackMs,
                       float releaseMs,
                       bool autoRelease,
                       float kneeDb,
                       DetectionMode detectionMode,
                       float rmsWindowMs,
                       float sidechainHpfHz,
                       bool sidechainListen,
                       float makeUpGainDb,
                       bool autoGain,
                       float mixPercent,
                       float inputGainDb,
                       float outputGainDb);

    void process(juce::AudioBuffer<float>& buffer);

    // Metering Readouts (Thread-safe atomics)
    float getGainReductionDb() const noexcept { return gainReductionDb.load(std::memory_order_relaxed); }
    float getInputPeakDb() const noexcept { return inputPeakDb.load(std::memory_order_relaxed); }
    float getOutputPeakDb() const noexcept { return outputPeakDb.load(std::memory_order_relaxed); }
    float getInputRmsDb() const noexcept { return inputRmsDb.load(std::memory_order_relaxed); }
    float getOutputRmsDb() const noexcept { return outputRmsDb.load(std::memory_order_relaxed); }

    // Visualizer Ring Buffer
    static constexpr int visualizerBufferSize = 2048;
    int getVisualizerReadIndex() const noexcept { return visualizerWriteIndex.load(std::memory_order_relaxed); }
    const VisualizerSample& getVisualizerSample(int index) const noexcept { return visualizerBuffer[index % visualizerBufferSize]; }

    // Static mathematical helper for unit testing & gain computing
    static float computeStaticGainReductionDb(float inputDb, float thresholdDb, float ratio, float kneeDb);
    static float computeAutoGainDb(float thresholdDb, float ratio);

private:
    double currentSampleRate = 44100.0;
    int currentNumChannels = 2;

    // Parameters
    float paramThresholdDb = -20.0f;
    float paramRatio = 4.0f;
    float paramAttackMs = 15.0f;
    float paramReleaseMs = 100.0f;
    bool paramAutoRelease = false;
    float paramKneeDb = 6.0f;
    DetectionMode paramDetectionMode = DetectionMode::Peak;
    float paramSidechainHpfHz = 20.0f;
    bool paramSidechainListen = false;
    float paramMakeUpGainDb = 0.0f;
    bool paramAutoGain = false;
    float paramMix = 1.0f;
    float paramInputGainDb = 0.0f;
    float paramOutputGainDb = 0.0f;

    // Ballistics Coefficients
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    float envelopeDb = 0.0f;

    // RMS detector state
    float rmsMeanSquare = 0.0f;
    float rmsAlpha = 0.0f;

    // Sidechain State Variable Filter (2nd order HPF)
    juce::dsp::StateVariableTPTFilter<float> sidechainFilter;

    // Smoothed parameter gains
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedInputGain;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedOutputGain;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedMakeupGain;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedMix;

    // Metering
    std::atomic<float> gainReductionDb { 0.0f };
    std::atomic<float> inputPeakDb { -100.0f };
    std::atomic<float> outputPeakDb { -100.0f };
    std::atomic<float> inputRmsDb { -100.0f };
    std::atomic<float> outputRmsDb { -100.0f };

    // Visualizer Ring Buffer
    VisualizerSample visualizerBuffer[visualizerBufferSize];
    std::atomic<int> visualizerWriteIndex { 0 };
    int visualizerSubsampleCounter = 0;
    float visualizerInPeak = 0.0f;
    float visualizerOutPeak = 0.0f;
    float visualizerGrMin = 0.0f;

    void updateFilter();
    void updateBallistics();
    static inline float linearToDb(float linear) noexcept
    {
        return (linear > 1.0e-5f) ? 20.0f * std::log10(linear) : -100.0f;
    }
    static inline float dbToLinear(float db) noexcept
    {
        return std::pow(10.0f, db * 0.05f);
    }
};

} // namespace betterpresser
