#include "S612SevenSegDisplay.h"

namespace S612 {
namespace UI {

SevenSegDisplay::SevenSegDisplay() {}

SevenSegDisplay::~SevenSegDisplay() { stopTimer(); }

void SevenSegDisplay::paint(juce::Graphics &g) {
  g.fillAll(juce::Colour(10, 10, 10));

  juce::Colour onColor(255, 176, 0);
  juce::Colour offColor(60, 40, 0);

  bool drawA = false, drawB = false, drawC = false, drawD = false,
       drawE = false, drawF = false, drawG = false;

  char c = m_currentChar;

  if (m_isBlinking && !m_blinkState) {
    // all off
  } else {
    if (c == '0') {
      drawA = drawB = drawC = drawD = drawE = drawF = true;
    } else if (c == '1') {
      drawB = drawC = true;
    } else if (c == '2') {
      drawA = drawB = drawD = drawE = drawG = true;
    } else if (c == '3') {
      drawA = drawB = drawC = drawD = drawG = true;
    } else if (c == '4') {
      drawB = drawC = drawF = drawG = true;
    } else if (c == '5') {
      drawA = drawC = drawD = drawF = drawG = true;
    } else if (c == '6') {
      drawA = drawC = drawD = drawE = drawF = drawG = true;
    } else if (c == '7') {
      drawA = drawB = drawC = true;
    } else if (c == '8') {
      drawA = drawB = drawC = drawD = drawE = drawF = drawG = true;
    } else if (c == '9') {
      drawA = drawB = drawC = drawD = drawF = drawG = true;
    } else if (c == 'd') {
      drawB = drawC = drawD = drawE = drawG = true;
    } else if (c == 'E') {
      drawA = drawD = drawE = drawF = drawG = true;
    } else if (c == 'G') {
      drawA = drawC = drawD = drawE = drawF = true;
    }
  }

  auto w = getWidth() * 0.8f;
  auto h = getHeight() * 0.8f;
  auto ox = getWidth() * 0.1f;
  auto oy = getHeight() * 0.1f;
  auto thick = w * 0.2f;

  auto drawSeg = [&](bool on, float sx, float sy, float sw, float sh) {
    g.setColour(on ? onColor : offColor);
    g.fillRect(sx, sy, sw, sh);
  };

  drawSeg(drawA, ox + thick, oy, w - 2 * thick, thick);                    // A
  drawSeg(drawB, ox + w - thick, oy + thick, thick, h / 2 - 1.5f * thick); // B
  drawSeg(drawC, ox + w - thick, oy + h / 2 + 0.5f * thick, thick,
          h / 2 - 1.5f * thick);                                    // C
  drawSeg(drawD, ox + thick, oy + h - thick, w - 2 * thick, thick); // D
  drawSeg(drawE, ox, oy + h / 2 + 0.5f * thick, thick,
          h / 2 - 1.5f * thick);                               // E
  drawSeg(drawF, ox, oy + thick, thick, h / 2 - 1.5f * thick); // F
  drawSeg(drawG, ox + thick, oy + h / 2 - 0.5f * thick, w - 2 * thick,
          thick); // G
}

void SevenSegDisplay::timerCallback() {
  if (m_tempActive) {
    m_tempActive = false;
    m_currentChar = m_savedChar;
    if (!m_isBlinking) {
      stopTimer();
    } else {
      startTimer(500);
    }
  } else {
    m_blinkState = !m_blinkState;
  }
  repaint();
}

void SevenSegDisplay::setChar(char c) {
  if (m_tempActive) {
    m_savedChar = c;
  } else if (m_currentChar != c) {
    m_currentChar = c;
    repaint();
  }
}

void SevenSegDisplay::showTempChar(char c, int durationMs) {
  if (!m_tempActive) {
    m_savedChar = m_currentChar;
    m_tempActive = true;
  }
  m_currentChar = c;
  startTimer(durationMs);
  repaint();
}

void SevenSegDisplay::startBlinking() {
  m_isBlinking = true;
  startTimer(500);
}

void SevenSegDisplay::stopBlinking() {
  m_isBlinking = false;
  m_blinkState = true;
  stopTimer();
  repaint();
}

} // namespace UI
} // namespace S612
