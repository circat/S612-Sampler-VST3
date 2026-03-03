#include "S612DSPCommon.h"
#include <cmath>
#include <algorithm>

namespace S612
{

float S612BitQuantizer::process(float input)
{
    const float steps = 4096.0f;
    return std::round(input * steps) / steps;
}

double S612SampleRateCalc::getSampleRateFromMidiNote(int midiNote)
{
    midiNote = std::max(36, std::min(72, midiNote));
    double normalized = (midiNote - 36.0) / 36.0;
    return 4000.0 * std::pow(8.0, normalized);
}

double S612SampleRateCalc::getMaxSamplingTime(int midiNote)
{
    return 32000.0 / getSampleRateFromMidiNote(midiNote);
}

void S612PitchShifter::setRootNote(int note)
{
    m_rootNote = note;
    m_rate = std::pow(2.0, (m_playedNote - m_rootNote) / 12.0);
}

void S612PitchShifter::setPlayedNote(int note)
{
    m_playedNote = note;
    m_rate = std::pow(2.0, (m_playedNote - m_rootNote) / 12.0);
}

float S612PitchShifter::process(float input)
{
    return input;
}

} // namespace S612
