#include "S612AnalogFilter.h"
#include <cmath>

namespace S612 {

S612AnalogFilter::S612AnalogFilter() {}

void S612AnalogFilter::prepareToPlay(double sampleRate, int /*blockSize*/) {
  m_sampleRate = sampleRate;
  m_biquad1.reset();
  m_biquad2.reset();

  // Set initial state
  m_currentCutoffHz = m_targetCutoffHz;
  updateCoefficients();
}

void S612AnalogFilter::setFilterKnob(float knob01) {
  // Akai S612 filter range: approx 200Hz to 18kHz
  // Logarithmic scaling
  float minHz = 200.0f;
  float maxHz = 18000.0f;
  m_targetCutoffHz = minHz * std::pow(maxHz / minHz, knob01);
}

float S612AnalogFilter::process(float input) {
  // Smooth cutoff frequency change (to prevent clicks/zipper noise)
  if (std::abs(m_targetCutoffHz - m_currentCutoffHz) > 0.1f) {
    m_currentCutoffHz = m_currentCutoffHz * 0.95f + m_targetCutoffHz * 0.05f;
    updateCoefficients();
  }

  float out = m_biquad1.processSingleSampleRaw(input);
  out = m_biquad2.processSingleSampleRaw(out);

  return out;
}

void S612AnalogFilter::updateCoefficients() {
  // Butterworth 4th order coefficients (2 cascaded biquads)
  // Q values for 4th order Butterworth: 0.541196 and 1.306563

  const double q1 = 0.541196100146197;
  const double q2 = 1.306562964876376;

  auto coeffs1 =
      juce::IIRCoefficients::makeLowPass(m_sampleRate, m_currentCutoffHz, q1);
  auto coeffs2 =
      juce::IIRCoefficients::makeLowPass(m_sampleRate, m_currentCutoffHz, q2);

  m_biquad1.setCoefficients(coeffs1);
  m_biquad2.setCoefficients(coeffs2);
}

} // namespace S612
