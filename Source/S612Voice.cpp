#include "S612Voice.h"
#include "S612Engine.h"
#include <cmath>

namespace S612 {

void S612Voice::prepareToPlay(double sampleRate,
                              int maximumExpectedSamplesPerBlock) {
  m_sampleRate = sampleRate;
  m_filter.prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
}

void S612Voice::startNote(int midiNote, float velocity) {
  m_midiNote = midiNote;
  m_velocity = velocity;
  m_isActive = true;
  m_isReleasing = false;
  m_decayEnvelope = 1.0f;
  m_lfoPhase = 0.0f;
  m_lfoDelayCountdown = (int)(m_lfoDelay * m_sampleRate);

  // Configure scanner
  int totalSamples = (m_engine ? (int)m_engine->getSampleBuffer().size() : 0);
  m_scanner.setStartPoint(m_startPoint, totalSamples);
  m_scanner.setEndPoint(m_endPoint, totalSamples);
  m_scanner.setSplicePoint(m_splicePoint, totalSamples);
  m_scanner.setMode(static_cast<S612Scanner::Mode>(m_scanMode));

  // Configure filter
  m_filter.setFilterKnob(m_filterCutoff);

  // Calculate playback speed: pitch shift relative to root note (C4 = 60)
  int rootNote = (m_engine ? m_engine->getRootNote() : 60);
  float semitones = (float)(midiNote - rootNote);

  // Scaled by the ratio of Recording Rate vs Host Rate
  float rateRatio = (float)(m_originalSampleRate / m_sampleRate);

  // Combine semitones from note + fine tune (cents)
  float totalSemitones = semitones + (m_tuneCents / 100.0f);
  m_pitchRatio = rateRatio * std::pow(2.0f, totalSemitones / 12.0f);

  // Position initialized by start point inside scanner
  m_scanner.reset(m_startPoint * (float)totalSamples);
}

void S612Voice::stopNote(bool allowTailOff) {
  if (allowTailOff)
    m_isReleasing = true;
  else {
    m_isActive = false;
    m_isReleasing = false;
  }
}

void S612Voice::renderNextBlock(juce::AudioBuffer<float> &buffer,
                                int startSample, int numSamples) {
  if (!m_isActive || !m_engine || !m_engine->hasSample())
    return;

  const auto &sampleBuf = m_engine->getSampleBuffer();
  int totalSamples = m_engine->getRecordedLength();
  if (totalSamples == 0)
    return;
  // Sync scanner + filter params just in case they changed
  m_scanner.setStartPoint(m_startPoint, totalSamples);
  m_scanner.setEndPoint(m_endPoint, totalSamples);
  m_scanner.setSplicePoint(m_splicePoint, totalSamples);
  m_scanner.setMode(static_cast<S612Scanner::Mode>(m_scanMode));

  m_filter.setFilterKnob(m_filterCutoff);

  float outputGain = m_velocity;

  for (int i = startSample; i < startSample + numSamples; ++i) {
    if (!m_isActive || m_scanner.isFinished()) {
      m_isActive = false;
      break;
    }

    // Decay envelope (Release)
    if (m_isReleasing) {
      // Scale m_decay (0-1) to a max of 10 seconds
      float decayTimeSeconds = m_decay * 10.0f;
      float decaySamples = decayTimeSeconds * (float)m_sampleRate;

      if (decaySamples < 1.0f) {
        m_isActive = false;
        break;
      }

      // Exponential decay: hits 0.001 (-60dB) in decaySamples
      float decayRate = 1.0f - std::pow(0.001f, 1.0f / decaySamples);
      m_decayEnvelope *= (1.0f - decayRate);

      if (m_decayEnvelope < 0.0001f) {
        m_isActive = false;
        break;
      }
    }

    // LFO (vibrato / pitch mod)
    float lfoMod = 0.0f;
    if (m_lfoDelayCountdown > 0) {
      --m_lfoDelayCountdown;
    } else {
      m_lfoPhase += m_lfoSpeed * 10.0f / m_sampleRate;
      if (m_lfoPhase > juce::MathConstants<float>::twoPi)
        m_lfoPhase -= juce::MathConstants<float>::twoPi;
      lfoMod = std::sin(m_lfoPhase) * m_lfoDepth * 0.05f; // ±5 semitones max
    }

    float pitch = m_pitchRatio * std::pow(2.0f, lfoMod / 12.0f);

    // Get next index from scanner
    float currentIdx = m_scanner.getNextSampleIndex(pitch, totalSamples);

    // Point Sampling (Nearest Neighbor)
    // The S612 didn't interpolate digitally. It used a variable-rate
    // clock for the D/A wandler, which results in raw aliasing.
    int pos0 = (int)(currentIdx + 0.5f);
    float outSample =
        (pos0 >= 0 && pos0 < totalSamples) ? sampleBuf[pos0] : 0.0f;

    // Apply envelope + velocity
    float envSample = outSample * m_decayEnvelope * outputGain;

    // Apply 4th-order Analog Filter (Barbara's Section)
    float out = m_filter.process(envSample);

    // Write to all channels
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
      buffer.addSample(ch, i, out);
  }
}

float S612Voice::getPositionNormalized() const {
  if (!m_engine || !m_engine->hasSample())
    return 0.0f;
  int len = m_engine->getRecordedLength();
  if (len <= 0)
    return 0.0f;
  return m_scanner.getPosition() / static_cast<float>(len);
}

} // namespace S612
