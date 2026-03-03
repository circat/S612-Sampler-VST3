#pragma once

#include "S612Voice.h"
#include <array>
#include <atomic>
#include <juce_audio_basics/juce_audio_basics.h>
#include <memory>
#include <vector>

namespace S612 {

class S612Engine;

class S612VoiceManager {
public:
  S612VoiceManager() = default;
  ~S612VoiceManager();

  void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock);
  void releaseResources();
  void processBlock(juce::AudioBuffer<float> &buffer, int numSamples);

  void noteOn(int channel, int noteNumber, float velocity);
  void noteOff(int channel, int noteNumber, bool allowTail);
  void allNotesOff();

  void setEngine(S612Engine *e) { m_engine = e; }
  void setMidiChannel(int ch) { m_midiChannel = ch; }

  // Parameter setters
  void setStartPoint(float v) { m_startPoint = v; }
  void setEndPoint(float v) { m_endPoint = v; }
  void setSplicePoint(float v) { m_splicePoint = v; }
  void setScanMode(int v) { m_scanMode = v; }
  void setDecay(float v) { m_decay = v; }
  void setLfoParams(float speed, float depth, float delay) {
    m_lfoSpeed = speed;
    m_lfoDepth = depth;
    m_lfoDelay = delay;
  }
  void setFilterCutoff(float v) { m_filterCutoff = v; }
  void setTune(float v) { m_tune = v; }
  void setMonoMode(bool mono) { m_monoMode = mono; }
  void setOriginalSampleRate(double rate) { m_originalSampleRate = rate; }
  float getAveragePositionNormalized() const;

  // Voice activity for LEDs
  std::array<std::atomic<bool>, 6> m_voiceActive;

  static constexpr int NumVoices = 6;

private:
  struct VoiceInstance {
    int midiNote = -1;
    bool active = false;
    uint64_t age = 0;
    S612Voice *voice = nullptr;
  };

  std::array<VoiceInstance, NumVoices> m_voices;
  std::array<std::unique_ptr<S612Voice>, NumVoices> m_voiceObjects;

  S612Engine *m_engine = nullptr;
  double m_sampleRate = 44100.0;
  int m_midiChannel = 0;
  uint64_t m_voiceAgeCounter = 0;

  float m_startPoint = 0.0f;
  float m_endPoint = 1.0f;
  float m_splicePoint = 0.8f;
  int m_scanMode = 0;
  float m_decay = 0.5f;
  float m_lfoSpeed = 0.5f;
  float m_lfoDepth = 0.0f;
  float m_lfoDelay = 0.0f;
  float m_filterCutoff = 1.0f;
  float m_tune = 0.0f;
  bool m_monoMode = false;
  double m_originalSampleRate = 16000.0;

  void applyParamsToVoice(int voiceIndex);
  int findFreeVoice();
  int findOldestVoice();
  int findVoiceForNote(int midiNote);
};

} // namespace S612
