#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace S612 {
namespace UI {

class InputMeter : public juce::Component, public juce::Timer {
public:
    InputMeter();
    ~InputMeter() override;

    void setLevel(float level);
    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    float m_currentLevel = 0.0f;
    float m_peakLevel = 0.0f;
    float m_decayRate = 0.95f;
};

} // namespace UI
} // namespace S612
