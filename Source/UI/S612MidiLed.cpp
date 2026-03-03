#include "S612MidiLed.h"

namespace S612 {
namespace UI {

MidiLed::MidiLed() {
    startTimer(50);
}

MidiLed::~MidiLed() {
    stopTimer();
}

void MidiLed::midiReceived() {
    m_isOn = true;
    m_counter = 10;
    repaint();
}

void MidiLed::timerCallback() {
    if (m_counter > 0) {
        m_counter--;
        if (m_counter == 0) {
            m_isOn = false;
            repaint();
        }
    }
}

void MidiLed::paint(juce::Graphics& g) {
    auto w = getWidth();
    auto h = getHeight();
    auto radius = juce::jmin(w, h) * 0.4f;
    auto cx = w * 0.5f;
    auto cy = h * 0.5f;
    
    if (m_isOn) {
        // Glow
        g.setColour(juce::Colour(0, 200, 0));
        g.fillEllipse(cx - radius * 2, cy - radius * 2, radius * 4, radius * 4);
        
        // LED on
        g.setColour(juce::Colour(50, 255, 50));
        g.fillEllipse(cx - radius, cy - radius, radius * 2, radius * 2);
    } else {
        // LED off
        g.setColour(juce::Colour(10, 40, 10));
        g.fillEllipse(cx - radius, cy - radius, radius * 2, radius * 2);
        
        g.setColour(juce::Colour(5, 20, 5));
        g.drawEllipse(cx - radius, cy - radius, radius * 2, radius * 2, 1.0f);
    }
}

} // namespace UI
} // namespace S612
