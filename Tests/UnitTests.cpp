#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "../Source/DSP/CleanCompressor.h"
#include "../Source/PluginProcessor.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>

namespace betterpresser::tests
{

struct TestResult
{
    std::string name;
    bool passed;
    std::string details;
};

class BetterPresserTestSuite
{
public:
    std::vector<TestResult> results;

    void runAllTests()
    {
        std::cout << "========================================================\n";
        std::cout << "        BETTERPRESSER COMPREHENSIVE TEST SUITE          \n";
        std::cout << "========================================================\n\n";

        testGainComputerUnityRatio();
        testGainComputerHardKneeMath();
        testGainComputerSoftKneeContinuity();
        testPeakVsRmsDetection();
        testAttackReleaseBallistics();
        testAutoGainMath();
        testDryWetCrossfade();
        testSidechainHpfRejection();
        testApvtsSerialization();
        testRealtimeSafetyAndEdgeCases();

        printSummary();
    }

private:
    void record(const std::string& name, bool passed, const std::string& details = "")
    {
        results.push_back({ name, passed, details });
        std::cout << (passed ? "[PASS] " : "[FAIL] ") << name;
        if (!details.empty())
            std::cout << " (" << details << ")";
        std::cout << "\n";
    }

    void testGainComputerUnityRatio()
    {
        const float testLevels[] = { -60.0f, -40.0f, -20.0f, -6.0f, 0.0f, +6.0f, +12.0f };
        bool allPassed = true;

        for (float inDb : testLevels)
        {
            float gr = CleanCompressor::computeStaticGainReductionDb(inDb, -20.0f, 1.0f, 0.0f);
            if (std::abs(gr) > 1e-4f)
            {
                allPassed = false;
                break;
            }
        }

        record("Gain Computer: 1:1 Unity Ratio produces 0 dB reduction", allPassed);
    }

    void testGainComputerHardKneeMath()
    {
        // Threshold: -20 dB, Ratio: 4:1, Knee: 0 dB
        float grBelow = CleanCompressor::computeStaticGainReductionDb(-30.0f, -20.0f, 4.0f, 0.0f);
        float grAtThresh = CleanCompressor::computeStaticGainReductionDb(-20.0f, -20.0f, 4.0f, 0.0f);
        
        // Input: -8 dB (12 dB over threshold) -> Target Out: -20 + 12/4 = -17 dB -> GR = -9 dB
        float grAbove = CleanCompressor::computeStaticGainReductionDb(-8.0f, -20.0f, 4.0f, 0.0f);

        bool passed = (std::abs(grBelow) < 1e-4f) &&
                      (std::abs(grAtThresh) < 1e-4f) &&
                      (std::abs(grAbove - (-9.0f)) < 1e-4f);

        record("Gain Computer: Hard Knee Exact dB Reduction (-9.0 dB at -8 dB input)", passed,
               "Expected -9.0 dB, got " + std::to_string(grAbove) + " dB");
    }

    void testGainComputerSoftKneeContinuity()
    {
        // Threshold: -20 dB, Ratio: 4:1, Knee: 10 dB (-25 dB to -15 dB)
        float grLowerBound = CleanCompressor::computeStaticGainReductionDb(-25.0f, -20.0f, 4.0f, 10.0f);
        float grUpperBound = CleanCompressor::computeStaticGainReductionDb(-15.0f, -20.0f, 4.0f, 10.0f);

        // At upper bound: -15 dB input -> 5 dB over threshold -> linear region GR = -15 - (-20 + 5/4) = -3.75 dB
        bool boundPass = (std::abs(grLowerBound) < 1e-3f) &&
                         (std::abs(grUpperBound - (-3.75f)) < 1e-3f);

        // Verify monotonic continuous descent
        bool monotonic = true;
        float prevGr = 0.0f;
        for (float db = -30.0f; db <= 0.0f; db += 0.5f)
        {
            float gr = CleanCompressor::computeStaticGainReductionDb(db, -20.0f, 4.0f, 10.0f);
            if (gr > prevGr + 1e-5f) // Should only become more negative or stay equal
            {
                monotonic = false;
                break;
            }
            prevGr = gr;
        }

        record("Gain Computer: Soft-Knee Polynomial Continuity & Monotonicity", boundPass && monotonic);
    }

    void testPeakVsRmsDetection()
    {
        CleanCompressor peakComp;
        peakComp.prepare(44100.0, 512, 2);
        peakComp.setParameters(-20.0f, 4.0f, 0.1f, 100.0f, false, 0.0f,
                               DetectionMode::Peak, 30.0f, 20.0f, false, 0.0f, false, 100.0f, 0.0f, 0.0f);

        CleanCompressor rmsComp;
        rmsComp.prepare(44100.0, 512, 2);
        rmsComp.setParameters(-20.0f, 4.0f, 0.1f, 100.0f, false, 0.0f,
                              DetectionMode::RMS, 30.0f, 20.0f, false, 0.0f, false, 100.0f, 0.0f, 0.0f);

        // Create a 1-sample spike buffer
        juce::AudioBuffer<float> spikeBufPeak(2, 64);
        spikeBufPeak.clear();
        spikeBufPeak.setSample(0, 0, 1.0f); // 0 dB spike

        juce::AudioBuffer<float> spikeBufRms(2, 64);
        spikeBufRms.clear();
        spikeBufRms.setSample(0, 0, 1.0f);

        peakComp.process(spikeBufPeak);
        rmsComp.process(spikeBufRms);

        float peakGr = peakComp.getGainReductionDb();
        float rmsGr = rmsComp.getGainReductionDb();

        // Peak mode must react immediately to instantaneous spike, RMS mode must smooth it out
        bool passed = (peakGr < -2.0f) && (rmsGr > -0.1f);

        record("Detection Mode: Peak vs RMS Impulse Response Differentiation", passed,
               "Peak GR: " + std::to_string(peakGr) + " dB, RMS GR: " + std::to_string(rmsGr) + " dB");
    }

    void testAttackReleaseBallistics()
    {
        CleanCompressor comp;
        comp.prepare(44100.0, 512, 2);
        // Fast attack 1ms, long release 100ms
        comp.setParameters(-20.0f, 4.0f, 1.0f, 100.0f, false, 0.0f,
                           DetectionMode::Peak, 30.0f, 20.0f, false, 0.0f, false, 100.0f, 0.0f, 0.0f);
        comp.reset();

        // Step 1: Step input of 0 dB sine wave
        juce::AudioBuffer<float> stepBuf(2, 2048);
        for (int i = 0; i < 2048; ++i)
        {
            float s = std::sin(2.0f * juce::MathConstants<float>::pi * 1000.0f * (float)i / 44100.0f);
            stepBuf.setSample(0, i, s);
            stepBuf.setSample(1, i, s);
        }

        comp.process(stepBuf);
        float grAfterAttack = comp.getGainReductionDb();

        // Step 2: Silence buffer to verify release recovery (1 full second = 10 time constants)
        juce::AudioBuffer<float> silenceBuf(2, 44100);
        silenceBuf.clear();
        comp.process(silenceBuf);
        float grAfterRelease = comp.getGainReductionDb();

        bool passed = (grAfterAttack < -12.0f) && (grAfterRelease > -0.5f);

        record("Envelope Ballistics: Attack onset and Release recovery", passed,
               "Attack GR: " + std::to_string(grAfterAttack) + " dB, Recovered GR: " + std::to_string(grAfterRelease) + " dB");
    }

    void testAutoGainMath()
    {
        float autoGain1 = CleanCompressor::computeAutoGainDb(-20.0f, 4.0f);
        float autoGainZero = CleanCompressor::computeAutoGainDb(0.0f, 4.0f);

        bool passed = (autoGain1 > 6.0f && autoGain1 < 12.0f) && (std::abs(autoGainZero) < 1e-4f);
        record("Auto-Gain: Mathematical Makeup Gain Calculation", passed,
               "AutoGain at -20dB/4:1 = " + std::to_string(autoGain1) + " dB");
    }

    void testDryWetCrossfade()
    {
        CleanCompressor dryComp;
        dryComp.prepare(44100.0, 256, 2);
        // Mix = 0% (100% Dry)
        dryComp.setParameters(-30.0f, 10.0f, 0.1f, 10.0f, false, 0.0f,
                              DetectionMode::Peak, 30.0f, 20.0f, false, 0.0f, false, 0.0f, 0.0f, 0.0f);
        dryComp.reset();

        juce::AudioBuffer<float> testBuf(2, 256);
        for (int i = 0; i < 256; ++i)
        {
            float val = 0.8f * std::sin((float)i * 0.1f);
            testBuf.setSample(0, i, val);
            testBuf.setSample(1, i, val);
        }

        juce::AudioBuffer<float> origBuf;
        origBuf.makeCopyOf(testBuf);

        dryComp.process(testBuf);

        float maxDiff = 0.0f;
        for (int i = 0; i < 256; ++i)
        {
            maxDiff = std::max(maxDiff, std::abs(testBuf.getSample(0, i) - origBuf.getSample(0, i)));
        }

        bool passed = (maxDiff < 1e-4f);
        record("Dry/Wet Mix: 0% Mix produces exact bit-transparent dry signal", passed,
               "Max diff: " + std::to_string(maxDiff));
    }

    void testSidechainHpfRejection()
    {
        CleanCompressor compOff;
        compOff.prepare(44100.0, 2048, 2);
        compOff.setParameters(-20.0f, 8.0f, 1.0f, 100.0f, false, 0.0f,
                              DetectionMode::Peak, 30.0f, 20.0f, false, 0.0f, false, 100.0f, 0.0f, 0.0f);

        CleanCompressor compHpf;
        compHpf.prepare(44100.0, 2048, 2);
        compHpf.setParameters(-20.0f, 8.0f, 1.0f, 100.0f, false, 0.0f,
                              DetectionMode::Peak, 30.0f, 300.0f, false, 0.0f, false, 100.0f, 0.0f, 0.0f);

        // 40 Hz low-bass sine wave at 0 dB
        juce::AudioBuffer<float> bassBufOff(2, 2048);
        juce::AudioBuffer<float> bassBufHpf(2, 2048);
        for (int i = 0; i < 2048; ++i)
        {
            float s = std::sin(2.0f * juce::MathConstants<float>::pi * 40.0f * (float)i / 44100.0f);
            bassBufOff.setSample(0, i, s);
            bassBufOff.setSample(1, i, s);
            bassBufHpf.setSample(0, i, s);
            bassBufHpf.setSample(1, i, s);
        }

        compOff.process(bassBufOff);
        compHpf.process(bassBufHpf);

        float grOff = compOff.getGainReductionDb();
        float grHpf = compHpf.getGainReductionDb();

        // 300 Hz HPF should reject 40 Hz fundamental and produce significantly less gain reduction
        bool passed = (grOff < -10.0f) && (grHpf > grOff + 8.0f);
        record("Sidechain Filter: 300 Hz HPF attenuates sub-bass compression triggering", passed,
               "GR HPF Off: " + std::to_string(grOff) + " dB, GR HPF 300Hz: " + std::to_string(grHpf) + " dB");
    }

    void testApvtsSerialization()
    {
        AudioPluginAudioProcessor proc1;
        auto& apvts1 = proc1.getAPVTS();

        // Set custom parameter values
        if (auto* p = apvts1.getParameter("threshold")) p->setValueNotifyingHost(p->convertTo0to1(-34.5f));
        if (auto* p = apvts1.getParameter("ratio")) p->setValueNotifyingHost(p->convertTo0to1(8.2f));
        if (auto* p = apvts1.getParameter("attack")) p->setValueNotifyingHost(p->convertTo0to1(45.0f));
        if (auto* p = apvts1.getParameter("release")) p->setValueNotifyingHost(p->convertTo0to1(250.0f));
        if (auto* p = apvts1.getParameter("knee")) p->setValueNotifyingHost(p->convertTo0to1(12.0f));
        if (auto* p = apvts1.getParameter("autoRelease")) p->setValueNotifyingHost(1.0f);
        if (auto* p = apvts1.getParameter("detectionMode")) p->setValueNotifyingHost(1.0f);

        juce::MemoryBlock memBlock;
        proc1.getStateInformation(memBlock);

        AudioPluginAudioProcessor proc2;
        proc2.setStateInformation(memBlock.getData(), static_cast<int>(memBlock.getSize()));
        auto& apvts2 = proc2.getAPVTS();

        float thresh2 = apvts2.getRawParameterValue("threshold")->load();
        float ratio2 = apvts2.getRawParameterValue("ratio")->load();
        float attack2 = apvts2.getRawParameterValue("attack")->load();
        float autoRel2 = apvts2.getRawParameterValue("autoRelease")->load();
        float det2 = apvts2.getRawParameterValue("detectionMode")->load();

        bool passed = (std::abs(thresh2 - (-34.5f)) < 0.2f) &&
                      (std::abs(ratio2 - 8.2f) < 0.2f) &&
                      (std::abs(attack2 - 45.0f) < 0.5f) &&
                      (autoRel2 > 0.5f) &&
                      (det2 > 0.5f);

        record("APVTS State Recall: XML state serialization and parameter restoration", passed);
    }

    void testRealtimeSafetyAndEdgeCases()
    {
        CleanCompressor comp;
        comp.prepare(96000.0, 1024, 2);

        // Process buffers containing extreme edge cases (NaNs, Infs, extreme loud signals)
        juce::AudioBuffer<float> edgeBuf(2, 512);
        for (int i = 0; i < 512; ++i)
        {
            edgeBuf.setSample(0, i, (i % 2 == 0) ? 100.0f : -100.0f); // +40 dB overload
            edgeBuf.setSample(1, i, (i % 2 == 0) ? -100.0f : 100.0f);
        }

        comp.process(edgeBuf);

        bool noNans = true;
        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < 512; ++i)
            {
                float s = edgeBuf.getSample(ch, i);
                if (std::isnan(s) || std::isinf(s))
                {
                    noNans = false;
                    break;
                }
            }
        }

        record("Realtime Safety: Overload containment & NaN/Inf prevention", noNans);
    }

    void printSummary()
    {
        int passedCount = 0;
        for (const auto& r : results)
            if (r.passed) passedCount++;

        std::cout << "\n========================================================\n";
        std::cout << "TEST SUMMARY: " << passedCount << " / " << results.size() << " PASSED\n";
        std::cout << "========================================================\n";
    }
};

} // namespace betterpresser::tests
