#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <atomic>

namespace betterpresser
{

class AnalogVUMeter : public juce::Component
{
public:
    AnalogVUMeter();
    ~AnalogVUMeter() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Update needle target from DSP Gain Reduction in dB (0.0 to -30.0 dB)
    void updateGainReduction(float grDb, float dtSeconds = 1.0f / 60.0f);

private:
    float currentNeedlePos = 0.0f; // Normalized 0.0 (-30dB) to 1.0 (0dB)
    float needleVelocity = 0.0f;
    float targetNeedlePos = 1.0f;

    // Ballistics constants (ANSI C16.5 VU standard)
    static constexpr float dampingFactor = 0.68f;
    static constexpr float naturalFrequency = 22.0f;

    static float mapDbToNormalizedPosition(float db) noexcept;
    static float mapNormalizedPositionToAngle(float norm) noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogVUMeter)
};

} // namespace betterpresser
