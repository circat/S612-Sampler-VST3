#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

namespace S612 {

/**
 * S612AnalogFilter emulates the MF6CN-50 switched capacitor filter.
 * It is a 4th-order (24dB/oct) Butterworth low-pass filter.
 */
class S612AnalogFilter {
public:
  S612AnalogFilter();

  void prepareToPlay(double sampleRate, int blockSize);
  void setFilterKnob(float knob01);
  float process(float input);

private:
  double m_sampleRate = 44100.0;
  float m_cutoffHz = 18000.0f;
  float m_targetCutoffHz = 18000.0f;

  // Two cascaded biquads for 4th order Butterworth
  juce::IIRFilter m_biquad1;
  juce::IIRFilter m_biquad2;

  void updateCoefficients();

  // Parameter smoothing
  float m_currentCutoffHz = 18000.0f;
};

} // namespace S612
