#pragma once

namespace S612 {

class S612Scanner {
public:
  enum Mode { ONE_SHOT, LOOPING, ALTERNATING };

  void setStartPoint(float p) { m_startPoint = p; }
  void setEndPoint(float p) { m_endPoint = p; }
  void setMode(Mode m) {
    m_mode = m;
    if (m != ONE_SHOT)
      m_finished = false;
  }
  void setStartPoint(float p, int bufferSize) {
    m_startPoint = p;
    m_startPointSamples = p * (float)bufferSize;
  }
  void setEndPoint(float p, int bufferSize) {
    m_endPoint = p;
    m_endPointSamples = p * (float)bufferSize;
  }
  void setSplicePoint(float p, int bufferSize) {
    m_splicePoint = p;
    m_splicePointSamples = p * (float)bufferSize;
  }

  float getNextSampleIndex(float pitchShift, int bufferSize);
  void reset(float startPointSamples);
  bool isFinished() const { return m_finished; }
  float getPosition() const { return m_position; }

private:
  float m_startPoint = 0.0f;
  float m_endPoint = 1.0f;
  float m_splicePoint = 0.8f;
  float m_startPointSamples = 0.0f;
  float m_endPointSamples = 0.0f;
  float m_splicePointSamples = 0.0f;
  Mode m_mode = ONE_SHOT;

  float m_position = 0.0f;
  bool m_finished = false;
  bool m_pingPongForward = true;
};

} // namespace S612
