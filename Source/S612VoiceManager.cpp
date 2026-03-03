#include "S612VoiceManager.h"
#include "S612Engine.h"
#include "S612Voice.h"

namespace S612 {

S612VoiceManager::~S612VoiceManager() {
  allNotesOff();
  for (auto &vo : m_voiceObjects)
    vo.reset();
}

void S612VoiceManager::prepareToPlay(double sampleRate,
                                     int maximumExpectedSamplesPerBlock) {
  m_sampleRate = sampleRate;
  for (auto &flag : m_voiceActive)
    flag = false;

  // Create voices
  for (int i = 0; i < NumVoices; ++i) {
    m_voiceObjects[i] = std::make_unique<S612Voice>();
    m_voiceObjects[i]->prepareToPlay(sampleRate,
                                     maximumExpectedSamplesPerBlock);
    m_voiceObjects[i]->setEngine(m_engine);
    m_voices[i].voice = m_voiceObjects[i].get();
    m_voices[i].midiNote = -1;
    m_voices[i].active = false;
    m_voices[i].age = 0;
  }
}

void S612VoiceManager::releaseResources() { allNotesOff(); }

void S612VoiceManager::noteOn(int channel, int midiNote, float velocity) {
  // Check MIDI channel filter (0 = omni)
  if (m_midiChannel != 0 && channel != m_midiChannel - 1)
    return;

  // MIDI note range for S612: 36 (C2) – 96 (C7)
  if (midiNote < 0 || midiNote > 127)
    return;

  if (m_monoMode)
    allNotesOff();

  // Stop same note if active (poly mode retrigger)
  int existing = findVoiceForNote(midiNote);
  if (existing >= 0) {
    m_voices[existing].active = false;
    m_voiceActive[existing] = false;
    if (m_voiceObjects[existing])
      m_voiceObjects[existing]->stopNote(false);
  }

  // Find free voice or steal oldest
  int voiceIndex = findFreeVoice();
  if (voiceIndex < 0)
    voiceIndex = findOldestVoice();

  if (voiceIndex >= 0 && m_voiceObjects[voiceIndex]) {
    // Push parameters before starting note
    applyParamsToVoice(voiceIndex);

    m_voices[voiceIndex].midiNote = midiNote;
    m_voices[voiceIndex].age = ++m_voiceAgeCounter;
    m_voices[voiceIndex].active = true;
    m_voiceActive[voiceIndex] = true;

    m_voiceObjects[voiceIndex]->startNote(midiNote, velocity);
  }
}

void S612VoiceManager::noteOff(int channel, int midiNote, bool allowTailOff) {
  int voiceIndex = findVoiceForNote(midiNote);
  if (voiceIndex >= 0) {
    if (m_voiceObjects[voiceIndex])
      m_voiceObjects[voiceIndex]->stopNote(allowTailOff);

    if (!allowTailOff) {
      m_voices[voiceIndex].active = false;
      m_voiceActive[voiceIndex] = false;
    }
  }
}

void S612VoiceManager::allNotesOff() {
  for (int i = 0; i < NumVoices; ++i) {
    if (m_voiceObjects[i])
      m_voiceObjects[i]->stopNote(false);
    m_voices[i].active = false;
    m_voiceActive[i] = false;
  }
}

void S612VoiceManager::applyParamsToVoice(int voiceIndex) {
  if (!m_voiceObjects[voiceIndex])
    return;

  auto *v = m_voiceObjects[voiceIndex].get();
  v->setStartPoint(m_startPoint);
  v->setEndPoint(m_endPoint);
  v->setSplicePoint(m_splicePoint);
  v->setScanMode(m_scanMode);
  v->setDecay(m_decay);
  v->setLfoParams(m_lfoSpeed, m_lfoDepth, m_lfoDelay);
  v->setFilterCutoff(m_filterCutoff);
  v->setTune(m_tune);
  v->setOriginalSampleRate(m_originalSampleRate);
}

void S612VoiceManager::processBlock(juce::AudioBuffer<float> &buffer,
                                    int numSamples) {
  for (int i = 0; i < NumVoices; ++i) {
    if (!m_voiceObjects[i])
      continue;

    S612Voice *v = m_voiceObjects[i].get();
    if (v->isActive()) {
      // Update parameters in real-time (Live Tweaking)
      applyParamsToVoice(i);

      v->renderNextBlock(buffer, 0, numSamples);

      // Clean up finished voices
      if (!v->isActive()) {
        m_voices[i].active = false;
        m_voiceActive[i] = false;
      }
    }
  }
}

int S612VoiceManager::findFreeVoice() {
  for (int i = 0; i < NumVoices; ++i) {
    if (!m_voices[i].active)
      return i;
  }
  return -1;
}

int S612VoiceManager::findOldestVoice() {
  uint64_t oldestAge = UINT64_MAX;
  int oldestIdx = 0;

  for (int i = 0; i < NumVoices; ++i) {
    if (m_voices[i].age < oldestAge) {
      oldestAge = m_voices[i].age;
      oldestIdx = i;
    }
  }
  return oldestIdx;
}

int S612VoiceManager::findVoiceForNote(int midiNote) {
  for (int i = 0; i < NumVoices; ++i) {
    if (m_voices[i].midiNote == midiNote && m_voices[i].active)
      return i;
  }
  return -1;
}

float S612VoiceManager::getAveragePositionNormalized() const {
  float sum = 0.0f;
  int count = 0;
  for (int i = 0; i < NumVoices; ++i) {
    if (m_voiceObjects[i] && m_voiceObjects[i]->isActive()) {
      sum += m_voiceObjects[i]->getPositionNormalized();
      count++;
    }
  }
  return (count > 0) ? (sum / (float)count) : m_startPoint;
}

} // namespace S612
