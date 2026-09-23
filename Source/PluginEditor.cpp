#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      compressionGraph(p.getCompressor()),
      inputLevelMeter("IN"),
      outputLevelMeter("OUT"),
      inputGainKnob(p.getAPVTS(), "inputGain", "INPUT", " dB"),
      thresholdKnob(p.getAPVTS(), "threshold", "THRESHOLD", " dB"),
      ratioKnob(p.getAPVTS(), "ratio", "RATIO", ":1"),
      attackKnob(p.getAPVTS(), "attack", "ATTACK", " ms"),
      releaseKnob(p.getAPVTS(), "release", "RELEASE", " ms"),
      kneeKnob(p.getAPVTS(), "knee", "KNEE", " dB"),
      rmsWindowKnob(p.getAPVTS(), "rmsWindow", "RMS TIME", " ms"),
      makeUpGainKnob(p.getAPVTS(), "makeUpGain", "MAKE UP", " dB"),
      sidechainHpfKnob(p.getAPVTS(), "sidechainHPF", "SC HPF", " Hz"),
      mixKnob(p.getAPVTS(), "mix", "MIX", "%"),
      outputGainKnob(p.getAPVTS(), "outputGain", "OUTPUT", " dB")
{
    juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypefaceName("Segoe UI");
    setLookAndFeel(&customLookAndFeel);

    // Display Area
    addAndMakeVisible(vuMeter);
    addChildComponent(compressionGraph); // Hidden by default if displayMode is 0

    // Meters
    addAndMakeVisible(inputLevelMeter);
    addAndMakeVisible(outputLevelMeter);

    // Knobs
    addAndMakeVisible(inputGainKnob);
    addAndMakeVisible(thresholdKnob);
    addAndMakeVisible(ratioKnob);
    addAndMakeVisible(attackKnob);
    addAndMakeVisible(releaseKnob);
    addAndMakeVisible(kneeKnob);
    addAndMakeVisible(rmsWindowKnob);
    addAndMakeVisible(makeUpGainKnob);
    addAndMakeVisible(sidechainHpfKnob);
    addAndMakeVisible(mixKnob);
    addAndMakeVisible(outputGainKnob);

    // Display Switch Buttons
    meterTabButton.setRadioGroupId(101);
    meterTabButton.setClickingTogglesState(true);
    meterTabButton.setToggleState(true, juce::dontSendNotification);
    meterTabButton.onClick = [this]() {
        if (auto* param = processorRef.getAPVTS().getParameter("displayMode"))
            param->setValueNotifyingHost(0.0f);
        updateDisplayVisibility();
    };
    addAndMakeVisible(meterTabButton);

    graphTabButton.setRadioGroupId(101);
    graphTabButton.setClickingTogglesState(true);
    graphTabButton.onClick = [this]() {
        if (auto* param = processorRef.getAPVTS().getParameter("displayMode"))
            param->setValueNotifyingHost(1.0f);
        updateDisplayVisibility();
    };
    addAndMakeVisible(graphTabButton);

    // Peak / RMS Buttons
    peakModeButton.setRadioGroupId(102);
    peakModeButton.setClickingTogglesState(true);
    peakModeButton.setToggleState(true, juce::dontSendNotification);
    peakModeButton.onClick = [this]() {
        DBG("PEAK MODE CLICKED");
        if (auto* param = processorRef.getAPVTS().getParameter("detectionMode"))
            param->setValueNotifyingHost(0.0f);
    };
    addAndMakeVisible(peakModeButton);

    rmsModeButton.setRadioGroupId(102);
    rmsModeButton.setClickingTogglesState(true);
    rmsModeButton.onClick = [this]() {
        DBG("RMS MODE CLICKED");
        if (auto* param = processorRef.getAPVTS().getParameter("detectionMode"))
            param->setValueNotifyingHost(1.0f);
    };
    addAndMakeVisible(rmsModeButton);

    // Toggle Buttons & Attachments
    addAndMakeVisible(autoReleaseButton);
    addAndMakeVisible(autoGainButton);
    addAndMakeVisible(scListenButton);

    autoReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.getAPVTS(), "autoRelease", autoReleaseButton);
    autoGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.getAPVTS(), "autoGain", autoGainButton);
    scListenAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processorRef.getAPVTS(), "sidechainListen", scListenButton);

    // Sync initial button states
    if (auto* detParam = processorRef.getAPVTS().getRawParameterValue("detectionMode"))
    {
        bool isRms = detParam->load() > 0.5f;
        peakModeButton.setToggleState(!isRms, juce::dontSendNotification);
        rmsModeButton.setToggleState(isRms, juce::dontSendNotification);
    }

    if (auto* dispParam = processorRef.getAPVTS().getRawParameterValue("displayMode"))
    {
        bool isGraph = dispParam->load() > 0.5f;
        meterTabButton.setToggleState(!isGraph, juce::dontSendNotification);
        graphTabButton.setToggleState(isGraph, juce::dontSendNotification);
        updateDisplayVisibility();
    }

    // Set Editor Window Size
    setSize(780, 500);

    // 60 FPS Timer for Smooth Needle Ballistics & Metering
    startTimerHz(60);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void AudioPluginAudioProcessorEditor::updateDisplayVisibility()
{
    bool isGraph = graphTabButton.getToggleState();
    vuMeter.setVisible(!isGraph);
    compressionGraph.setVisible(isGraph);
}

void AudioPluginAudioProcessorEditor::timerCallback()
{
    auto& comp = processorRef.getCompressor();

    // 1. Update Analog VU Meter Needle
    vuMeter.updateGainReduction(comp.getGainReductionDb(), 1.0f / 60.0f);

    // 2. Update Live Waveform Graph
    if (compressionGraph.isVisible())
    {
        compressionGraph.updateGraph();
    }

    // 3. Update Input & Output Level Meters
    inputLevelMeter.setLevel(comp.getInputPeakDb(), comp.getInputRmsDb());
    outputLevelMeter.setLevel(comp.getOutputPeakDb(), comp.getOutputRmsDb());

    // 4. Sync display & detection buttons if host automated
    if (auto* detParam = processorRef.getAPVTS().getRawParameterValue("detectionMode"))
    {
        bool isRms = detParam->load() > 0.5f;
        if (rmsModeButton.getToggleState() != isRms)
        {
            peakModeButton.setToggleState(!isRms, juce::dontSendNotification);
            rmsModeButton.setToggleState(isRms, juce::dontSendNotification);
        }
    }

    if (auto* dispParam = processorRef.getAPVTS().getRawParameterValue("displayMode"))
    {
        bool isGraph = dispParam->load() > 0.5f;
        if (graphTabButton.getToggleState() != isGraph)
        {
            meterTabButton.setToggleState(!isGraph, juce::dontSendNotification);
            graphTabButton.setToggleState(isGraph, juce::dontSendNotification);
            updateDisplayVisibility();
        }
    }
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics &g)
{
    auto bounds = getLocalBounds().toFloat();

    // 1. Brushed Metal Slate Chassis Gradient
    juce::ColourGradient chassisGrad(
        betterpresser::CustomLookAndFeel::getChassisGradientTop(), 0.0f, 0.0f,
        betterpresser::CustomLookAndFeel::getChassisGradientBottom(), 0.0f, bounds.getHeight(), false);
    g.setGradientFill(chassisGrad);
    g.fillRect(bounds);

    // 2. Top Bar
    auto topBarBounds = bounds.removeFromTop(44.0f);
    g.setColour(juce::Colour(0xff0e161e));
    g.fillRect(topBarBounds);

    g.setColour(juce::Colour(0xff223547));
    g.drawHorizontalLine(44, 0.0f, bounds.getWidth());

    // Title Logo & Engraved Branding
    g.setFont(juce::Font(15.0f, juce::Font::bold));
    g.setColour(juce::Colours::white);
    g.drawText("BETTERPRESSER", 20, 10, 180, 16, juce::Justification::left, false);

    g.setFont(juce::Font(9.5f, juce::Font::plain));
    g.setColour(betterpresser::CustomLookAndFeel::getTextSecondary());
    g.drawText("CLEAN DYNAMICS ENGINE", 20, 26, 180, 14, juce::Justification::left, false);

    // 3. Subtle Panel Dividers & Outlines
    g.setColour(juce::Colour(0xff233545).withAlpha(0.5f));

    // Left Column Separator
    g.drawVerticalLine(100, 52.0f, bounds.getHeight() - 10.0f);

    // Right Column Separator
    g.drawVerticalLine(static_cast<int>(bounds.getWidth()) - 100, 52.0f, bounds.getHeight() - 10.0f);

    // Bottom Footer Label
    g.setFont(juce::Font(9.0f, juce::Font::plain));
    g.setColour(betterpresser::CustomLookAndFeel::getTextSecondary().withAlpha(0.6f));
    g.drawText("ARCHIE DSP  •  64-BIT PRECISION", 0, getHeight() - 18, getWidth(), 14,
               juce::Justification::centred, false);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // Fixed reference canvas: 780 x 500
    const int totalWidth = getWidth();
    const int totalHeight = getHeight();

    // Top Bar Selectors
    // Detection Mode (Peak / RMS)
    peakModeButton.setBounds(totalWidth - 230, 10, 48, 24);
    rmsModeButton.setBounds(totalWidth - 178, 10, 48, 24);
    scListenButton.setBounds(totalWidth - 115, 10, 95, 24);

    // Left Column: Input Meter & Input Gain (Center X = 50)
    inputLevelMeter.setBounds(42, 56, 40, 290);
    inputGainKnob.setBounds(10, 360, 80, 88);

    // Right Column: Output Meter & Output Gain (Center X = totalWidth - 50)
    outputLevelMeter.setBounds(totalWidth - 58, 56, 40, 290);
    outputGainKnob.setBounds(totalWidth - 90, 360, 80, 88);

    // Center Display Area (VU Meter / Compression Graph)
    const int centerDisplayX = 116;
    const int centerDisplayY = 56;
    const int centerDisplayW = totalWidth - 232; // ~548 px
    const int centerDisplayH = 175;

    vuMeter.setBounds(centerDisplayX, centerDisplayY, centerDisplayW, centerDisplayH);
    compressionGraph.setBounds(centerDisplayX, centerDisplayY, centerDisplayW, centerDisplayH);

    // Display Tab Switch Buttons (Inside Display Top-Center)
    meterTabButton.setBounds(centerDisplayX + centerDisplayW / 2 - 58, centerDisplayY + 6, 56, 20);
    graphTabButton.setBounds(centerDisplayX + centerDisplayW / 2 + 2, centerDisplayY + 6, 56, 20);

    // Main Knob Grid (Center Lower Area)
    const int knobSpacingX = 76;
    const int knobWidth = 80;
    const int knobHeight = 88;

    // Center display spans X: 116 to 664 (width 548). 
    // Total width of Row 1 (5 knobs + 1 button) = (4 * 76) + 80 + 76 + 84 = 464.
    // Offset to center = (548 - 464) / 2 = 42.
    // Base X = 116 + 42 = 158.
    const int row1StartX = 158;
    const int row2StartX = row1StartX + (knobSpacingX / 2); // 196 (staggers the 4 knobs perfectly under the 5 knobs)
    const int buttonX = row1StartX + (knobSpacingX * 5); // 538

    // Row 1 (Y = 246): Threshold | Ratio | Knee | MakeUp | SC HPF | AutoGain Btn
    const int row1Y = 246;
    thresholdKnob.setBounds(row1StartX, row1Y, knobWidth, knobHeight);
    ratioKnob.setBounds(row1StartX + knobSpacingX, row1Y, knobWidth, knobHeight);
    kneeKnob.setBounds(row1StartX + knobSpacingX * 2, row1Y, knobWidth, knobHeight);
    makeUpGainKnob.setBounds(row1StartX + knobSpacingX * 3, row1Y, knobWidth, knobHeight);
    sidechainHpfKnob.setBounds(row1StartX + knobSpacingX * 4, row1Y, knobWidth, knobHeight);
    autoGainButton.setBounds(buttonX, row1Y + 28, 84, 28);

    // Row 2 (Y = 348): Attack | Release | RMS Time | Mix | AutoRelease Btn
    const int row2Y = 348;
    attackKnob.setBounds(row2StartX, row2Y, knobWidth, knobHeight);
    releaseKnob.setBounds(row2StartX + knobSpacingX, row2Y, knobWidth, knobHeight);
    rmsWindowKnob.setBounds(row2StartX + knobSpacingX * 2, row2Y, knobWidth, knobHeight);
    mixKnob.setBounds(row2StartX + knobSpacingX * 3, row2Y, knobWidth, knobHeight);
    autoReleaseButton.setBounds(buttonX, row2Y + 28, 84, 28);
}
