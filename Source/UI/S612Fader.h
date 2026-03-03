#pragma once
#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>


namespace S612 {
namespace UI {

class Fader : public juce::Slider {
public:
  Fader();
  ~Fader() override;

  void paint(juce::Graphics &g) override;
  void mouseDown(const juce::MouseEvent &e) override;

  std::function<void(const juce::MouseEvent &)> onMouseDown;
};

} // namespace UI
} // namespace S612
