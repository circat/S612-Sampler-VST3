#include "S612Scanner.h"
#include <algorithm>
#include <juce_audio_basics/juce_audio_basics.h>

namespace S612 {

float S612Scanner::getNextSampleIndex(float pitchShift, int bufferSize) {
  if (m_finished || bufferSize <= 0)
    return m_position;

  // Convert points to absolute sample indices
  float startIdx = m_startPointSamples;
  float endIdx = m_endPointSamples;
  float spliceIdx = m_splicePointSamples;

  // Direction handling (if end < start, we are in reverse mode)
  bool isReverse = (endIdx < startIdx);

  // In hardware, the pitch direction is handled by the sampler's clock.
  // If we play a note, 'pitchShift' is positive. The direction is determined
  // by the scanner's relative positions.
  float movement = isReverse ? -pitchShift : pitchShift;

  // Update position
  float nextPos = m_position + (m_pingPongForward ? movement : -movement);

  // ONE SHOT check
  if (m_mode == ONE_SHOT) {
    if (isReverse) {
      if (nextPos <= endIdx) {
        m_finished = true;
        m_position = endIdx;
      } else
        m_position = nextPos;
    } else {
      if (nextPos >= endIdx) {
        m_finished = true;
        m_position = endIdx;
      } else
        m_position = nextPos;
    }
    return m_position;
  }

  // LOOPING or ALTERNATING
  if (isReverse) {
    // REVERSE: start > end. Loop is between endIdx (low) and spliceIdx (high).
    // Ensure spliceIdx makes sense
    float actualSplice = std::max(spliceIdx, endIdx + 1.0f);

    if (m_pingPongForward) {
      if (nextPos <= endIdx) {
        if (m_mode == LOOPING) {
          m_position = actualSplice;
        } else { // ALTERNATING
          m_pingPongForward = false;
          m_position = endIdx;
        }
      } else {
        m_position = nextPos;
      }
    } else { // Moving backwards towards splice (high index)
      if (nextPos >= actualSplice) {
        m_pingPongForward = true;
        m_position = actualSplice;
      } else {
        m_position = nextPos;
      }
    }
  } else {
    // FORWARD: start < end. Loop is between spliceIdx (low) and endIdx (high).
    float actualSplice = std::min(spliceIdx, endIdx - 1.0f);

    if (m_pingPongForward) {
      if (nextPos >= endIdx) {
        if (m_mode == LOOPING) {
          m_position = actualSplice;
        } else { // ALTERNATING
          m_pingPongForward = false;
          m_position = endIdx;
        }
      } else {
        m_position = nextPos;
      }
    } else { // Moving backwards towards splice (low index)
      if (nextPos <= actualSplice) {
        m_pingPongForward = true;
        m_position = actualSplice;
      } else {
        m_position = nextPos;
      }
    }
  }

  return m_position;
}

void S612Scanner::reset(float startPointSamples) {
  m_position = startPointSamples;
  m_finished = false;
  m_pingPongForward = true;
}

} // namespace S612
