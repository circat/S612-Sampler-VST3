#include "S612Knob.h"

namespace S612 {
namespace UI {

Knob::Knob()
    : juce::Slider(juce::Slider::RotaryVerticalDrag, juce::Slider::NoTextBox) {}

Knob::~Knob() {}

void Knob::setKnobStyle(Style style) {
  m_style = style;
  repaint();
}

void Knob::paint(juce::Graphics &g) {
  // Custom painting delegated to LookAndFeel with custom colors logic
  juce::Slider::paint(g);

  // Add label
  g.setFont(juce::Font("Arial", 9.0f, juce::Font::bold));
  g.setColour(juce::Colours::whitesmoke.withAlpha(0.9f));
  g.drawText(getName().toUpperCase(), 0, getHeight() - 15, getWidth(), 15,
             juce::Justification::centred, false);
}

void Knob::mouseDown(const juce::MouseEvent &e) {
  if (onMouseDown)
    onMouseDown(e);
  juce::Slider::mouseDown(e);
}

} // namespace UI
} // namespace S612
