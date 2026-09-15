#include "CustomLookAndFeel.h"
#include <cmath>

namespace betterpresser
{

CustomLookAndFeel::CustomLookAndFeel()
{
    setDefaultSansSerifTypefaceName("Segoe UI");
    
    setColour(juce::Slider::rotarySliderFillColourId, getAccentCyan());
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff2a3c4d));
    setColour(juce::Label::textColourId, getTextPrimary());
    setColour(juce::TextButton::textColourOffId, getTextPrimary());
    setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff121d27));
    setColour(juce::ComboBox::textColourId, getTextPrimary());
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff2e4354));
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                        int x, int y, int width, int height,
                                        float sliderPosProportional,
                                        float rotaryStartAngle,
                                        float rotaryEndAngle,
                                        juce::Slider& slider)
{
    juce::ignoreUnused(slider);

    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                         static_cast<float>(width), static_cast<float>(height)).reduced(3.0f);
    auto radius = std::min(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;

    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // 1. Draw Outer Track & Tick Marks
    const float trackRadius = radius - 2.0f;
    const float trackWidth = 3.5f;

    // Background track arc
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0xff16222b));
    g.strokePath(backgroundArc, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Active value arc with cyan accent
    if (sliderPosProportional > 0.001f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f, rotaryStartAngle, angle, true);
        
        // Subtle outer glow on active arc
        g.setColour(getAccentCyan().withAlpha(0.25f));
        g.strokePath(valueArc, juce::PathStrokeType(trackWidth + 2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.setColour(getAccentCyan());
        g.strokePath(valueArc, juce::PathStrokeType(trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 2. Draw Knob Body
    const float knobRadius = radius - 7.5f;
    auto knobBounds = juce::Rectangle<float>(centreX - knobRadius, centreY - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

    // Drop shadow under knob
    g.setColour(juce::Colours::black.withAlpha(0.45f));
    g.fillEllipse(knobBounds.translated(0.0f, 2.5f));

    // Outer machined metal bezel
    juce::ColourGradient bezelGrad(juce::Colour(0xff7a8c9b), centreX, knobBounds.getY(),
                                  juce::Colour(0xff253440), centreX, knobBounds.getBottom(), false);
    g.setGradientFill(bezelGrad);
    g.fillEllipse(knobBounds);

    // Inner knob surface (radial brushed aluminum effect)
    auto innerKnobBounds = knobBounds.reduced(1.5f);
    juce::ColourGradient capGrad(juce::Colour(0xff4a5c6c), centreX, innerKnobBounds.getY(),
                                juce::Colour(0xff1e2a34), centreX, innerKnobBounds.getBottom(), false);
    g.setGradientFill(capGrad);
    g.fillEllipse(innerKnobBounds);

    // Subtle highlight rim
    g.setColour(juce::Colour(0xff8ea2b4).withAlpha(0.5f));
    g.drawEllipse(innerKnobBounds, 1.0f);

    // 3. Pointer Line / Notch
    const float pointerLength = knobRadius * 0.75f;
    const float pointerThickness = 2.5f;
    const float pStartX = centreX + (knobRadius * 0.25f) * std::sin(angle);
    const float pStartY = centreY - (knobRadius * 0.25f) * std::cos(angle);
    const float pEndX = centreX + pointerLength * std::sin(angle);
    const float pEndY = centreY - pointerLength * std::cos(angle);

    // Pointer shadow
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.drawLine(pStartX + 0.5f, pStartY + 1.0f, pEndX + 0.5f, pEndY + 1.0f, pointerThickness);

    // Pointer body (bright white/cyan)
    g.setColour(juce::Colours::white);
    g.drawLine(pStartX, pStartY, pEndX, pEndY, pointerThickness);
}

void CustomLookAndFeel::drawToggleButton(juce::Graphics& g,
                                        juce::ToggleButton& button,
                                        bool shouldDrawButtonAsHighlighted,
                                        bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    auto bounds = button.getLocalBounds().toFloat();
    bool toggled = button.getToggleState();

    // Metallic pill background
    juce::Colour bgColour = toggled ? getAccentCyan().withAlpha(0.2f) : juce::Colour(0xff121d26);
    g.setColour(bgColour);
    g.fillRoundedRectangle(bounds, 4.0f);

    // Border
    g.setColour(toggled ? getAccentCyan() : juce::Colour(0xff2d4050));
    g.drawRoundedRectangle(bounds, 4.0f, 1.2f);

    // Text & LED indicator
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.setColour(toggled ? juce::Colours::white : getTextSecondary());
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred, true);
}

void CustomLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                            juce::Button& button,
                                            const juce::Colour& backgroundColour,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(backgroundColour);
    auto bounds = button.getLocalBounds().toFloat();
    bool toggled = button.getToggleState();

    juce::Colour baseColour = toggled ? juce::Colour(0xff274157) : juce::Colour(0xff14202b);
    if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter(0.15f);
    if (shouldDrawButtonAsDown)
        baseColour = baseColour.darker(0.1f);

    juce::ColourGradient grad(baseColour.brighter(0.1f), bounds.getX(), bounds.getY(),
                              baseColour.darker(0.1f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(toggled ? getAccentCyan() : juce::Colour(0xff2d4050));
    g.drawRoundedRectangle(bounds, 4.0f, 1.2f);
}

void CustomLookAndFeel::drawButtonText(juce::Graphics& g,
                                      juce::TextButton& button,
                                      bool shouldDrawButtonAsHighlighted,
                                      bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    auto font = getTextButtonFont(button, button.getHeight());
    g.setFont(font);
    g.setColour(button.getToggleState() ? juce::Colours::white : getTextPrimary());
    g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(2),
                     juce::Justification::centred, 1);
}

void CustomLookAndFeel::drawComboBox(juce::Graphics& g,
                                    int width, int height,
                                    bool isButtonDown,
                                    int buttonX, int buttonY, int buttonW, int buttonH,
                                    juce::ComboBox& box)
{
    juce::ignoreUnused(isButtonDown, buttonX, buttonY, buttonW, buttonH, box);
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

    g.setColour(juce::Colour(0xff121d27));
    g.fillRoundedRectangle(bounds, 3.0f);

    g.setColour(juce::Colour(0xff2e4354));
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

    // Down arrow
    juce::Path arrow;
    float arrowX = width - 12.0f;
    float arrowY = height * 0.5f - 2.0f;
    arrow.startNewSubPath(arrowX - 4.0f, arrowY);
    arrow.lineTo(arrowX, arrowY + 4.0f);
    arrow.lineTo(arrowX + 4.0f, arrowY);
    g.setColour(getTextSecondary());
    g.strokePath(arrow, juce::PathStrokeType(1.5f));
}

juce::Font CustomLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return juce::Font(std::max(10.0f, buttonHeight * 0.55f), juce::Font::bold);
}

juce::Font CustomLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(11.0f, juce::Font::plain);
}

} // namespace betterpresser
