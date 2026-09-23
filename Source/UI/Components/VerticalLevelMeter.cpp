#include "VerticalLevelMeter.h"
#include <cmath>

namespace betterpresser
{

VerticalLevelMeter::VerticalLevelMeter(const juce::String& titleText)
    : title(titleText)
{
    setOpaque(false);
}

void VerticalLevelMeter::resized()
{
}

float VerticalLevelMeter::mapDbToNormalized(float db) noexcept
{
    // Range: -60 dB (0.0) to +6 dB (1.0)
    if (db <= -60.0f)
        return 0.0f;
    if (db >= 6.0f)
        return 1.0f;

    // Non-linear scaling for audio metering
    if (db >= 0.0f)
    {
        return 0.85f + (db / 6.0f) * 0.15f;
    }
    else if (db >= -18.0f)
    {
        return 0.50f + ((db + 18.0f) / 18.0f) * 0.35f;
    }
    else
    {
        return ((db + 60.0f) / 42.0f) * 0.50f;
    }
}

void VerticalLevelMeter::setLevel(float peakDb, float rmsDb)
{
    // Ballistics decay
    if (peakDb > currentPeakDb)
    {
        currentPeakDb = peakDb;
    }
    else
    {
        currentPeakDb = currentPeakDb * 0.88f + peakDb * 0.12f;
    }

    currentRmsDb = rmsDb;

    // Peak hold
    if (peakDb > peakHoldDb)
    {
        peakHoldDb = peakDb;
        peakHoldTimer = 40; // ~650ms hold at 60fps
    }
    else if (--peakHoldTimer <= 0)
    {
        peakHoldDb = peakHoldDb * 0.95f + currentPeakDb * 0.05f;
    }

    // Clip indicator
    if (peakDb >= 0.0f)
    {
        isClipping = true;
        clipHoldTimer = 60; // 1 second hold
    }
    else if (--clipHoldTimer <= 0)
    {
        isClipping = false;
    }

    repaint();
}

void VerticalLevelMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    if (bounds.getWidth() <= 10.0f || bounds.getHeight() <= 30.0f)
        return;

    // Top Title
    g.setFont(juce::Font(9.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff8b9ba8));
    g.drawText(title, juce::Rectangle<float>(0.0f, 0.0f, 16.0f, 12.0f),
               juce::Justification::centred, false);

    // Meter slot geometry
    float slotTop = 14.0f;
    float slotHeight = bounds.getHeight() - slotTop - 4.0f;
    float meterWidth = 8.0f;
    float meterX = bounds.getX() + 4.0f;

    auto slotBounds = juce::Rectangle<float>(meterX, slotTop, meterWidth, slotHeight);

    // Slot background
    g.setColour(juce::Colour(0xff0b1218));
    g.fillRoundedRectangle(slotBounds, 2.0f);
    g.setColour(juce::Colour(0xff1d2c38));
    g.drawRoundedRectangle(slotBounds, 2.0f, 1.0f);

    // Fill Active Meter Level
    float normPeak = mapDbToNormalized(currentPeakDb);
    if (normPeak > 0.001f)
    {
        float barH = normPeak * slotHeight;
        auto barBounds = juce::Rectangle<float>(meterX + 1.0f, slotBounds.getBottom() - barH,
                                                meterWidth - 2.0f, barH);

        juce::ColourGradient meterGrad(
            juce::Colour(0xff29b6f6), meterX, slotBounds.getBottom(),
            juce::Colour(0xffe53935), meterX, slotBounds.getY(), false);
        meterGrad.addColour(0.65, juce::Colour(0xff66bb6a)); // Green at -12dB
        meterGrad.addColour(0.85, juce::Colour(0xffffb74d)); // Yellow at 0dB

        g.setGradientFill(meterGrad);
        g.fillRoundedRectangle(barBounds, 1.5f);
    }

    // Peak Hold Line
    float normHold = mapDbToNormalized(peakHoldDb);
    if (normHold > 0.01f)
    {
        float holdY = slotBounds.getBottom() - normHold * slotHeight;
        g.setColour(peakHoldDb >= 0.0f ? juce::Colour(0xffff5252) : juce::Colours::white);
        g.drawLine(meterX, holdY, meterX + meterWidth, holdY, 1.5f);
    }

    // dB Scale Text on the Right
    const float scaleDbs[] = { 6.0f, 0.0f, -6.0f, -12.0f, -24.0f, -40.0f, -60.0f };
    g.setFont(juce::Font(8.0f, juce::Font::plain));

    for (float db : scaleDbs)
    {
        float norm = mapDbToNormalized(db);
        float tickY = slotBounds.getBottom() - norm * slotHeight;

        // Tick mark
        g.setColour(juce::Colour(0xff3a4f61));
        g.drawLine(meterX + meterWidth + 1.0f, tickY, meterX + meterWidth + 3.0f, tickY, 1.0f);

        // dB Text
        g.setColour(db >= 0.0f ? juce::Colour(0xffd32f2f) : juce::Colour(0xff6f8496));
        juce::String txt = (db > 0.0f ? "+" : "") + juce::String(static_cast<int>(db));
        g.drawText(txt,
                   juce::Rectangle<float>(meterX + meterWidth + 4.0f, tickY - 4.5f, 22.0f, 9.0f),
                   juce::Justification::left, false);
    }

    // Top Clip LED
    float ledSize = 5.0f;
    auto ledBounds = juce::Rectangle<float>(meterX + (meterWidth - ledSize) * 0.5f, slotTop - 8.0f, ledSize, ledSize);
    g.setColour(isClipping ? juce::Colour(0xffff1744) : juce::Colour(0xff3e1619));
    g.fillEllipse(ledBounds);
    if (isClipping)
    {
        g.setColour(juce::Colour(0xffff5252).withAlpha(0.6f));
        g.drawEllipse(ledBounds.expanded(1.0f), 1.0f);
    }
}

} // namespace betterpresser
