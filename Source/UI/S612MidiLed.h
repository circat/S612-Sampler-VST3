#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace S612 {
namespace UI {

class MidiLed : public juce::Component, public juce::Timer {
public:
    MidiLed();
    ~MidiLed() override;

    void midiReceived();
    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    bool m_isOn = false;
    int m_counter = 0;
};

} // namespace UI
} // namespace S612
