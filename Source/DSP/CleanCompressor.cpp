#include "CleanCompressor.h"
#include <algorithm>

namespace betterpresser
{

CleanCompressor::CleanCompressor()
{
    sidechainFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    reset();
}

void CleanCompressor::prepare(double sampleRate, int samplesPerBlock, int numChannels)
{
    currentSampleRate = (sampleRate > 8000.0) ? sampleRate : 44100.0;
    currentNumChannels = std::max(1, numChannels);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = currentSampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(std::max(1, samplesPerBlock));
    spec.numChannels = static_cast<juce::uint32>(currentNumChannels);

    sidechainFilter.prepare(spec);
    sidechainFilter.setType(juce::dsp::StateVariableTPTFilterType::highpass);

    smoothedInputGain.reset(currentSampleRate, 0.02); // 20ms smoothing
    smoothedOutputGain.reset(currentSampleRate, 0.02);
    smoothedMakeupGain.reset(currentSampleRate, 0.02);
    smoothedMix.reset(currentSampleRate, 0.02);

    // 30ms RMS integration window
    rmsAlpha = std::exp(-1.0f / (0.030f * static_cast<float>(currentSampleRate)));

    updateFilter();
    updateBallistics();
    reset();
}

void CleanCompressor::reset()
{
    envelopeDb = 0.0f;
    rmsMeanSquare = 0.0f;
    sidechainFilter.reset();

    smoothedInputGain.setCurrentAndTargetValue(dbToLinear(paramInputGainDb));
    smoothedOutputGain.setCurrentAndTargetValue(dbToLinear(paramOutputGainDb));
    
    float totalMakeup = paramMakeUpGainDb + (paramAutoGain ? computeAutoGainDb(paramThresholdDb, paramRatio) : 0.0f);
    smoothedMakeupGain.setCurrentAndTargetValue(dbToLinear(totalMakeup));
    smoothedMix.setCurrentAndTargetValue(paramMix);

    gainReductionDb.store(0.0f, std::memory_order_relaxed);
    inputPeakDb.store(-100.0f, std::memory_order_relaxed);
    outputPeakDb.store(-100.0f, std::memory_order_relaxed);
    inputRmsDb.store(-100.0f, std::memory_order_relaxed);
    outputRmsDb.store(-100.0f, std::memory_order_relaxed);

    for (int i = 0; i < visualizerBufferSize; ++i)
    {
        visualizerBuffer[i] = { 0.0f, 0.0f, 0.0f };
    }
    visualizerWriteIndex.store(0, std::memory_order_relaxed);
    visualizerSubsampleCounter = 0;
}

void CleanCompressor::updateFilter()
{
    float cutoff = std::clamp(paramSidechainHpfHz, 20.0f, 500.0f);
    sidechainFilter.setCutoffFrequency(cutoff);
    sidechainFilter.setResonance(0.7071f);
}

void CleanCompressor::updateBallistics()
{
    float sr = static_cast<float>(currentSampleRate);
    float attSec = std::max(0.00005f, paramAttackMs * 0.001f);
    float relSec = std::max(0.001f, paramReleaseMs * 0.001f);

    attackCoeff = std::exp(-1.0f / (attSec * sr));
    releaseCoeff = std::exp(-1.0f / (relSec * sr));
}

void CleanCompressor::setParameters(float thresholdDb,
                                     float ratio,
                                     float attackMs,
                                     float releaseMs,
                                     bool autoRelease,
                                     float kneeDb,
                                     DetectionMode detectionMode,
                                     float sidechainHpfHz,
                                     bool sidechainListen,
                                     float makeUpGainDb,
                                     bool autoGain,
                                     float mixPercent,
                                     float inputGainDb,
                                     float outputGainDb)
{
    paramThresholdDb = thresholdDb;
    paramRatio = std::max(1.0f, ratio);
    paramAttackMs = attackMs;
    paramReleaseMs = releaseMs;
    paramAutoRelease = autoRelease;
    paramKneeDb = std::max(0.0f, kneeDb);
    paramDetectionMode = detectionMode;
    paramSidechainHpfHz = sidechainHpfHz;
    paramSidechainListen = sidechainListen;
    paramMakeUpGainDb = makeUpGainDb;
    paramAutoGain = autoGain;
    paramMix = std::clamp(mixPercent * 0.01f, 0.0f, 1.0f);
    paramInputGainDb = inputGainDb;
    paramOutputGainDb = outputGainDb;

    updateFilter();
    updateBallistics();

    smoothedInputGain.setTargetValue(dbToLinear(paramInputGainDb));
    smoothedOutputGain.setTargetValue(dbToLinear(paramOutputGainDb));

    float totalMakeup = paramMakeUpGainDb + (paramAutoGain ? computeAutoGainDb(paramThresholdDb, paramRatio) : 0.0f);
    smoothedMakeupGain.setTargetValue(dbToLinear(totalMakeup));
    smoothedMix.setTargetValue(paramMix);
}

float CleanCompressor::computeStaticGainReductionDb(float inputDb, float thresholdDb, float ratio, float kneeDb)
{
    float r = std::max(1.0f, ratio);
    float w = std::max(0.0f, kneeDb);

    if (w > 0.001f)
    {
        float lowerBound = thresholdDb - w * 0.5f;
        float upperBound = thresholdDb + w * 0.5f;

        if (inputDb <= lowerBound)
        {
            return 0.0f;
        }
        else if (inputDb > upperBound)
        {
            float targetOutputDb = thresholdDb + (inputDb - thresholdDb) / r;
            return targetOutputDb - inputDb;
        }
        else
        {
            float delta = inputDb - thresholdDb + w * 0.5f;
            float targetOutputDb = inputDb + ((1.0f / r - 1.0f) * (delta * delta)) / (2.0f * w);
            return targetOutputDb - inputDb;
        }
    }
    else
    {
        if (inputDb <= thresholdDb)
        {
            return 0.0f;
        }
        else
        {
            float targetOutputDb = thresholdDb + (inputDb - thresholdDb) / r;
            return targetOutputDb - inputDb;
        }
    }
}

float CleanCompressor::computeAutoGainDb(float thresholdDb, float ratio)
{
    float r = std::max(1.0f, ratio);
    if (thresholdDb >= 0.0f)
        return 0.0f;

    // Standard auto-makeup compensation
    float grEst = -thresholdDb * (1.0f - 1.0f / r) * 0.55f;
    return std::clamp(grEst, 0.0f, 36.0f);
}

void CleanCompressor::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numSamples == 0 || numChannels == 0)
        return;

    float blockInputPeak = 0.0f;
    float blockOutputPeak = 0.0f;
    double blockInputSumSq = 0.0;
    double blockOutputSumSq = 0.0;
    float minGrDbThisBlock = 0.0f;

    // Slower, smoother subsampling for visualizer (~6ms per point = ~166 points/sec => ~3.3s display window)
    const int visualizerSubsampleInterval = std::max(64, static_cast<int>(currentSampleRate * 0.006));

    for (int i = 0; i < numSamples; ++i)
    {
        float inGain = smoothedInputGain.getNextValue();
        float outGain = smoothedOutputGain.getNextValue();
        float makeupGain = smoothedMakeupGain.getNextValue();
        float mixVal = smoothedMix.getNextValue();

        // 1. Read input and apply input gain
        float dryL = buffer.getSample(0, i) * inGain;
        float dryR = (numChannels > 1) ? (buffer.getSample(1, i) * inGain) : dryL;

        float inAbsMax = std::max(std::abs(dryL), std::abs(dryR));
        blockInputPeak = std::max(blockInputPeak, inAbsMax);
        blockInputSumSq += (dryL * dryL + dryR * dryR) * 0.5;

        // 2. Sidechain filtering
        float scL = sidechainFilter.processSample(0, dryL);
        float scR = (numChannels > 1) ? sidechainFilter.processSample(1, dryR) : scL;

        if (paramSidechainListen)
        {
            buffer.setSample(0, i, scL * outGain);
            if (numChannels > 1)
                buffer.setSample(1, i, scR * outGain);
            continue;
        }

        // 3. Detection (Peak or RMS)
        float detectorLin = 0.0f;
        if (paramDetectionMode == DetectionMode::Peak)
        {
            detectorLin = std::max(std::abs(scL), std::abs(scR));
        }
        else
        {
            float energy = 0.5f * (scL * scL + scR * scR);
            rmsMeanSquare = rmsAlpha * rmsMeanSquare + (1.0f - rmsAlpha) * energy;
            detectorLin = std::sqrt(std::max(1.0e-10f, rmsMeanSquare));
        }

        float detectorDb = linearToDb(detectorLin);

        // 4. Gain Computer (Static Gain Reduction in dB)
        float targetGrDb = computeStaticGainReductionDb(detectorDb, paramThresholdDb, paramRatio, paramKneeDb);

        // 5. Envelope Follower Ballistics
        if (targetGrDb < envelopeDb)
        {
            // Attack (gain reduction becoming more severe)
            envelopeDb = attackCoeff * envelopeDb + (1.0f - attackCoeff) * targetGrDb;
        }
        else
        {
            // Release (recovery towards 0 dB)
            float effectiveRelCoeff = releaseCoeff;
            if (paramAutoRelease)
            {
                // Dynamic program-dependent release:
                // Fast recovery for brief high transients, slower smoother recovery for sustained low-level compression
                float crestFactorDb = std::max(0.0f, detectorDb - linearToDb(std::sqrt(std::max(1.0e-10f, rmsMeanSquare))));
                float speedUp = std::clamp(crestFactorDb * 0.1f, 1.0f, 4.0f);
                effectiveRelCoeff = std::pow(releaseCoeff, speedUp);
            }
            envelopeDb = effectiveRelCoeff * envelopeDb + (1.0f - effectiveRelCoeff) * targetGrDb;
        }

        if (envelopeDb > 0.0f)
            envelopeDb = 0.0f;

        minGrDbThisBlock = std::min(minGrDbThisBlock, envelopeDb);

        // 6. Gain Application
        float compGainLin = dbToLinear(envelopeDb);
        float totalWetGain = compGainLin * makeupGain;

        float wetL = dryL * totalWetGain;
        float wetR = dryR * totalWetGain;

        // 7. Parallel Mix & Output Gain
        float outL = ((1.0f - mixVal) * dryL + mixVal * wetL) * outGain;
        float outR = ((1.0f - mixVal) * dryR + mixVal * wetR) * outGain;

        buffer.setSample(0, i, outL);
        if (numChannels > 1)
            buffer.setSample(1, i, outR);

        float outAbsMax = std::max(std::abs(outL), std::abs(outR));
        blockOutputPeak = std::max(blockOutputPeak, outAbsMax);
        blockOutputSumSq += (outL * outL + outR * outR) * 0.5;

        // Smooth tracking for visualizer buffer
        visualizerInPeak = std::max(visualizerInPeak, inAbsMax);
        visualizerOutPeak = std::max(visualizerOutPeak, outAbsMax);
        visualizerGrMin = std::min(visualizerGrMin, envelopeDb);

        // 8. Visualizer Push
        if (++visualizerSubsampleCounter >= visualizerSubsampleInterval)
        {
            visualizerSubsampleCounter = 0;
            int writeIdx = visualizerWriteIndex.load(std::memory_order_relaxed);
            visualizerBuffer[writeIdx % visualizerBufferSize] = {
                std::clamp(visualizerInPeak, 0.0f, 2.0f),
                std::clamp(visualizerOutPeak, 0.0f, 2.0f),
                visualizerGrMin
            };
            visualizerWriteIndex.store((writeIdx + 1) % visualizerBufferSize, std::memory_order_relaxed);
            visualizerInPeak = 0.0f;
            visualizerOutPeak = 0.0f;
            visualizerGrMin = 0.0f;
        }
    }

    if (!paramSidechainListen)
    {
        // Store atomic metering readouts (current envelope at end of block)
        gainReductionDb.store(envelopeDb, std::memory_order_relaxed);
        inputPeakDb.store(linearToDb(blockInputPeak), std::memory_order_relaxed);
        outputPeakDb.store(linearToDb(blockOutputPeak), std::memory_order_relaxed);

        float inRmsLin = std::sqrt(static_cast<float>(blockInputSumSq / std::max(1, numSamples)));
        float outRmsLin = std::sqrt(static_cast<float>(blockOutputSumSq / std::max(1, numSamples)));
        inputRmsDb.store(linearToDb(inRmsLin), std::memory_order_relaxed);
        outputRmsDb.store(linearToDb(outRmsLin), std::memory_order_relaxed);
    }
}

} // namespace betterpresser
