#pragma once

#include "S612InputStage.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <vector>

namespace S612 {

class S612VoiceManager;

class S612Engine {
public:
  S612Engine() = default;
  ~S612Engine() = default;

  static constexpr int MaxSamples = 262144;

  void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock);
  void releaseResources();

  void processBlock(juce::AudioBuffer<float> &buffer,
                    const juce::MidiBuffer &midiMessages,
                    juce::AudioProcessorValueTreeState &apvts,
                    juce::AudioBuffer<float> &sidechainBuffer);

  std::vector<float> &getSampleBuffer() { return m_sampleBuffer; }
  const std::vector<float> &getSampleBuffer() const { return m_sampleBuffer; }

  void saveSample(const juce::File &file);
  void loadSample(const juce::File &file, double targetRate);

  bool hasSample() const { return m_hasSample; }
  void setHasSample(bool has) { m_hasSample = has; }

  enum class RecordState { IDLE, STANDBY, RECORDING, OVERDUBBING };
  RecordState getRecordState() const { return m_recordState; }
  void setRecordState(RecordState state) { m_recordState = state; }

  int getRecFreqIndex() const { return m_recFreqIndex; }
  void setRecFreqIndex(int index) { m_recFreqIndex = index; }

  int getRootNote() const { return m_rootNote; }
  void setRootNote(int note) { m_rootNote = note; }

  float getInputLevel() const { return m_inputLevel; }
  int getRecordedLength() const { return m_recordedLength; }

  void setMonitoring(bool enabled) { m_monitoringEnabled = enabled; }
  bool isMonitoring() const { return m_monitoringEnabled; }

  void setVoiceManager(S612VoiceManager *vm) { m_voiceManager = vm; }

  bool isManualSpliceActive() const { return m_manualSpliceActive; }

private:
  std::vector<float> m_sampleBuffer;
  S612VoiceManager *m_voiceManager = nullptr;
  S612InputStage m_inputStage;

  bool m_hasSample = false;
  int m_recordedLength = 0;
  int m_writePosition = 0;
  RecordState m_recordState = RecordState::IDLE;
  int m_recFreqIndex = 0;
  int m_rootNote = 60;

  float m_inputLevel = 0.0f;
  float m_lastInputLevel = 0.0f;
  bool m_monitoringEnabled = false;
  double m_sampleRate = 44100.0;

  bool m_lastManualSplice = false;
  bool m_manualSpliceActive = false;
};

} // namespace S612
