#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <map>
#include <string>

namespace S612 {

class S612VoiceManager;

class S612MidiHandler {
public:
  S612MidiHandler() = default;
  ~S612MidiHandler() = default;

  void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock);
  void releaseResources();

  void setVoiceManager(S612VoiceManager *vm) { m_voiceManager = vm; }
  void setAPVTS(juce::AudioProcessorValueTreeState *apvts) { m_apvts = apvts; }

  void processBlock(const juce::MidiBuffer &midiBuffer, int numSamples);

  void setPitchBend(float value) { m_pitchBend = value; }
  float getPitchBend() const { return m_pitchBend; }

  void setModWheel(float value) { m_modWheel = value; }
  float getModWheel() const { return m_modWheel; }

  void setSustain(bool active) { m_sustainActive = active; }
  bool isSustainActive() const { return m_sustainActive; }

  void startLearning(const juce::String &paramID) {
    m_learningParamID = paramID;
  }
  bool isLearning() const { return m_learningParamID.isNotEmpty(); }
  void forgetMapping(const juce::String &paramID);

private:
  S612VoiceManager *m_voiceManager = nullptr;
  juce::AudioProcessorValueTreeState *m_apvts = nullptr;

  float m_pitchBend = 0.0f; // -1.0 to 1.0
  float m_modWheel = 0.0f;  // 0.0 to 1.0
  bool m_sustainActive = false;

  juce::String m_learningParamID;
  std::map<int, juce::String> m_ccMappings;
};

} // namespace S612
