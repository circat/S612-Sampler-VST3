#include "S612InputMeter.h"

namespace S612 {
namespace UI {

InputMeter::InputMeter() { startTimer(30); }

InputMeter::~InputMeter() { stopTimer(); }

void InputMeter::setLevel(float level) {
  // Input is linear 0..1+
  if (level > m_currentLevel) {
    m_currentLevel = level;
  }
}

void InputMeter::timerCallback() {
  // Decay level for visual smoothness
  m_currentLevel *= 0.85f;
  if (m_currentLevel < 0.001f)
    m_currentLevel = 0.0f;

  if (m_currentLevel > m_peakLevel) {
    m_peakLevel = m_currentLevel;
  }
  m_peakLevel *= 0.98f;

  repaint();
}

void InputMeter::paint(juce::Graphics &g) {
  const float w = (float)getWidth();
  const float h = (float)getHeight();

  // Background: dark recessed area
  g.setColour(juce::Colour(0xFF050505));
  g.fillRoundedRectangle(0, 0, w, h, 2.0f);

  const int numSegments = 12;
  const float padding = 2.0f;
  const float segWidth = (w - (numSegments + 1) * padding) / numSegments;
  const float segHeight = h - 2 * padding;

  // Original S612 Meter Scale:
  // Green segments until target, then red for clip (+3dB)

  for (int i = 0; i < numSegments; ++i) {
    float segThreshold = (float)(i + 1) / (float)numSegments;

    bool isOn = m_currentLevel >= segThreshold;
    bool isPeak = m_peakLevel >= segThreshold &&
                  m_peakLevel < segThreshold + (1.0f / numSegments);

    float x = padding + i * (segWidth + padding);
    float y = padding;

    // Color logic
    juce::Colour baseColor;
    if (i < 9) {
      // Green/Yellow zone
      baseColor = juce::Colour(50, 200, 50); // Bright Green
    } else {
      // Red zone (+3dB area)
      baseColor = juce::Colour(255, 50, 50); // Bright Red
    }

    if (isOn) {
      g.setColour(baseColor);
      g.fillRect(x, y, segWidth, segHeight);

      // Subtle glow/inner shine
      g.setColour(baseColor.withAlpha(0.3f));
      g.drawRect(x, y, segWidth, segHeight, 1.0f);
    } else {
      // Off state (dark dim version of the color)
      g.setColour(baseColor.withMultipliedBrightness(0.15f));
      g.fillRect(x, y, segWidth, segHeight);
    }

    // Peak indicator (single thin line above segment)
    if (isPeak) {
      g.setColour(juce::Colours::white.withAlpha(0.6f));
      g.drawHorizontalLine((int)y, x, x + segWidth);
    }
  }

  // "+3" Text label above the red segments
  g.setColour(juce::Colour(0xFFBBBBBB));
  g.setFont(juce::Font("Arial", 8.0f, juce::Font::bold));
  // The last 3 segments are the red ones
  float textX = padding + 9 * (segWidth + padding);
  g.drawText("+3", (int)textX, 0, (int)(3 * segWidth), 10,
             juce::Justification::centred);
}

} // namespace UI
} // namespace S612
