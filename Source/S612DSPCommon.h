#pragma once

namespace S612
{

class S612BitQuantizer
{
public:
    float process(float input);
};

class S612SampleRateCalc
{
public:
    static double getSampleRateFromMidiNote(int midiNote);
    static double getMaxSamplingTime(int midiNote);
};

class S612PitchShifter
{
public:
    void setRootNote(int note);
    void setPlayedNote(int note);
    float process(float input);
    
private:
    int m_rootNote = 60;
    int m_playedNote = 60;
    float m_rate = 1.0f;
};

} // namespace S612
