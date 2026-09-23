#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/CleanCompressor.h"

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    betterpresser::CleanCompressor& getCompressor() noexcept { return compressor; }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    betterpresser::CleanCompressor compressor;

    // Cached parameter pointers for lock-free audio thread reads
    std::atomic<float>* thresholdParam = nullptr;
    std::atomic<float>* ratioParam = nullptr;
    std::atomic<float>* attackParam = nullptr;
    std::atomic<float>* releaseParam = nullptr;
    std::atomic<float>* autoReleaseParam = nullptr;
    std::atomic<float>* kneeParam = nullptr;
    std::atomic<float>* detectionModeParam = nullptr;
    std::atomic<float>* rmsWindowParam = nullptr;
    std::atomic<float>* sidechainHpfParam = nullptr;
    std::atomic<float>* sidechainListenParam = nullptr;
    std::atomic<float>* makeUpGainParam = nullptr;
    std::atomic<float>* autoGainParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* inputGainParam = nullptr;
    std::atomic<float>* outputGainParam = nullptr;
};
