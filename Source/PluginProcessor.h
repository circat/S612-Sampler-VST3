#pragma once

#include "S612Parameters.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>

namespace S612 {

class PluginProcessor : public juce::AudioProcessor {
public:
  PluginProcessor();
  ~PluginProcessor() override;

  void prepareToPlay(double, int) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout &) const override;
  bool canAddBus(bool) const override { return true; }
  bool canRemoveBus(bool) const override { return true; }
  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override { return true; }

  const juce::String getName() const override { return "S612 Sampler"; }
  bool acceptsMidi() const override { return true; }
  bool producesMidi() const override { return false; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 2.0; }

  // Program methods - required by JUCE 7
  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return "Default"; }
  void changeProgramName(int, const juce::String &) override {}

  void getStateInformation(juce::MemoryBlock &) override;
  void setStateInformation(const void *, int) override;

  juce::AudioProcessorValueTreeState &getAPVTS() { return m_apvts; }

  // Meter & MIDI status
  float getInputLevel() const;
  bool getMidiActivity() const;
  void resetMidiActivity() { m_lastMidiTime = 0; }
  void setMidiChannel(int ch);

  class S612MidiHandler &getMidiHandler();
  class S612Engine &getEngine();

private:
  static juce::AudioProcessorValueTreeState::ParameterLayout
  createParameterLayout();

  juce::AudioProcessorValueTreeState m_apvts;

  float m_inputLevel = 0.0f;
  uint32_t m_lastMidiTime = 0;

  struct Impl;
  std::unique_ptr<Impl> m_impl;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
};

juce::AudioProcessor *createPluginFilter();

} // namespace S612
