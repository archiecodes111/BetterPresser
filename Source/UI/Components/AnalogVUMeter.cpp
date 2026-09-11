#include "AnalogVUMeter.h"
#include <cmath>
#include <algorithm>

namespace betterpresser
{

AnalogVUMeter::AnalogVUMeter()
{
    setOpaque(false);
    currentNeedlePos = 1.0f; // Resting at 0 dB (no gain reduction)
    targetNeedlePos = 1.0f;
}

void AnalogVUMeter::resized()
{
}

float AnalogVUMeter::mapDbToNormalizedPosition(float db) noexcept
{
    // db is <= 0 (e.g. 0dB = 1.0 on right, -30dB = 0.0 on left)
    // Non-linear VU compression scale spacing
    if (db >= 0.0f)
        return 1.0f;
    if (db <= -30.0f)
        return 0.0f;

    if (db >= -5.0f)
    {
        // 0 to -5 dB covers [0.75, 1.0]
        return 0.75f + (db + 5.0f) / 5.0f * 0.25f;
    }
    else if (db >= -10.0f)
    {
        // -5 to -10 dB covers [0.52, 0.75]
        return 0.52f + (db + 10.0f) / 5.0f * 0.23f;
    }
    else if (db >= -20.0f)
    {
        // -10 to -20 dB covers [0.22, 0.52]
        return 0.22f + (db + 20.0f) / 10.0f * 0.30f;
    }
    else
    {
        // -20 to -30 dB covers [0.0, 0.22]
        return (db + 30.0f) / 10.0f * 0.22f;
    }
}

float AnalogVUMeter::mapNormalizedPositionToAngle(float norm) noexcept
{
    // Arc spans from -42 degrees (-0.733 rad) to +42 degrees (+0.733 rad)
    const float minAngle = -0.733f;
    const float maxAngle = 0.733f;
    float clamped = std::clamp(norm, -0.05f, 1.08f);
    return minAngle + clamped * (maxAngle - minAngle);
}

void AnalogVUMeter::updateGainReduction(float grDb, float dtSeconds)
{
    targetNeedlePos = mapDbToNormalizedPosition(grDb);

    // 2nd order mass-spring-damper physics simulation
    float springForce = naturalFrequency * naturalFrequency * (targetNeedlePos - currentNeedlePos);
    float dampingForce = 2.0f * dampingFactor * naturalFrequency * needleVelocity;
    float acceleration = springForce - dampingForce;

    needleVelocity += acceleration * dtSeconds;
    currentNeedlePos += needleVelocity * dtSeconds;

    // Mechanical end stops
    if (currentNeedlePos < -0.05f)
    {
        currentNeedlePos = -0.05f;
        needleVelocity = 0.0f;
    }
    else if (currentNeedlePos > 1.08f)
    {
        currentNeedlePos = 1.08f;
        needleVelocity = 0.0f;
    }

    repaint();
}

void AnalogVUMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    if (bounds.getWidth() <= 10.0f || bounds.getHeight() <= 10.0f)
        return;

    // 1. Recessed Bezel Frame & Shadow
    g.setColour(juce::Colour(0xff090f14));
    g.fillRoundedRectangle(bounds, 6.0f);

    g.setColour(juce::Colour(0xff233340));
    g.drawRoundedRectangle(bounds, 6.0f, 1.5f);

    auto meterFaceBounds = bounds.reduced(3.5f);

    // 2. Vintage Illuminated Dial Background
    juce::ColourGradient dialGrad(juce::Colour(0xfff5f0dc), meterFaceBounds.getCentreX(), meterFaceBounds.getY(),
                                 juce::Colour(0xffdcd3b8), meterFaceBounds.getCentreX(), meterFaceBounds.getBottom(), false);
    g.setGradientFill(dialGrad);
    g.fillRoundedRectangle(meterFaceBounds, 4.0f);

    // Subtle inner shadow at top of dial
    juce::ColourGradient topShadow(juce::Colours::black.withAlpha(0.25f), meterFaceBounds.getCentreX(), meterFaceBounds.getY(),
                                   juce::Colours::transparentBlack, meterFaceBounds.getCentreX(), meterFaceBounds.getY() + 18.0f, false);
    g.setGradientFill(topShadow);
    g.fillRoundedRectangle(meterFaceBounds, 4.0f);

    // 3. Dial Geometry & Arc Markings
    const float pivotX = meterFaceBounds.getCentreX();
    const float pivotY = meterFaceBounds.getBottom() + meterFaceBounds.getHeight() * 0.45f;
    const float needleRadius = meterFaceBounds.getHeight() * 1.15f;
    const float scaleRadius = needleRadius * 0.88f;

    // Scale Arc
    const float angleMin = mapNormalizedPositionToAngle(0.0f);
    const float angleMax = mapNormalizedPositionToAngle(1.0f);

    juce::Path scaleArc;
    scaleArc.addCentredArc(pivotX, pivotY, scaleRadius, scaleRadius, 0.0f, angleMin, angleMax, true);
    g.setColour(juce::Colour(0xff2b2b2b));
    g.strokePath(scaleArc, juce::PathStrokeType(1.8f));

    // Red Overload Region above 0 dB
    const float angleOverload = mapNormalizedPositionToAngle(1.06f);
    juce::Path overloadArc;
    overloadArc.addCentredArc(pivotX, pivotY, scaleRadius, scaleRadius, 0.0f, angleMax, angleOverload, true);
    g.setColour(juce::Colour(0xffd32f2f));
    g.strokePath(overloadArc, juce::PathStrokeType(2.5f));

    // Tick Marks & Numbers
    struct TickDef { float db; const char* label; bool major; };
    const TickDef ticks[] = {
        { -30.0f, "30", true },
        { -20.0f, "20", true },
        { -15.0f, "", false },
        { -10.0f, "10", true },
        { -7.0f, "7", false },
        { -5.0f, "5", true },
        { -3.0f, "3", false },
        { -2.0f, "2", false },
        { -1.0f, "1", false },
        {  0.0f, "0", true }
    };

    g.setFont(juce::Font(10.5f, juce::Font::bold));

    for (const auto& tick : ticks)
    {
        float norm = mapDbToNormalizedPosition(tick.db);
        float a = mapNormalizedPositionToAngle(norm);
        float sinA = std::sin(a);
        float cosA = std::cos(a);

        float rOuter = scaleRadius + (tick.major ? 4.0f : 2.5f);
        float rInner = scaleRadius - (tick.major ? 5.5f : 3.5f);

        float x1 = pivotX + rInner * sinA;
        float y1 = pivotY - rInner * cosA;
        float x2 = pivotX + rOuter * sinA;
        float y2 = pivotY - rOuter * cosA;

        g.setColour(juce::Colour(0xff222222));
        g.drawLine(x1, y1, x2, y2, tick.major ? 1.6f : 1.0f);

        if (tick.major && tick.label[0] != '\0')
        {
            float rText = scaleRadius - 13.0f;
            float tx = pivotX + rText * sinA;
            float ty = pivotY - rText * cosA;

            g.drawText(tick.label,
                       juce::Rectangle<float>(tx - 12.0f, ty - 6.0f, 24.0f, 12.0f),
                       juce::Justification::centred, false);
        }
    }

    // Title / VU Legend
    g.setColour(juce::Colour(0xff333333));
    g.setFont(juce::Font(9.5f, juce::Font::bold));
    g.drawText("GAIN REDUCTION  dB",
               juce::Rectangle<float>(meterFaceBounds.getX(), meterFaceBounds.getY() + 12.0f,
                                      meterFaceBounds.getWidth(), 14.0f),
               juce::Justification::centred, false);

    // 4. Physical Needle
    const float needleAngle = mapNormalizedPositionToAngle(currentNeedlePos);
    const float nSin = std::sin(needleAngle);
    const float nCos = std::cos(needleAngle);

    const float nTipX = pivotX + (needleRadius * 0.96f) * nSin;
    const float nTipY = pivotY - (needleRadius * 0.96f) * nCos;
    const float nTailX = pivotX - 12.0f * nSin;
    const float nTailY = pivotY + 12.0f * nCos;

    // Needle shadow
    g.setColour(juce::Colours::black.withAlpha(0.25f));
    g.drawLine(nTailX + 2.0f, nTailY + 2.0f, nTipX + 2.0f, nTipY + 2.0f, 1.8f);

    // Needle pointer
    g.setColour(juce::Colour(0xff1a1a1a));
    g.drawLine(nTailX, nTailY, nTipX, nTipY, 1.8f);

    // Needle tip accent (bright red or amber)
    const float tipLen = 14.0f;
    g.setColour(juce::Colour(0xffd32f2f));
    g.drawLine(nTipX - tipLen * nSin, nTipY + tipLen * nCos, nTipX, nTipY, 1.8f);

    // 5. Center Pivot Cap
    const float capRadius = 14.0f;
    auto capBounds = juce::Rectangle<float>(pivotX - capRadius, pivotY - capRadius, capRadius * 2.0f, capRadius * 2.0f);

    // Clip to face bounds so pivot stays inside bottom
    juce::Graphics::ScopedSaveState state(g);
    g.reduceClipRegion(meterFaceBounds.toNearestInt());

    juce::ColourGradient capGrad(juce::Colour(0xff404040), pivotX, capBounds.getY(),
                                juce::Colour(0xff111111), pivotX, capBounds.getBottom(), false);
    g.setGradientFill(capGrad);
    g.fillEllipse(capBounds);

    g.setColour(juce::Colour(0xff666666));
    g.drawEllipse(capBounds, 1.2f);

    // Center screw highlight
    g.setColour(juce::Colour(0xff999999));
    g.fillEllipse(juce::Rectangle<float>(pivotX - 3.0f, pivotY - 3.0f, 6.0f, 6.0f));
}

} // namespace betterpresser
