#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace S612 {
namespace UI {

class Knob : public juce::Slider {
public:
  Knob();
  ~Knob() override;

  enum Style { STYLE_STANDARD, STYLE_LFO };

  void setKnobStyle(Style style);
  void paint(juce::Graphics &g) override;
  void mouseDown(const juce::MouseEvent &e) override;

  std::function<void(const juce::MouseEvent &)> onMouseDown;

private:
  Style m_style = STYLE_STANDARD;
};

} // namespace UI
} // namespace S612
