#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace betterpresser
{

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();
    ~CustomLookAndFeel() override = default;

    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider& slider) override;

    void drawToggleButton(juce::Graphics& g,
                          juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics& g,
                        juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics& g,
                      int width, int height,
                      bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    juce::Font getLabelFont(juce::Label&) override;

    // Palette Colors
    static juce::Colour getChassisBackground() noexcept { return juce::Colour(0xff182430); }
    static juce::Colour getChassisGradientTop() noexcept { return juce::Colour(0xff223545); }
    static juce::Colour getChassisGradientBottom() noexcept { return juce::Colour(0xff121b24); }
    static juce::Colour getPanelDark() noexcept { return juce::Colour(0xff0d151c); }
    static juce::Colour getAccentCyan() noexcept { return juce::Colour(0xff4fc3f7); }
    static juce::Colour getAccentOrange() noexcept { return juce::Colour(0xffff9800); }
    static juce::Colour getMeterBeige() noexcept { return juce::Colour(0xfff3eedc); }
    static juce::Colour getTextPrimary() noexcept { return juce::Colour(0xffe0e6ed); }
    static juce::Colour getTextSecondary() noexcept { return juce::Colour(0xff8b9ba8); }
};

} // namespace betterpresser
