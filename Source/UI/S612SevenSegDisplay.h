#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace S612 {
namespace UI {

class SevenSegDisplay : public juce::Component, public juce::Timer {
public:
  SevenSegDisplay();
  ~SevenSegDisplay() override;

  void paint(juce::Graphics &g) override;
  void timerCallback() override;

  void setChar(char c);
  void showTempChar(char c, int durationMs = 1500);
  char getCurrentChar() const { return m_currentChar; }
  void startBlinking();
  void stopBlinking();

private:
  char m_currentChar = '0';
  char m_savedChar = '0';
  bool m_tempActive = false;
  bool m_blinkState = true;
  bool m_isBlinking = false;
};

} // namespace UI
} // namespace S612
