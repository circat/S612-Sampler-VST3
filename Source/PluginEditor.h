#pragma once
#include "UI/S612Fader.h"
#include "UI/S612InputMeter.h"
#include "UI/S612Knob.h"
#include "UI/S612LookAndFeel.h"
#include "UI/S612MidiLed.h"
#include "UI/S612SevenSegDisplay.h"
#include "UI/S612VoiceLEDs.h"
#include <array>
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

namespace S612 {

class PluginProcessor;

class PluginEditor : public juce::AudioProcessorEditor, public juce::Timer {
public:
  explicit PluginEditor(PluginProcessor &);
  ~PluginEditor() override;

  void paint(juce::Graphics &) override;
  void resized() override;
  void timerCallback() override;

private:
  PluginProcessor &m_processor;
  UI::LookAndFeel m_lookAndFeel;

  std::array<bool, 6> m_voiceFlags{};
  std::unique_ptr<UI::VoiceLEDs> m_voiceLeds;
  int m_currentMidiChannel = 0; // 0 = Omni
  juce::Image m_backgroundImage;

  // ── REC SECTION ──────────────────────────────────────────────────────────
  UI::Knob m_recLevelKnob;
  UI::Knob m_monitorKnob;
  juce::TextButton m_newBtn;
  juce::TextButton m_overdubBtn;

  // ── DATA / MIDI buttons ───────────────────────────────────────────────────
  juce::TextButton m_keyTransBtn; // MONO/POLY
  juce::TextButton m_saveBtn, m_loadBtn;
  juce::TextButton m_rateUpBtn;

  // ── SCAN / MODE buttons ───────────────────────────────────────────────────
  juce::TextButton m_oneShotBtn;
  juce::TextButton m_loopingBtn;
  juce::TextButton m_altBtn;
  juce::TextButton m_manualSpliceBtn;

  // ── SCAN faders ───────────────────────────────────────────────────────────
  UI::Fader m_startFader; // START / SPLICE
  UI::Fader m_endFader;   // END POINT

  // ── LFO knobs ─────────────────────────────────────────────────────────────
  UI::Knob m_lfoSpeedKnob;
  UI::Knob m_lfoDepthKnob;
  UI::Knob m_lfoDelayKnob;
  UI::Knob m_tuneKnob;

  // ── OUTPUT knobs ──────────────────────────────────────────────────────────
  UI::Knob m_filterKnob;
  UI::Knob m_decayKnob;
  UI::Knob m_levelKnob;

  // ── Displays ──────────────────────────────────────────────────────────────
  UI::MidiLed m_midiLed;
  UI::InputMeter m_inputMeter;

  // ── APVTS Attachments ─────────────────────────────────────────────────────
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
      m_recLevelAttachment, m_monitorAttachment, m_startAttachment,
      m_endAttachment, m_lfoSpeedAttachment, m_lfoDepthAttachment,
      m_lfoDelayAttachment, m_filterAttachment, m_decayAttachment,
      m_levelAttachment, m_tuneAttachment;

  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
      m_manualSpliceAttachment, m_keyTransAttachment;

  std::unique_ptr<juce::FileChooser> m_chooser;

  // ── Helpers ───────────────────────────────────────────────────────────────
  void adjustMidiChannel(int delta);
  void setScanMode(int mode); // 0=OneShot, 1=Loop, 2=Alt
  void updateScanModeButtons(int mode);
  void updateMidiChannel(int ch);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};

} // namespace S612
