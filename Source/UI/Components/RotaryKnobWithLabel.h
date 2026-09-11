#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace betterpresser
{

class RotaryKnobWithLabel : public juce::Component
{
public:
    RotaryKnobWithLabel(juce::AudioProcessorValueTreeState& apvts,
                        const juce::String& paramId,
                        const juce::String& labelText,
                        const juce::String& suffix = "");
    ~RotaryKnobWithLabel() override = default;

    void resized() override;
    void paint(juce::Graphics& g) override;

    juce::Slider& getSlider() noexcept { return slider; }

private:
    juce::Slider slider;
    juce::Label titleLabel;
    juce::Label valueLabel;
    juce::String valueSuffix;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    void updateValueText();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnobWithLabel)
};

} // namespace betterpresser
