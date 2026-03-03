#pragma once

#include <vector>
#include <juce_core/juce_core.h>
#include <vector>

namespace S612
{

class S612Sound
{
public:
    S612Sound() = default;
    ~S612Sound() = default;
    
    std::vector<float> m_buffer;
    int m_numSamples = 0;
    double m_recordingSampleRate = 16000.0;
    int m_rootNote = 60;
    
    float m_startPoint = 0.0f;
    float m_endPoint = 1.0f;
    float m_splicePoint = 0.8f;
    int m_scanMode = 0;
    bool m_manualSplice = false;
    float m_lfoSpeed = 0.5f;
    float m_lfoDepth = 0.0f;
    float m_lfoDelay = 0.0f;
    float m_filter = 1.0f;
    float m_decay = 0.5f;
    int m_transpose = 0;
    float m_tune = 0.0f;
};

} // namespace S612
