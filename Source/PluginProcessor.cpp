#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "S612Engine.h"
#include "S612MidiHandler.h"
#include "S612Parameters.h"
#include "S612VoiceManager.h"

using namespace S612;

namespace S612 {

struct PluginProcessor::Impl {
  S612Engine engine;
  S612MidiHandler midiHandler;
  S612VoiceManager voiceManager;

  Impl() = default;
};

PluginProcessor::PluginProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
              .withInput("Sidechain", juce::AudioChannelSet::stereo(), false)),
      m_apvts(*this, nullptr, "S612Parameters", createParameterLayout()) {
  m_impl = std::make_unique<Impl>();

  // Initialize voice manager with engine
  m_impl->voiceManager.setEngine(&m_impl->engine);
  m_impl->midiHandler.setVoiceManager(&m_impl->voiceManager);
  m_impl->midiHandler.setAPVTS(&m_apvts);
  m_impl->engine.setVoiceManager(&m_impl->voiceManager);

  // Load default sample
  juce::File defaultSample("f:\\S612VSTi\\START.wav");
  if (defaultSample.existsAsFile())
    m_impl->engine.loadSample(defaultSample, 32000.0);
}

PluginProcessor::~PluginProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout
PluginProcessor::createParameterLayout() {
  juce::AudioProcessorValueTreeState::ParameterLayout layout;

  // Recording
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      recLevel, "REC LEVEL", juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
      1.0f));

  // Scanning
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      startPoint, "START/SPLICE",
      juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      endPoint, "END POINT", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
      1.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      splicePoint, "SPLICE", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
      0.8f));
  layout.add(std::make_unique<juce::AudioParameterBool>(manualSplice,
                                                        "MANU. SPLICE", false));
  layout.add(std::make_unique<juce::AudioParameterInt>(
      scanMode, "SCAN MODE", 0, 2, 0)); // 0=OneShot, 1=Looping, 2=Alternating

  // LFO
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      lfoSpeed, "LFO SPEED", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
      0.5f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      lfoDepth, "LFO DEPTH", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
      0.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      lfoDelay, "LFO DELAY", juce::NormalisableRange<float>(0.0f, 5.0f, 0.1f),
      0.0f));

  // Filter & Decay
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      filter, "FILTER", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
      1.0f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      decay, "DECAY", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      outputLevel, "LEVEL", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
      0.8f));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      monitorLevel, "MONITOR",
      juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f));

  // Transpose & Tune
  layout.add(std::make_unique<juce::AudioParameterInt>(transpose, "TRANSPOSE",
                                                       -24, 24, 0));
  layout.add(std::make_unique<juce::AudioParameterFloat>(
      tune, "TUNE", juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
      0.0f));

  // MIDI
  layout.add(std::make_unique<juce::AudioParameterInt>(midiChannel, "MIDI CH",
                                                       0, 9, 0)); // 0=Omni
  layout.add(
      std::make_unique<juce::AudioParameterBool>(monoPoly, "MONO MODE", false));

  // Sample Rate / Frequency
  layout.add(std::make_unique<juce::AudioParameterInt>(
      recFreq, "REC FREQ", 0, 2, 0)); // 0=32k, 1=16k, 2=8k

  layout.add(std::make_unique<juce::AudioParameterBool>(midiLearn, "MIDI LEARN",
                                                        false));

  return layout;
}

void PluginProcessor::prepareToPlay(double sampleRate,
                                    int maximumExpectedSamplesPerBlock) {
  m_impl->engine.prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
  m_impl->voiceManager.prepareToPlay(sampleRate,
                                     maximumExpectedSamplesPerBlock);
  m_impl->midiHandler.prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
}

void PluginProcessor::releaseResources() {
  m_impl->engine.releaseResources();
  m_impl->voiceManager.releaseResources();
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
  // Accept any layout for now
  return true;
}

void PluginProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                   juce::MidiBuffer &midiMessages) {
  // Track MIDI activity
  if (!midiMessages.isEmpty()) {
    m_lastMidiTime = juce::Time::getCurrentTime().getMilliseconds();
  }

  // Get sidechain input (Bus 1) for sampling and monitoring
  juce::AudioBuffer<float> sidechainBuffer;
  auto sidechainBus = getBusBuffer(buffer, true, 1);
  if (sidechainBus.getNumChannels() > 0) {
    sidechainBuffer.makeCopyOf(sidechainBus);
  } else {
    // If no sidechain is connected, use main input as fallback
    sidechainBuffer.makeCopyOf(buffer);
  }

  // Process MIDI
  m_impl->midiHandler.processBlock(midiMessages, buffer.getNumSamples());

  // Process audio with sidechain input
  m_impl->engine.processBlock(buffer, midiMessages, m_apvts, sidechainBuffer);
}

juce::AudioProcessorEditor *PluginProcessor::createEditor() {
  return new PluginEditor(*this);
}

void PluginProcessor::getStateInformation(juce::MemoryBlock &destData) {
  auto state = m_apvts.copyState();
  std::unique_ptr<juce::XmlElement> xml(state.createXml());
  copyXmlToBinary(*xml, destData);
}

void PluginProcessor::setStateInformation(const void *data, int sizeInBytes) {
  std::unique_ptr<juce::XmlElement> xmlState(
      getXmlFromBinary(data, sizeInBytes));
  if (xmlState && xmlState->hasTagName(m_apvts.state.getType())) {
    m_apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
  }
}

float PluginProcessor::getInputLevel() const {
  return m_impl->engine.getInputLevel();
}

bool PluginProcessor::getMidiActivity() const {
  auto now = juce::Time::getCurrentTime().getMilliseconds();
  return (now - m_lastMidiTime) < 100;
}

S612Engine &PluginProcessor::getEngine() { return m_impl->engine; }

void PluginProcessor::setMidiChannel(int ch) {
  if (m_impl)
    m_impl->voiceManager.setMidiChannel(ch);
}

S612MidiHandler &PluginProcessor::getMidiHandler() {
  return m_impl->midiHandler;
}

} // namespace S612

// JUCE Plugin Entry Point
juce::AudioProcessor *createPluginFilter() {
  return new S612::PluginProcessor();
}
