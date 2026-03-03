#include "S612LookAndFeel.h"

namespace S612 {
namespace UI {

LookAndFeel::LookAndFeel() {
  // Dark colour scheme matching S612 hardware
  setColour(juce::Slider::thumbColourId, juce::Colour(0xFFFFFFFF));
  setColour(juce::Slider::trackColourId, juce::Colour(0xFF444444));
  setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF333333));
  setColour(juce::TextButton::textColourOffId, juce::Colour(0xFFCCCCCC));
  setColour(juce::TextButton::textColourOnId, juce::Colour(0xFFFFFFFF));
}

LookAndFeel::~LookAndFeel() {}

// ─── Rotary Knobs
// ─────────────────────────────────────────────────────────────
void LookAndFeel::drawRotarySlider(juce::Graphics &g, int x, int y, int width,
                                   int height, float sliderPos,
                                   const float rotaryStartAngle,
                                   const float rotaryEndAngle,
                                   juce::Slider &slider) {
  const juce::String name = slider.getName();

  auto radius = (float)juce::jmin(width / 2, height / 2) - 3.0f;
  auto centreX = (float)x + (float)width * 0.5f;
  auto centreY = (float)y + (float)height * 0.5f;
  auto rx = centreX - radius;
  auto ry = centreY - radius;
  auto rw = radius * 2.0f;
  auto angle =
      rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

  // ── Pick colours based on knob name ──────────────────────────────────────
  juce::Colour bodyColour, capColour, pointerColour;

  if (name == "REC LEVEL") {
    bodyColour = juce::Colour(0xFF2A2A2A);
    capColour = juce::Colour(0xFFE06800); // Orange
    pointerColour = juce::Colour(0xFFFFFFFF);
  } else if (name == "MONITOR") {
    bodyColour = juce::Colour(0xFF2A2A2A);
    capColour = juce::Colour(0xFFDDDDDD); // White/silver
    pointerColour = juce::Colour(0xFF111111);
  } else if (name == "TUNE") {
    bodyColour = juce::Colour(0xFF282828);
    capColour = juce::Colour(0xFFCCCCCC); // Light grey cap (S612 TUNE is white)
    pointerColour = juce::Colour(0xFF111111);
  } else {
    // LFO / OUTPUT knobs – teal/green cap
    bodyColour = juce::Colour(0xFF222222);
    capColour = juce::Colour(0xFF00897B); // Teal
    pointerColour = juce::Colour(0xFFFFFFFF);
  }

  // ── Body shadow ───────────────────────────────────────────────────────────
  g.setColour(juce::Colour(0xFF111111));
  g.fillEllipse(rx + 2, ry + 2, rw, rw);

  // ── Body ──────────────────────────────────────────────────────────────────
  {
    juce::ColourGradient grad(bodyColour.brighter(0.15f),
                              centreX - radius * 0.3f, centreY - radius * 0.3f,
                              bodyColour.darker(0.5f), centreX + radius * 0.3f,
                              centreY + radius * 0.3f, true);
    g.setGradientFill(grad);
    g.fillEllipse(rx, ry, rw, rw);
  }

  // ── Grip lines (tactile ridges) ───────────────────────────────────────────
  g.setColour(bodyColour.darker(0.6f));
  for (int i = 0; i < 8; ++i) {
    float gripAngle = (float)i / 8.0f * juce::MathConstants<float>::twoPi;
    float innerR = radius * 0.72f;
    float outerR = radius * 0.94f;
    float cx1 = centreX + std::sin(gripAngle) * innerR;
    float cy1 = centreY - std::cos(gripAngle) * innerR;
    float cx2 = centreX + std::sin(gripAngle) * outerR;
    float cy2 = centreY - std::cos(gripAngle) * outerR;
    g.drawLine(cx1, cy1, cx2, cy2, 1.2f);
  }

  // ── Coloured cap (inner circle) ───────────────────────────────────────────
  float capR = radius * 0.62f;
  {
    juce::ColourGradient capGrad(capColour.brighter(0.2f),
                                 centreX - capR * 0.4f, centreY - capR * 0.4f,
                                 capColour.darker(0.3f), centreX + capR * 0.4f,
                                 centreY + capR * 0.4f, true);
    g.setGradientFill(capGrad);
    g.fillEllipse(centreX - capR, centreY - capR, capR * 2.0f, capR * 2.0f);
  }

  // ── Pointer line ──────────────────────────────────────────────────────────
  juce::Path pointer;
  float pointerLen = radius * 0.85f;
  float pointerW = 2.5f;
  pointer.addRoundedRectangle(-pointerW * 0.5f, -pointerLen, pointerW,
                              pointerLen * 0.85f, 1.0f);
  pointer.applyTransform(
      juce::AffineTransform::rotation(angle).translated(centreX, centreY));

  g.setColour(pointerColour);
  g.fillPath(pointer);

  // ── Arc indicator ─────────────────────────────────────────────────────────
  juce::Path arc;
  arc.addCentredArc(centreX, centreY, radius + 2.0f, radius + 2.0f, 0.0f,
                    rotaryStartAngle, angle, true);
  g.setColour(capColour.withAlpha(0.6f));
  g.strokePath(arc, juce::PathStrokeType(1.5f));
}

// ─── Linear (horizontal) Sliders – S612 Fader
// ─────────────────────────────────
void LookAndFeel::drawLinearSlider(juce::Graphics &g, int x, int y, int width,
                                   int height, float sliderPos,
                                   float /*minSliderPos*/,
                                   float /*maxSliderPos*/,
                                   const juce::Slider::SliderStyle style,
                                   juce::Slider & /*slider*/) {
  if (style != juce::Slider::LinearHorizontal) {
    // Don't call base with non-horizontal style – just draw nothing for
    // non-horizontal sliders (we only use horizontal faders in this plugin).
    return;
  }

  float trackY = (float)(y + height / 2);

  // Track background
  g.setColour(juce::Colour(0xFF111111));
  g.fillRoundedRectangle((float)x, trackY - 2.5f, (float)width, 5.0f, 2.5f);
  g.setColour(juce::Colour(0xFF4A4A4A));
  g.drawRoundedRectangle((float)x, trackY - 2.5f, (float)width, 5.0f, 2.5f,
                         1.0f);

  // Filled portion
  float thumbX = sliderPos;
  g.setColour(juce::Colour(0xFF50A0C0));
  g.fillRoundedRectangle((float)x, trackY - 2.0f, thumbX - (float)x, 4.0f,
                         2.0f);

  // Thumb (fader cap)
  float thumbW = 14.0f;
  float thumbH = (float)height * 0.75f;
  float thumbLeft = thumbX - thumbW * 0.5f;
  float thumbTop = (float)y + ((float)height - thumbH) * 0.5f;

  // Shadow
  g.setColour(juce::Colour(0xFF111111));
  g.fillRoundedRectangle(thumbLeft + 1, thumbTop + 2, thumbW, thumbH, 2.0f);

  // Body
  juce::ColourGradient grad(juce::Colour(0xFF707070), thumbLeft, thumbTop,
                            juce::Colour(0xFF393939), thumbLeft + thumbW,
                            thumbTop + thumbH, false);
  g.setGradientFill(grad);
  g.fillRoundedRectangle(thumbLeft, thumbTop, thumbW, thumbH, 2.0f);

  // White centre line
  g.setColour(juce::Colour(0xFFEEEEEE));
  g.fillRect(thumbX - 0.75f, thumbTop + thumbH * 0.2f, 1.5f, thumbH * 0.6f);
}

// ─── Buttons
// ──────────────────────────────────────────────────────────────────
void LookAndFeel::drawButtonBackground(
    juce::Graphics &g, juce::Button &button,
    const juce::Colour & /*backgroundColour*/, bool isHighlighted,
    bool isDown) {
  auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
  bool toggleOn = button.getToggleState();
  auto name = button.getName();

  // Pick button colour from S612 hardware palette
  juce::Colour base;
  if (name == "REC_MODE_NEW" || name == "REC_MODE_DUB") {
    base = toggleOn ? juce::Colour(0xFFCC2020) : juce::Colour(0xFF882020);
  } else if (name.contains("DATA")) {
    // Cream / ivory data buttons
    base = toggleOn ? juce::Colour(0xFFEEEED0) : juce::Colour(0xFFBBBBA0);
  } else if (name.contains("MODE_ONE") || name.contains("MODE_LOOP") ||
             name.contains("MODE_ALT")) {
    // Teal scan mode buttons
    base = toggleOn ? juce::Colour(0xFF00BFA5) : juce::Colour(0xFF006B5E);
  } else if (name.contains("MODE_MANU")) {
    // Teal manual splice
    base = toggleOn ? juce::Colour(0xFF00BFA5) : juce::Colour(0xFF006B5E);
  } else {
    // Generic dark buttons
    base = toggleOn ? juce::Colour(0xFF555555) : juce::Colour(0xFF333333);
  }

  if (isDown)
    base = base.darker(0.25f);
  if (isHighlighted && !isDown)
    base = base.brighter(0.1f);

  // Bevel
  g.setColour(isDown ? base.darker(0.4f) : base.brighter(0.25f));
  g.fillRoundedRectangle(bounds, 2.0f);
  g.setColour(base);
  g.fillRoundedRectangle(bounds.reduced(1.0f, isDown ? 2.0f : 0.0f), 2.0f);

  // LED dot for REC buttons
  if (name == "REC_MODE_NEW" || name == "REC_MODE_DUB") {
    g.setColour(toggleOn ? juce::Colours::red : juce::Colour(0xFF440000));
    g.fillEllipse(bounds.getX() + 3.0f, bounds.getCentreY() - 2.5f, 5.0f, 5.0f);
  }

  // Bottom shadow
  g.setColour(isDown ? base.brighter(0.3f) : base.darker(0.5f));
  g.drawHorizontalLine((int)bounds.getBottom(), bounds.getX(),
                       bounds.getRight());
}

void LookAndFeel::drawButtonText(juce::Graphics &g, juce::TextButton &button,
                                 bool /*isHighlighted*/, bool /*isDown*/) {
  auto name = button.getName();
  bool light = name.contains("DATA");

  g.setColour(light ? juce::Colour(0xFF222222) : juce::Colour(0xFFEEEEEE));
  g.setFont(juce::Font("Arial", 7.5f, juce::Font::bold));
  g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(2),
                   juce::Justification::centred, 2);
}

} // namespace UI
} // namespace S612
