#include "RotaryKnobWithLabel.h"
#include "../LookAndFeel/CustomLookAndFeel.h"

namespace betterpresser
{

RotaryKnobWithLabel::RotaryKnobWithLabel(juce::AudioProcessorValueTreeState& apvts,
                                         const juce::String& paramId,
                                         const juce::String& labelText,
                                         const juce::String& suffix)
    : valueSuffix(suffix)
{
    // Slider Setup
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f,
                               juce::MathConstants<float>::pi * 2.8f, true);
    slider.setDoubleClickReturnValue(true, 0.0);
    slider.onValueChange = [this]() { updateValueText(); };
    addAndMakeVisible(slider);

    // Title Label
    titleLabel.setText(labelText, juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(10.5f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::getTextSecondary());
    addAndMakeVisible(titleLabel);

    // Value Label
    valueLabel.setJustificationType(juce::Justification::centred);
    valueLabel.setFont(juce::Font(10.0f, juce::Font::plain));
    valueLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::getTextPrimary());
    addAndMakeVisible(valueLabel);

    // APVTS Attachment
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramId, slider);

    // Initial update
    updateValueText();
}

void RotaryKnobWithLabel::updateValueText()
{
    float val = static_cast<float>(slider.getValue());
    if (valueSuffix.isNotEmpty())
    {
        if (valueSuffix == " dB")
        {
            valueLabel.setText((val > 0.0f ? "+" : "") + juce::String(val, 1) + " dB", juce::dontSendNotification);
        }
        else if (valueSuffix == ":1")
        {
            valueLabel.setText(juce::String(val, 1) + ":1", juce::dontSendNotification);
        }
        else if (valueSuffix == " ms")
        {
            if (val >= 1000.0f)
                valueLabel.setText(juce::String(val * 0.001f, 2) + " s", juce::dontSendNotification);
            else if (val < 1.0f)
                valueLabel.setText(juce::String(val, 2) + " ms", juce::dontSendNotification);
            else
                valueLabel.setText(juce::String(static_cast<int>(val)) + " ms", juce::dontSendNotification);
        }
        else if (valueSuffix == "%")
        {
            valueLabel.setText(juce::String(static_cast<int>(val)) + "%", juce::dontSendNotification);
        }
        else if (valueSuffix == " Hz")
        {
            if (val <= 20.0f)
                valueLabel.setText("Off", juce::dontSendNotification);
            else
                valueLabel.setText(juce::String(static_cast<int>(val)) + " Hz", juce::dontSendNotification);
        }
        else
        {
            valueLabel.setText(juce::String(val, 1) + valueSuffix, juce::dontSendNotification);
        }
    }
    else
    {
        valueLabel.setText(juce::String(val, 1), juce::dontSendNotification);
    }
}

void RotaryKnobWithLabel::resized()
{
    auto bounds = getLocalBounds();
    int labelHeight = 15;
    int valueHeight = 14;

    titleLabel.setBounds(bounds.removeFromTop(labelHeight));
    valueLabel.setBounds(bounds.removeFromBottom(valueHeight));

    // Center slider in remaining area
    slider.setBounds(bounds.reduced(2));
}

void RotaryKnobWithLabel::paint(juce::Graphics& g)
{
    juce::ignoreUnused(g);
}

} // namespace betterpresser
