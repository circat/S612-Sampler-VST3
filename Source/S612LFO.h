#pragma once

namespace S612
{

class S612LFO
{
public:
    void prepareToPlay(double sampleRate, int blockSize);
    float getOutput();
    void setSpeed(float speed) { m_speed = speed; }
    void setDepth(float depth) { m_depth = depth; }
    void setDelay(float delay) { m_delay = delay; }
    void setModWheel(float mod) { m_modWheel = mod; }
    void keyOn();
    
private:
    float m_speed = 0.5f;
    float m_depth = 0.0f;
    float m_delay = 0.0f;
    float m_modWheel = 0.0f;
    float m_phase = 0.0f;
    float m_delayCounter = 0.0f;
};

} // namespace S612
