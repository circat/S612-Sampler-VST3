#include "S612Fader.h"

namespace S612 {
namespace UI {

Fader::Fader()
    : juce::Slider(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox) {
  // Set range explicitly just to be safe, though Attachment will override it
  setRange(0.0, 1.0);
}

Fader::~Fader() {}

void Fader::paint(juce::Graphics &g) {
  // Ensure we are using the horizontal style for our LookAndFeel drawing
  if (getSliderStyle() != juce::Slider::LinearHorizontal)
    setSliderStyle(juce::Slider::LinearHorizontal);

  juce::Slider::paint(g);
}

void Fader::mouseDown(const juce::MouseEvent &e) {
  if (onMouseDown)
    onMouseDown(e);
  juce::Slider::mouseDown(e);
}

} // namespace UI
} // namespace S612
