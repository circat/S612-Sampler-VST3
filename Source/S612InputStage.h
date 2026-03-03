#pragma once

namespace S612 {

class S612InputStage {
public:
  void prepareToPlay(double sampleRate, int blockSize);
  float processSampleInput(float sample);
  void setRecLevel(float level) { m_recLevel = level; }
  void setTargetSampleRate(double rate) { m_targetRate = rate; }

private:
  float m_recLevel = 1.0f;
  double m_targetRate = 32000.0;
  double m_sampleRate = 44100.0;
  double m_sampleCount = 0.0;
  float m_lastSampleValue = 0.0f;
};

} // namespace S612
