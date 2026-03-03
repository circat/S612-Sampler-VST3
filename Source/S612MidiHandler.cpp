#include "S612MidiHandler.h"
#include "S612VoiceManager.h"

namespace S612 {

void S612MidiHandler::prepareToPlay(double sampleRate,
                                    int maximumExpectedSamplesPerBlock) {}

void S612MidiHandler::releaseResources() {}

void S612MidiHandler::processBlock(const juce::MidiBuffer &midiBuffer,
                                   int numSamples) {
  if (!m_voiceManager)
    return;

  for (const auto metadata : midiBuffer) {
    const auto message = metadata.getMessage();

    if (message.isNoteOn()) {
      int note = message.getNoteNumber();
      float velocity = message.getVelocity() / 127.0f;
      m_voiceManager->noteOn(message.getChannel(), note, velocity);
    } else if (message.isNoteOff()) {
      int note = message.getNoteNumber();
      m_voiceManager->noteOff(message.getChannel(), note, !m_sustainActive);
    } else if (message.isPitchWheel()) {
      int value = message.getPitchWheelValue();
      m_pitchBend = (value - 8192) / 8192.0f;
    } else if (message.isController()) {
      int cc = message.getControllerNumber();
      float val = message.getControllerValue() / 127.0f;

      // MIDI Learn logic
      if (m_learningParamID.isNotEmpty()) {
        m_ccMappings[cc] = m_learningParamID;
        m_learningParamID = "";
      }

      // Dispatch learned CCs
      auto it = m_ccMappings.find(cc);
      if (it != m_ccMappings.end() && m_apvts) {
        if (auto *p = m_apvts->getParameter(it->second))
          p->setValueNotifyingHost(val);
      }

      if (cc == 1) // Mod Wheel
      {
        m_modWheel = val;
      } else if (cc == 64) // Sustain
      {
        bool sustainOn = message.getControllerValue() >= 64;
        if (sustainOn && !m_sustainActive) {
          m_sustainActive = true;
        } else if (!sustainOn && m_sustainActive) {
          m_sustainActive = false;
          m_voiceManager->allNotesOff();
        }
      }
    }
  }
}

void S612MidiHandler::forgetMapping(const juce::String &paramID) {
  for (auto it = m_ccMappings.begin(); it != m_ccMappings.end();) {
    if (it->second == paramID)
      it = m_ccMappings.erase(it);
    else
      ++it;
  }
}

} // namespace S612
