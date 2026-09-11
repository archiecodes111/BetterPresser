#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <algorithm>

namespace betterpresser
{

class VerticalLevelMeter : public juce::Component
{
public:
    explicit VerticalLevelMeter(const juce::String& titleText = "LEVEL");
    ~VerticalLevelMeter() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setLevel(float peakDb, float rmsDb);

private:
    juce::String title;
    float currentPeakDb = -100.0f;
    float currentRmsDb = -100.0f;
    float peakHoldDb = -100.0f;
    int peakHoldTimer = 0;
    bool isClipping = false;
    int clipHoldTimer = 0;

    static float mapDbToNormalized(float db) noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VerticalLevelMeter)
};

} // namespace betterpresser
