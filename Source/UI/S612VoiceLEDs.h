#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

namespace S612 {
namespace UI {

class VoiceLEDs : public juce::Component, public juce::Timer {
public:
    VoiceLEDs(std::array<bool, 6>& voiceFlags);
    ~VoiceLEDs() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    std::array<bool, 6>& m_voiceFlags;
    std::array<bool, 6> m_lastState;
};

} // namespace UI
} // namespace S612
