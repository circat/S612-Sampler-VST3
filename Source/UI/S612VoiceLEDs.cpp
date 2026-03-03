#include "S612VoiceLEDs.h"

namespace S612 {
namespace UI {

VoiceLEDs::VoiceLEDs(std::array<bool, 6>& voiceFlags) : m_voiceFlags(voiceFlags) {
    m_lastState.fill(false);
    startTimer(50);
}

VoiceLEDs::~VoiceLEDs() {
    stopTimer();
}

void VoiceLEDs::paint(juce::Graphics& g) {
    auto w = getWidth();
    auto h = getHeight();
    auto spacing = w / 6.0f;
    
    for (int i = 0; i < 6; ++i) {
        bool isOn = m_lastState[i];
        
        auto cx = spacing * i + spacing * 0.5f;
        auto cy = h * 0.5f;
        auto radius = 3.0f;
        
        if (isOn) {
            g.setColour(juce::Colour(0, 80, 0));
            g.fillEllipse(cx - radius * 3.0f, cy - radius * 3.0f, radius * 6.0f, radius * 6.0f);
            
            g.setColour(juce::Colour(50, 220, 50));
            g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
        } else {
            g.setColour(juce::Colour(10, 40, 10));
            g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
            g.setColour(juce::Colour(5, 20, 5));
            g.drawEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 0.5f);
        }
    }
}

void VoiceLEDs::timerCallback() {
    bool changed = false;
    for (int i = 0; i < 6; ++i) {
        bool current = m_voiceFlags[i];
        if (current != m_lastState[i]) {
            m_lastState[i] = current;
            changed = true;
        }
    }
    if (changed) {
        repaint();
    }
}

} // namespace UI
} // namespace S612
