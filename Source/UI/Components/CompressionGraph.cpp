#include "CompressionGraph.h"
#include <algorithm>
#include <vector>

namespace betterpresser
{

CompressionGraph::CompressionGraph(CleanCompressor& compressorRef)
    : compressor(compressorRef)
{
    setOpaque(false);
}

void CompressionGraph::resized()
{
}

void CompressionGraph::updateGraph()
{
    repaint();
}

void CompressionGraph::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    if (bounds.getWidth() <= 10.0f || bounds.getHeight() <= 10.0f)
        return;

    // 1. Recessed Bezel & Outer Frame
    g.setColour(juce::Colour(0xff090f14));
    g.fillRoundedRectangle(bounds, 6.0f);

    g.setColour(juce::Colour(0xff233340));
    g.drawRoundedRectangle(bounds, 6.0f, 1.5f);

    auto graphArea = bounds.reduced(4.0f);

    // Dark grid background
    g.setColour(juce::Colour(0xff0b131a));
    g.fillRoundedRectangle(graphArea, 4.0f);

    // 2. dB Grid Lines with subtle glow
    const float dbMarks[] = { 0.0f, -6.0f, -12.0f, -18.0f, -24.0f, -36.0f };
    g.setFont(juce::FontOptions(9.0f));

    for (float db : dbMarks)
    {
        // 0 dB at top, -40 dB at bottom
        float norm = (0.0f - db) / 40.0f;
        float y = graphArea.getY() + norm * graphArea.getHeight();

        g.setColour(juce::Colour(0xff142330));
        g.drawHorizontalLine(static_cast<int>(y), graphArea.getX(), graphArea.getRight());

        g.setColour(juce::Colour(0xff4a6275));
        g.drawText(juce::String(static_cast<int>(db)) + " dB",
                   juce::Rectangle<float>(graphArea.getX() + 6.0f, y - 5.0f, 40.0f, 10.0f),
                   juce::Justification::left, false);
    }

    // 3. Collect Raw History Samples from Ring Buffer
    const int numPoints = static_cast<int>(graphArea.getWidth());
    if (numPoints <= 4)
        return;

    int currentWriteIdx = compressor.getVisualizerReadIndex();
    const int totalBuf = CleanCompressor::visualizerBufferSize;

    const float graphBottom = graphArea.getBottom();
    const float graphTop = graphArea.getY();
    const float graphHeight = graphArea.getHeight();

    std::vector<float> rawInY(numPoints);
    std::vector<float> rawOutY(numPoints);
    std::vector<float> rawGrY(numPoints);

    for (int p = 0; p < numPoints; ++p)
    {
        int offset = numPoints - 1 - p;
        int bufIdx = (currentWriteIdx - 1 - offset + totalBuf * 10) % totalBuf;
        const auto& smp = compressor.getVisualizerSample(bufIdx);

        // Map audio level to Y coordinate (bottom up)
        rawInY[p] = graphBottom - std::clamp(smp.inputLevel, 0.0f, 1.2f) * (graphHeight * 0.85f);
        rawOutY[p] = graphBottom - std::clamp(smp.outputLevel, 0.0f, 1.2f) * (graphHeight * 0.85f);

        // Map Gain Reduction (0dB to -24dB descending from top)
        float grNorm = std::clamp(-smp.gainReductionDb / 24.0f, 0.0f, 1.0f);
        rawGrY[p] = graphTop + grNorm * (graphHeight * 0.70f);
    }

    // 4. Smooth 3-Point Weighted Moving Average (Gaussian Filter) for Eye Comfort
    std::vector<float> smoothInY(numPoints);
    std::vector<float> smoothOutY(numPoints);
    std::vector<float> smoothGrY(numPoints);

    for (int p = 0; p < numPoints; ++p)
    {
        int pPrev = std::max(0, p - 1);
        int pNext = std::min(numPoints - 1, p + 1);

        smoothInY[p] = 0.25f * rawInY[pPrev] + 0.50f * rawInY[p] + 0.25f * rawInY[pNext];
        smoothOutY[p] = 0.25f * rawOutY[pPrev] + 0.50f * rawOutY[p] + 0.25f * rawOutY[pNext];
        smoothGrY[p] = 0.25f * rawGrY[pPrev] + 0.50f * rawGrY[p] + 0.25f * rawGrY[pNext];
    }

    // Build Smooth Paths
    juce::Path inPath;
    juce::Path outPath;
    juce::Path grPath;

    for (int p = 0; p < numPoints; ++p)
    {
        float x = graphArea.getX() + static_cast<float>(p);

        if (p == 0)
        {
            inPath.startNewSubPath(x, smoothInY[p]);
            outPath.startNewSubPath(x, smoothOutY[p]);
            grPath.startNewSubPath(x, smoothGrY[p]);
        }
        else
        {
            inPath.lineTo(x, smoothInY[p]);
            outPath.lineTo(x, smoothOutY[p]);
            grPath.lineTo(x, smoothGrY[p]);
        }
    }

    // 5. Draw Gain Reduction Region (Amber / Warm Orange Vertical Gradient Fill)
    juce::Path grFill = grPath;
    grFill.lineTo(graphArea.getRight(), graphTop);
    grFill.lineTo(graphArea.getX(), graphTop);
    grFill.closeSubPath();

    juce::ColourGradient grGrad(
        juce::Colour(0xffff9800).withAlpha(0.28f), graphArea.getCentreX(), graphTop,
        juce::Colour(0xffff9800).withAlpha(0.03f), graphArea.getCentreX(), graphTop + graphHeight * 0.7f, false);
    g.setGradientFill(grGrad);
    g.fillPath(grFill);

    // Gain Reduction Line
    g.setColour(juce::Colour(0xffffa726));
    g.strokePath(grPath, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 6. Draw Output Waveform Glow & Fill (Soft Neon Cyan)
    juce::Path outFill = outPath;
    outFill.lineTo(graphArea.getRight(), graphBottom);
    outFill.lineTo(graphArea.getX(), graphBottom);
    outFill.closeSubPath();

    juce::ColourGradient outGrad(
        juce::Colour(0xff29b6f6).withAlpha(0.18f), graphArea.getCentreX(), graphTop,
        juce::Colour(0xff0288d1).withAlpha(0.02f), graphArea.getCentreX(), graphBottom, false);
    g.setGradientFill(outGrad);
    g.fillPath(outFill);

    // Output Stroke with soft glow
    g.setColour(juce::Colour(0xff4fc3f7).withAlpha(0.3f));
    g.strokePath(outPath, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour(juce::Colour(0xff81d4fa));
    g.strokePath(outPath, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 7. Draw Input Waveform (Dim Slate Blue Reference)
    g.setColour(juce::Colour(0xff456882).withAlpha(0.65f));
    g.strokePath(inPath, juce::PathStrokeType(1.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 8. Soft Edge Vignette / Fade Masks on Left & Right
    juce::ColourGradient leftFade(
        juce::Colour(0xff0b131a), graphArea.getX(), graphArea.getY(),
        juce::Colours::transparentBlack, graphArea.getX() + 24.0f, graphArea.getY(), false);
    g.setGradientFill(leftFade);
    g.fillRect(graphArea.getX(), graphArea.getY(), 24.0f, graphArea.getHeight());

    juce::ColourGradient rightFade(
        juce::Colours::transparentBlack, graphArea.getRight() - 24.0f, graphArea.getY(),
        juce::Colour(0xff0b131a), graphArea.getRight(), graphArea.getY(), false);
    g.setGradientFill(rightFade);
    g.fillRect(graphArea.getRight() - 24.0f, graphArea.getY(), 24.0f, graphArea.getHeight());

    // 9. Legend
    g.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    float legendX = graphArea.getRight() - 170.0f;
    float legendY = graphArea.getY() + 4.0f;

    // IN
    g.setColour(juce::Colour(0xff5c7d99));
    g.fillRoundedRectangle(legendX, legendY + 3.0f, 8.0f, 8.0f, 1.5f);
    g.drawText("IN", juce::Rectangle<float>(legendX + 12.0f, legendY, 20.0f, 14.0f), juce::Justification::left, false);

    // OUT
    g.setColour(juce::Colour(0xff4fc3f7));
    g.fillRoundedRectangle(legendX + 45.0f, legendY + 3.0f, 8.0f, 8.0f, 1.5f);
    g.drawText("OUT", juce::Rectangle<float>(legendX + 57.0f, legendY, 26.0f, 14.0f), juce::Justification::left, false);

    // GR
    g.setColour(juce::Colour(0xffff9800));
    g.fillRoundedRectangle(legendX + 100.0f, legendY + 3.0f, 8.0f, 8.0f, 1.5f);
    g.drawText("GR", juce::Rectangle<float>(legendX + 112.0f, legendY, 24.0f, 14.0f), juce::Justification::left, false);
}

} // namespace betterpresser
