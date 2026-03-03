#include "S612LFO.h"
#include <cmath>

namespace S612
{

void S612LFO::prepareToPlay(double sampleRate, int blockSize)
{
}

float S612LFO::getOutput()
{
    return 1.0f;
}

void S612LFO::keyOn()
{
    m_delayCounter = 0.0f;
}

} // namespace S612
