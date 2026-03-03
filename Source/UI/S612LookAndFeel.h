#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace S612 {
namespace UI {

class LookAndFeel : public juce::LookAndFeel_V4 {
public:
    LookAndFeel();
    ~LookAndFeel() override;

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, const float rotaryStartAngle,
                          const float rotaryEndAngle, juce::Slider&) override;

    void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle, juce::Slider&) override;

    void drawButtonBackground(juce::Graphics&, juce::Button&,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics&, juce::TextButton&,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;
};

} // namespace UI
} // namespace S612
