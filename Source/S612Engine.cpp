#include "S612Engine.h"
#include "S612Parameters.h"
#include "S612VoiceManager.h"
#include <algorithm>
#include <cmath>
#include <juce_audio_formats/juce_audio_formats.h>

namespace S612 {

void S612Engine::prepareToPlay(double sampleRate,
                               int maximumExpectedSamplesPerBlock) {
  m_sampleBuffer.resize(MaxSamples, 0.0f);
  m_sampleRate = sampleRate;
  m_writePosition = 0;
  m_inputLevel = 0.0f;
  m_inputStage.prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
}

void S612Engine::releaseResources() {}

void S612Engine::processBlock(juce::AudioBuffer<float> &buffer,
                              const juce::MidiBuffer &midiMessages,
                              juce::AudioProcessorValueTreeState &apvts,
                              juce::AudioBuffer<float> &sidechainBuffer) {
  int numSamples = buffer.getNumSamples();

  // ── 1. Read parameters ──────────────────────────────────────────────────
  auto getParam = [&](const char *id, float fallback) -> float {
    auto *p = apvts.getRawParameterValue(id);
    return p ? p->load() : fallback;
  };

  float recLevelVal = getParam(recLevel, 1.0f);
  float startPt = getParam(startPoint, 0.0f);
  float endPt = getParam(endPoint, 1.0f);
  float splicePt = getParam(splicePoint, 0.0f);
  float outputLvl = getParam(outputLevel, 0.8f);
  float monitorLvlVal = getParam(monitorLevel, 0.8f);
  float decayVal = getParam(decay, 0.5f);
  float filterVal = getParam(filter, 1.0f);
  float lfoSpeedVal = getParam(lfoSpeed, 0.5f);
  float lfoDepthVal = getParam(lfoDepth, 0.0f);
  float lfoDelayVal = getParam(lfoDelay, 0.0f);
  int scanModeVal = (int)getParam(scanMode, 0.0f);
  int recFreqIdx = (int)getParam(recFreq, 0.0f);
  bool monoModeVal = getParam(monoPoly, 0.0f) > 0.5f;
  float tuneVal = getParam(tune, 0.0f);

  double targetRates[] = {32000.0, 16000.0, 8000.0};
  double targetRate = targetRates[juce::jlimit(0, 2, recFreqIdx)];
  m_inputStage.setTargetSampleRate(targetRate);

  // ── 2. Track input level ────────────────────────────────────────────────
  float currentBlockMax = 0.0f;
  const bool hasSidechain = sidechainBuffer.getNumChannels() > 0 &&
                            sidechainBuffer.getNumSamples() > 0;

  if (hasSidechain) {
    for (int ch = 0; ch < sidechainBuffer.getNumChannels(); ++ch) {
      const float *data = sidechainBuffer.getReadPointer(ch);
      if (data) {
        for (int i = 0; i < sidechainBuffer.getNumSamples(); ++i) {
          float peak = std::abs(data[i]) * recLevelVal;
          currentBlockMax = juce::jmax(currentBlockMax, peak);
        }
      }
    }
  }

  m_inputLevel = currentBlockMax * 0.3f + m_lastInputLevel * 0.7f;
  m_lastInputLevel = m_inputLevel;

  // ── 3. State Machine ────────────────────────────────────────────────────
  if (m_recordState == RecordState::STANDBY && currentBlockMax > 0.01f) {
    m_recordState = RecordState::RECORDING;
    m_writePosition = 0;
    m_recordedLength = 0;
    m_hasSample = false;

    if (!midiMessages.isEmpty()) {
      for (const auto metadata : midiMessages) {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
          m_rootNote = msg.getNoteNumber();
          break;
        }
      }
    }
  }

  // ── 4. Recording ────────────────────────────────────────────────────────
  if (hasSidechain && (m_recordState == RecordState::RECORDING ||
                       m_recordState == RecordState::OVERDUBBING)) {
    m_inputStage.setRecLevel(recLevelVal);
    if (const float *src = sidechainBuffer.getReadPointer(0)) {
      int nSamples = juce::jmin(numSamples, MaxSamples - m_writePosition);
      if (nSamples > 0) {
        for (int i = 0; i < nSamples; ++i) {
          float s = m_inputStage.processSampleInput(src[i]);
          if (m_recordState == RecordState::RECORDING)
            m_sampleBuffer[m_writePosition + i] = s;
          else
            m_sampleBuffer[m_writePosition + i] = juce::jlimit(
                -1.0f, 1.0f, m_sampleBuffer[m_writePosition + i] * 0.5f + s);
        }
        m_writePosition += nSamples;
        m_recordedLength = juce::jmax(m_recordedLength, m_writePosition);
        m_hasSample = true;
      } else {
        m_recordState = RecordState::IDLE;
        m_writePosition = 0;
      }
    }
  }

  // ── 5. Push parameters ──────────────────────────────────────────────────
  if (m_voiceManager) {
    // Manual Splice Mode Toggle
    bool msTrigger = getParam(manualSplice, 0.0f) > 0.5f;
    if (msTrigger && !m_lastManualSplice) {
      m_manualSpliceActive = !m_manualSpliceActive;
      if (m_manualSpliceActive) {
        float pos = m_voiceManager->getAveragePositionNormalized();
        if (auto *p = apvts.getParameter(splicePoint))
          p->setValueNotifyingHost(pos);
      }
    }
    m_lastManualSplice = msTrigger;

    // Use captured splice, or follow startPoint if inactive
    float finalSplice = m_manualSpliceActive ? splicePt : startPt;

    m_voiceManager->setStartPoint(startPt);
    m_voiceManager->setEndPoint(endPt);
    m_voiceManager->setSplicePoint(finalSplice);
    m_voiceManager->setScanMode(scanModeVal);
    m_voiceManager->setDecay(decayVal);
    m_voiceManager->setLfoParams(lfoSpeedVal, lfoDepthVal, lfoDelayVal);
    m_voiceManager->setFilterCutoff(filterVal);
    m_voiceManager->setTune(tuneVal);
    m_voiceManager->setMonoMode(monoModeVal);
    m_voiceManager->setOriginalSampleRate(targetRate);
  }

  // ── 6. Rendering ────────────────────────────────────────────────────────
  buffer.clear();
  if (m_voiceManager && m_hasSample)
    m_voiceManager->processBlock(buffer, numSamples);

  for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
    float *data = buffer.getWritePointer(ch);
    for (int i = 0; i < numSamples; ++i)
      data[i] *= outputLvl;
  }

  // ── 7. MONITORING ───────────────────────────────────────────────────────
  if (hasSidechain && monitorLvlVal > 0.001f) {
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
      int scCh = juce::jmin(ch, sidechainBuffer.getNumChannels() - 1);
      if (const float *src = sidechainBuffer.getReadPointer(scCh)) {
        float *dst = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
          dst[i] += src[i] * recLevelVal * monitorLvlVal;
      }
    }
  }
}

void S612Engine::saveSample(const juce::File &file) {
  if (m_sampleBuffer.empty())
    return;
  file.deleteFile();
  if (auto outStream =
          std::unique_ptr<juce::FileOutputStream>(file.createOutputStream())) {
    juce::WavAudioFormat wavFormat;
    if (auto writer =
            std::unique_ptr<juce::AudioFormatWriter>(wavFormat.createWriterFor(
                outStream.get(), m_sampleRate, 1, 16, {}, 0))) {
      outStream.release();
      juce::AudioBuffer<float> b(1, (int)m_sampleBuffer.size());
      for (int i = 0; i < (int)m_sampleBuffer.size(); ++i)
        b.setSample(0, i, m_sampleBuffer[i]);
      writer->writeFromAudioSampleBuffer(b, 0, b.getNumSamples());
    }
  }
}

void S612Engine::loadSample(const juce::File &file, double targetRate) {
  juce::AudioFormatManager formatManager;
  formatManager.registerBasicFormats();

  if (auto reader = std::unique_ptr<juce::AudioFormatReader>(
          formatManager.createReaderFor(file))) {
    double sourceRate = reader->sampleRate;
    int sourceLen = (int)reader->lengthInSamples;

    juce::AudioBuffer<float> sourceBuffer(1, sourceLen);
    reader->read(&sourceBuffer, 0, sourceLen, 0, true, true);

    // Calculate target length based on rate ratio
    double ratio = targetRate / sourceRate;
    int targetLen = juce::jmin((int)MaxSamples, (int)(sourceLen * ratio));

    m_sampleBuffer.assign(MaxSamples, 0.0f);

    if (std::abs(ratio - 1.0) < 0.0001) {
      // Direct copy if rates match
      for (int i = 0; i < targetLen; ++i)
        m_sampleBuffer[i] = sourceBuffer.getSample(0, i);
    } else {
      // Characterful resampling to target hardware rate
      for (int i = 0; i < targetLen; ++i) {
        float sourcePos = (float)(i / ratio);
        int i0 = (int)sourcePos;
        int i1 = juce::jmin(sourceLen - 1, i0 + 1);
        float frac = sourcePos - i0;

        float s0 = sourceBuffer.getSample(0, i0);
        float s1 = sourceBuffer.getSample(0, i1);
        m_sampleBuffer[i] = s0 + (s1 - s0) * frac;
      }
    }

    // Apply 12-bit Quantization (Authentic S612 DAC/ADC behavior)
    for (int i = 0; i < targetLen; ++i) {
      float &s = m_sampleBuffer[i];
      // 12-bit = 4096 levels (±2048)
      s = std::round(s * 2048.0f) / 2048.0f;
    }

    m_hasSample = true;
    m_writePosition = 0;
    m_recordedLength = targetLen;
  }
}

} // namespace S612
