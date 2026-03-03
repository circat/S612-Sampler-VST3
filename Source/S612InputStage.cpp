#include "S612InputStage.h"
#include <algorithm>
#include <cmath>

namespace S612 {

void S612InputStage::prepareToPlay(double sampleRate, int /*blockSize*/) {
  m_sampleRate = sampleRate;
  m_sampleCount = 0.0;
  m_lastSampleValue = 0.0f;
}

float S612InputStage::processSampleInput(float sample) {
  // 1. Time-Quantization (Aliasing/Decimation)
  // The Akai S612 maximum sample rate was 32kHz (at 1 sec) or less.
  // We emulate a selectable clock for the primary sampling.
  const double samplesPerTarget = m_sampleRate / m_targetRate;

  m_sampleCount += 1.0;
  if (m_sampleCount >= samplesPerTarget) {
    m_sampleCount -= samplesPerTarget;

    // 2. Apply recording level gain
    float gained = sample * m_recLevel;

    // 3. Hard Clip (1985 ADC behavior)
    float clipped = std::max(-1.0f, std::min(1.0f, gained));

    // 4. 12-Bit Quantization
    m_lastSampleValue = std::floor(clipped * 2047.5f + 0.5f) / 2047.5f;
  }

  return m_lastSampleValue;
}

} // namespace S612
