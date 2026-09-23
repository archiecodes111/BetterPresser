#pragma once

#include "PluginProcessor.h"
#include "UI/LookAndFeel/CustomLookAndFeel.h"
#include "UI/Components/AnalogVUMeter.h"
#include "UI/Components/CompressionGraph.h"
#include "UI/Components/VerticalLevelMeter.h"
#include "UI/Components/RotaryKnobWithLabel.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              private juce::Timer
{
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

private:
    void timerCallback() override;
    void updateDisplayVisibility();

    AudioPluginAudioProcessor &processorRef;
    betterpresser::CustomLookAndFeel customLookAndFeel;

    // Display Area
    betterpresser::AnalogVUMeter vuMeter;
    betterpresser::CompressionGraph compressionGraph;

    // Level Meters
    betterpresser::VerticalLevelMeter inputLevelMeter;
    betterpresser::VerticalLevelMeter outputLevelMeter;

    // Center & Column Knobs
    betterpresser::RotaryKnobWithLabel inputGainKnob;
    betterpresser::RotaryKnobWithLabel thresholdKnob;
    betterpresser::RotaryKnobWithLabel ratioKnob;
    betterpresser::RotaryKnobWithLabel attackKnob;
    betterpresser::RotaryKnobWithLabel releaseKnob;
    betterpresser::RotaryKnobWithLabel kneeKnob;
    betterpresser::RotaryKnobWithLabel rmsWindowKnob;
    betterpresser::RotaryKnobWithLabel makeUpGainKnob;
    betterpresser::RotaryKnobWithLabel sidechainHpfKnob;
    betterpresser::RotaryKnobWithLabel mixKnob;
    betterpresser::RotaryKnobWithLabel outputGainKnob;

    // Buttons & Selectors
    juce::TextButton meterTabButton { "Meter" };
    juce::TextButton graphTabButton { "Graph" };
    juce::TextButton peakModeButton { "Peak" };
    juce::TextButton rmsModeButton { "RMS" };
    juce::ToggleButton autoReleaseButton { "Auto Rel" };
    juce::ToggleButton autoGainButton { "Auto Gain" };
    juce::ToggleButton scListenButton { "SC Listen" };

    // Button Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoReleaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> scListenAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
