#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         ),
      apvts(*this, nullptr, "Parameters", createParameters())
{
    thresholdParam = apvts.getRawParameterValue("threshold");
    ratioParam = apvts.getRawParameterValue("ratio");
    attackParam = apvts.getRawParameterValue("attack");
    releaseParam = apvts.getRawParameterValue("release");
    autoReleaseParam = apvts.getRawParameterValue("autoRelease");
    kneeParam = apvts.getRawParameterValue("knee");
    detectionModeParam = apvts.getRawParameterValue("detectionMode");
    rmsWindowParam = apvts.getRawParameterValue("rmsWindow");
    sidechainHpfParam = apvts.getRawParameterValue("sidechainHPF");
    sidechainListenParam = apvts.getRawParameterValue("sidechainListen");
    makeUpGainParam = apvts.getRawParameterValue("makeUpGain");
    autoGainParam = apvts.getRawParameterValue("autoGain");
    mixParam = apvts.getRawParameterValue("mix");
    inputGainParam = apvts.getRawParameterValue("inputGain");
    outputGainParam = apvts.getRawParameterValue("outputGain");
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Threshold: -60 dB to 0 dB
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "threshold", 1 },
        "Threshold",
        juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f),
        -20.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float val, int) { return juce::String(val, 1) + " dB"; },
        [](const juce::String& text) { return text.getFloatValue(); }));

    // Ratio: 1.0 to 30.0 (skewed to give more precision at lower ratios)
    auto ratioRange = juce::NormalisableRange<float>(1.0f, 30.0f, 0.1f);
    ratioRange.setSkewForCentre(4.0f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "ratio", 1 },
        "Ratio",
        ratioRange,
        4.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float val, int) { return juce::String(val, 1) + ":1"; },
        [](const juce::String& text) { return text.getFloatValue(); }));

    // Attack: 0.1 ms to 200 ms (log skewed)
    auto attackRange = juce::NormalisableRange<float>(0.1f, 200.0f, 0.1f);
    attackRange.setSkewForCentre(15.0f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "attack", 1 },
        "Attack",
        attackRange,
        15.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float val, int) { return (val < 1.0f) ? (juce::String(val, 2) + " ms") : (juce::String(val, 1) + " ms"); },
        [](const juce::String& text) { return text.getFloatValue(); }));

    // Release: 5 ms to 2000 ms (log skewed)
    auto releaseRange = juce::NormalisableRange<float>(5.0f, 2000.0f, 1.0f);
    releaseRange.setSkewForCentre(100.0f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "release", 1 },
        "Release",
        releaseRange,
        100.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float val, int) { return (val >= 1000.0f) ? (juce::String(val * 0.001f, 2) + " s") : (juce::String(static_cast<int>(val)) + " ms"); },
        [](const juce::String& text) { return text.getFloatValue(); }));

    // Auto-Release Toggle
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "autoRelease", 1 },
        "Auto Release",
        false));

    // Knee: 0 dB to 24 dB
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "knee", 1 },
        "Knee",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f),
        6.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float val, int) { return juce::String(val, 1) + " dB"; },
        [](const juce::String& text) { return text.getFloatValue(); }));

    // Detection Mode: 0 = Peak, 1 = RMS
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "detectionMode", 1 },
        "Detection Mode",
        juce::StringArray { "Peak", "RMS" },
        0));

    // RMS Window: 1 ms to 500 ms (log skewed)
    auto rmsRange = juce::NormalisableRange<float>(1.0f, 500.0f, 1.0f);
    rmsRange.setSkewForCentre(30.0f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "rmsWindow", 1 },
        "RMS Window",
        rmsRange,
        30.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float val, int) { return juce::String(static_cast<int>(val)) + " ms"; },
        [](const juce::String& text) { return text.getFloatValue(); }));

    // Sidechain HPF: 20 Hz to 500 Hz
    auto hpfRange = juce::NormalisableRange<float>(20.0f, 500.0f, 1.0f);
    hpfRange.setSkewForCentre(80.0f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "sidechainHPF", 1 },
        "SC HPF",
        hpfRange,
        20.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float val, int) { return (val <= 20.0f) ? "Off" : (juce::String(static_cast<int>(val)) + " Hz"); },
        [](const juce::String& text) { return text.getFloatValue(); }));

    // Sidechain Listen Toggle
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "sidechainListen", 1 },
        "SC Listen",
        false));

    // Make Up Gain: -24 dB to +24 dB
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "makeUpGain", 1 },
        "Make Up",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float val, int) { return (val > 0.0f ? "+" : "") + juce::String(val, 1) + " dB"; },
        [](const juce::String& text) { return text.getFloatValue(); }));

    // Auto Gain Toggle
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "autoGain", 1 },
        "Auto Gain",
        false));

    // Mix (Dry / Wet): 0% to 100%
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "mix", 1 },
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        100.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float val, int) { return juce::String(static_cast<int>(val)) + "%"; },
        [](const juce::String& text) { return text.getFloatValue(); }));

    // Input Gain: -24 dB to +24 dB
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "inputGain", 1 },
        "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float val, int) { return (val > 0.0f ? "+" : "") + juce::String(val, 1) + " dB"; },
        [](const juce::String& text) { return text.getFloatValue(); }));

    // Output Gain: -24 dB to +24 dB
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "outputGain", 1 },
        "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float val, int) { return (val > 0.0f ? "+" : "") + juce::String(val, 1) + " dB"; },
        [](const juce::String& text) { return text.getFloatValue(); }));

    // Display Mode: 0 = Meter, 1 = Graph
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "displayMode", 1 },
        "Display Mode",
        juce::StringArray { "Meter", "Graph" },
        0));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    compressor.prepare(sampleRate, samplesPerBlock, getTotalNumInputChannels());
}

void AudioPluginAudioProcessor::releaseResources()
{
    compressor.reset();
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                             juce::MidiBuffer &midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Update DSP parameters lock-free
    if (thresholdParam != nullptr)
    {
        compressor.setParameters(
            thresholdParam->load(std::memory_order_relaxed),
            ratioParam->load(std::memory_order_relaxed),
            attackParam->load(std::memory_order_relaxed),
            releaseParam->load(std::memory_order_relaxed),
            autoReleaseParam->load(std::memory_order_relaxed) > 0.5f,
            kneeParam->load(std::memory_order_relaxed),
            static_cast<betterpresser::DetectionMode>(static_cast<int>(detectionModeParam->load(std::memory_order_relaxed))),
            rmsWindowParam->load(std::memory_order_relaxed),
            sidechainHpfParam->load(std::memory_order_relaxed),
            sidechainListenParam->load(std::memory_order_relaxed) > 0.5f,
            makeUpGainParam->load(std::memory_order_relaxed),
            autoGainParam->load(std::memory_order_relaxed) > 0.5f,
            mixParam->load(std::memory_order_relaxed),
            inputGainParam->load(std::memory_order_relaxed),
            outputGainParam->load(std::memory_order_relaxed)
        );
    }

    compressor.process(buffer);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor *AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

//==============================================================================
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
