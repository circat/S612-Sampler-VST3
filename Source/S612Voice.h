#pragma once
#include "S612AnalogFilter.h"
#include "S612Scanner.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace S612 {

class S612Engine;

class S612Voice {
public:
  S612Voice() = default;
  ~S612Voice() = default;

  void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock);

  void startNote(int midiNote, float velocity);
  void stopNote(bool allowTailOff);
  bool isActive() const { return m_isActive; }

  void renderNextBlock(juce::AudioBuffer<float> &buffer, int startSample,
                       int numSamples);

  // Engine reference for sample buffer access
  void setEngine(S612Engine *engine) { m_engine = engine; }

  // Parameters
  void setStartPoint(float v) { m_startPoint = v; }
  void setEndPoint(float v) { m_endPoint = v; }
  void setSplicePoint(float v) { m_splicePoint = v; }
  void setScanMode(int mode) { m_scanMode = mode; }
  void setDecay(float v) { m_decay = v; }
  void setLfoSpeed(float v) { m_lfoSpeed = v; }
  void setLfoDepth(float v) { m_lfoDepth = v; }
  void setLfoDelay(float v) { m_lfoDelay = v; }
  void setLfoParams(float speed, float depth, float delay) {
    m_lfoSpeed = speed;
    m_lfoDepth = depth;
    m_lfoDelay = delay;
  }
  void setFilterCutoff(float v) { m_filterCutoff = v; }
  void setOriginalSampleRate(double rate) { m_originalSampleRate = rate; }
  void setTune(float cents) { m_tuneCents = cents; }
  float getPositionNormalized() const;

private:
  S612Engine *m_engine = nullptr;

  bool m_isActive = false;
  bool m_isReleasing = false;
  int m_midiNote = 60;
  float m_velocity = 0.0f;

  double m_sampleRate = 44100.0;
  S612Scanner m_scanner;
  S612AnalogFilter m_filter;
  float m_pitchRatio = 1.0f;

  // Parameters
  float m_startPoint = 0.0f;
  float m_endPoint = 1.0f;
  float m_splicePoint = 0.8f;
  int m_scanMode = 0; // 0=OneShot, 1=Loop, 2=Alternating
  float m_decay = 0.5f;
  float m_filterCutoff = 1.0f;
  double m_originalSampleRate = 16000.0;
  float m_tuneCents = 0.0f;
  float m_lfoSpeed = 0.5f;
  float m_lfoDepth = 0.0f;
  float m_lfoDelay = 0.0f;

  // Envelope & LFO state
  float m_decayEnvelope = 1.0f;
  float m_lfoPhase = 0.0f;
  int m_lfoDelayCountdown = 0;
};

} // namespace S612
