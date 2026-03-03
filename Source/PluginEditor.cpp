#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "S612Engine.h"
#include "S612MidiHandler.h"
#include "S612PanelBg.h"
#include "S612Parameters.h"

namespace S612 {

PluginEditor::PluginEditor(PluginProcessor &p)
    : AudioProcessorEditor(&p), m_processor(p) {
  setSize(1000, 240); // 19" Rack Proportions
  setLookAndFeel(&m_lookAndFeel);

  // Load realistic background image
  juce::File bgFile("f:\\S612VSTi\\backpanel_clean2.png");
  if (bgFile.existsAsFile())
    m_backgroundImage = juce::ImageFileFormat::loadFrom(bgFile);

  auto &apvts = m_processor.getAPVTS();

  // ─── Helper lambdas ────────────────────────────────────────────────────────
  auto addKnob = [&](UI::Knob &knob, const juce::String &name,
                     const char *paramID) {
    addAndMakeVisible(knob);
    knob.setName(name);
    knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    return std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID,
                                                              knob);
  };

  auto addFader = [&](UI::Fader &fader, const juce::String &name,
                      const char *paramID) {
    addAndMakeVisible(fader);
    fader.setName(name);
    fader.setSliderStyle(juce::Slider::LinearHorizontal);
    fader.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    return std::make_unique<
        juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID,
                                                              fader);
  };

  auto addBtn = [&](juce::TextButton &btn, const juce::String &text,
                    const juce::String &name) {
    addAndMakeVisible(btn);
    btn.setButtonText(text);
    btn.setName(name);
  };

  // ─── REC SECTION ───────────────────────────────────────────────────────────
  m_recLevelAttachment = addKnob(m_recLevelKnob, "REC LEVEL", recLevel);
  m_monitorAttachment = addKnob(m_monitorKnob, "MONITOR", monitorLevel);

  addBtn(m_newBtn, "NEW", "REC_MODE_NEW");
  m_newBtn.setClickingTogglesState(true);
  m_newBtn.onClick = [this]() {
    auto &eng = m_processor.getEngine();
    if (m_newBtn.getToggleState()) {
      eng.setRecordState(S612Engine::RecordState::STANDBY);
      eng.setHasSample(false);
      m_overdubBtn.setToggleState(false, juce::dontSendNotification);
    } else {
      eng.setRecordState(S612Engine::RecordState::IDLE);
    }
  };

  addBtn(m_overdubBtn, "OVER\nDUB", "REC_MODE_DUB");
  m_overdubBtn.setClickingTogglesState(true);
  m_overdubBtn.onClick = [this]() {
    auto &eng = m_processor.getEngine();
    if (m_overdubBtn.getToggleState()) {
      eng.setRecordState(S612Engine::RecordState::OVERDUBBING);
      m_newBtn.setToggleState(false, juce::dontSendNotification);
    } else {
      eng.setRecordState(S612Engine::RecordState::IDLE);
    }
  };

  addBtn(m_saveBtn, "SAVE", "DATA_SAVE");
  addBtn(m_loadBtn, "LOAD", "DATA_LOAD");

  // ─── DATA / MIDI SECTION ──────────────────────────────────────────────────
  addBtn(m_keyTransBtn, "MONO\n/POLY", "DATA_MONO");
  m_keyTransBtn.setClickingTogglesState(true);

  // ─── SCAN / MODE BUTTONS ──────────────────────────────────────────────────
  addBtn(m_oneShotBtn, "ONE\nSHOT", "MODE_ONE");
  addBtn(m_loopingBtn, "LOOPING", "MODE_LOOP");
  addBtn(m_altBtn, "ALTER\nNATING", "MODE_ALT");
  addBtn(m_manualSpliceBtn, "MANUAL\nSPLICE", "MODE_MANU");

  m_oneShotBtn.setRadioGroupId(1);
  m_loopingBtn.setRadioGroupId(1);
  m_altBtn.setRadioGroupId(1);

  m_oneShotBtn.setClickingTogglesState(true);
  m_loopingBtn.setClickingTogglesState(true);
  m_altBtn.setClickingTogglesState(true);
  m_oneShotBtn.setToggleState(true, juce::dontSendNotification);

  m_oneShotBtn.onClick = [this, &apvts]() {
    setScanMode(0);
    if (auto *p = apvts.getParameter(scanMode))
      p->setValueNotifyingHost(p->convertTo0to1(0));
  };
  m_loopingBtn.onClick = [this, &apvts]() {
    setScanMode(1);
    if (auto *p = apvts.getParameter(scanMode))
      p->setValueNotifyingHost(p->convertTo0to1(1));
  };
  m_altBtn.onClick = [this, &apvts]() {
    setScanMode(2);
    if (auto *p = apvts.getParameter(scanMode))
      p->setValueNotifyingHost(p->convertTo0to1(2));
  };

  // ─── START/SPLICE + END FADERS ────────────────────────────────────────────
  m_startAttachment = addFader(m_startFader, "START/SPLICE", startPoint);
  m_endAttachment = addFader(m_endFader, "END POINT", endPoint);

  // ─── LFO KNOBS ───────────────────────────────────────────────────────────
  m_lfoSpeedAttachment = addKnob(m_lfoSpeedKnob, "LFO SPEED", lfoSpeed);
  m_lfoDepthAttachment = addKnob(m_lfoDepthKnob, "LFO DEPTH", lfoDepth);
  m_lfoDelayAttachment = addKnob(m_lfoDelayKnob, "LFO DELAY", lfoDelay);
  m_tuneAttachment = addKnob(m_tuneKnob, "TUNE", tune);

  // ─── OUTPUT KNOBS ────────────────────────────────────────────────────────
  m_filterAttachment = addKnob(m_filterKnob, "FILTER", filter);
  m_decayAttachment = addKnob(m_decayKnob, "DECAY", decay);
  m_levelAttachment = addKnob(m_levelKnob, "LEVEL", outputLevel);

  // ─── DISPLAYS ────────────────────────────────────────────────────────────
  addAndMakeVisible(m_inputMeter);

  auto addMidiLearn = [this](auto &s, const juce::String &paramID) {
    s.onMouseDown = [this, paramID](const juce::MouseEvent &e) {
      if (e.mods.isRightButtonDown()) {
        juce::PopupMenu m;
        m.addItem(1, "Learn MIDI CC");
        m.addItem(2, "Clear MIDI Mapping");
        m.showMenuAsync(juce::PopupMenu::Options(),
                        [this, paramID](int result) {
                          if (result == 1)
                            m_processor.getMidiHandler().startLearning(paramID);
                          else if (result == 2)
                            m_processor.getMidiHandler().forgetMapping(paramID);
                        });
      }
    };
  };

  // Attach MIDI Learn to sliders
  addMidiLearn(m_startFader, startPoint);
  addMidiLearn(m_endFader, endPoint);
  addMidiLearn(m_lfoSpeedKnob, lfoSpeed);
  addMidiLearn(m_lfoDepthKnob, lfoDepth);
  addMidiLearn(m_lfoDelayKnob, lfoDelay);
  addMidiLearn(m_tuneKnob, tune);
  addMidiLearn(m_filterKnob, filter);
  addMidiLearn(m_decayKnob, decay);
  addMidiLearn(m_levelKnob, outputLevel);
  addMidiLearn(m_recLevelKnob, recLevel);
  addMidiLearn(m_monitorKnob, monitorLevel);

  // ─── REC FREQ Toggle button ─────────────────────────────────────────────
  addBtn(m_rateUpBtn, "RATE", "RATE_TOGGLE");

  m_rateUpBtn.onClick = [this, &apvts]() {
    if (auto *p = apvts.getParameter(recFreq)) {
      int cur = (int)(p->getValue() * 2.01f);
      int next = (cur + 1) % 3;
      p->setValueNotifyingHost(p->convertTo0to1(next));
    }
  };

  m_manualSpliceAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          apvts, manualSplice, m_manualSpliceBtn);
  m_keyTransAttachment =
      std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
          apvts, monoPoly, m_keyTransBtn);

  m_saveBtn.onClick = [this] {
    m_chooser = std::make_unique<juce::FileChooser>(
        "Save S612 Sample",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory), "*.wav");
    m_chooser->launchAsync(juce::FileBrowserComponent::saveMode |
                               juce::FileBrowserComponent::canSelectFiles,
                           [this](const juce::FileChooser &fc) {
                             auto file = fc.getResult();
                             if (file != juce::File())
                               m_processor.getEngine().saveSample(file);
                           });
  };

  m_loadBtn.onClick = [this] {
    m_chooser = std::make_unique<juce::FileChooser>(
        "Load S612 Sample",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory), "*.wav");
    m_chooser->launchAsync(
        juce::FileBrowserComponent::openMode |
            juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser &fc) {
          auto file = fc.getResult();
          if (file != juce::File()) {
            double targetRate = 32000.0;
            if (auto *p =
                    m_processor.getAPVTS().getRawParameterValue(recFreq)) {
              int idx = (int)(p->load() * 1.01f);
              if (idx == 1)
                targetRate = 16000.0;
              else if (idx == 2)
                targetRate = 8000.0;
            }
            m_processor.getEngine().loadSample(file, targetRate);
          }
        });
  };

  startTimerHz(30);
}

PluginEditor::~PluginEditor() {
  stopTimer();
  setLookAndFeel(nullptr);
}

void PluginEditor::adjustMidiChannel(int delta) {
  m_currentMidiChannel = juce::jlimit(0, 9, m_currentMidiChannel + delta);
  m_processor.setMidiChannel(m_currentMidiChannel);
}

void PluginEditor::setScanMode(int mode) {
  m_oneShotBtn.setToggleState(mode == 0, juce::dontSendNotification);
  m_loopingBtn.setToggleState(mode == 1, juce::dontSendNotification);
  m_altBtn.setToggleState(mode == 2, juce::dontSendNotification);
}

void PluginEditor::timerCallback() {
  m_inputMeter.setLevel(m_processor.getInputLevel());

  // Update Manual Splice Button Color based on Engine state
  bool msActive = m_processor.getEngine().isManualSpliceActive();
  m_manualSpliceBtn.setColour(juce::TextButton::buttonOnColourId,
                              msActive ? juce::Colours::red
                                       : juce::Colours::black);
  m_manualSpliceBtn.setToggleState(msActive, juce::dontSendNotification);

  repaint(); // For VU and Rate LEDs
}

void PluginEditor::updateScanModeButtons(int mode) { setScanMode(mode); }
void PluginEditor::updateMidiChannel(int ch) { m_currentMidiChannel = ch; }

void PluginEditor::paint(juce::Graphics &g) {
  const int W = getWidth();
  const int H = getHeight();

  if (m_backgroundImage.isValid()) {
    g.drawImageWithin(m_backgroundImage, 0, 0, W, H,
                      juce::RectanglePlacement::fillDestination);
  } else {
    // Fallback if image fails
    juce::Colour topColor(40, 42, 45);
    juce::Colour botColor(15, 16, 18);
    juce::ColourGradient grad(topColor, 0, 0, botColor, 0, (float)H, false);
    g.setGradientFill(grad);
    g.fillAll();
  }

  // Grain/Noise overlay for a bit more 'grit'
  juce::Random r;
  for (int i = 0; i < 300; ++i) {
    g.setColour(juce::Colours::white.withAlpha(0.01f));
    g.fillRect(r.nextInt(W), r.nextInt(H), 1, 1);
  }

  const int earW = 40;
  g.setColour(juce::Colours::black.withAlpha(0.4f));
  g.drawVerticalLine(earW, 0, (float)H);
  g.drawVerticalLine(W - earW, 0, (float)H);

  auto divider = [&](int x) {
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.drawVerticalLine(x, 15, (float)H - 15);
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawVerticalLine(x + 1, 15, (float)H - 15);
  };
  divider(220);
  divider(460);
  divider(660);

  auto label = [&](const juce::String &text, int x, int y, int w, int h) {
    g.setColour(juce::Colours::whitesmoke.withAlpha(0.8f));
    g.setFont(juce::Font("Arial", 10.0f, juce::Font::bold));
    g.drawText(text, x, y, w, h, juce::Justification::centred);
  };

  label("SAMPLE RATE", 355, 125, 100, 12);

  // ── Rate LEDs ────────────────────────────────────────────────────────────
  auto drawRateLED = [&](int x, int y, bool active, const char *txt) {
    // Outer glow if active
    if (active) {
      g.setColour(juce::Colours::red.withAlpha(0.3f));
      g.fillEllipse(x - 2, y - 2, 10, 10);
    }

    g.setColour(active ? juce::Colours::red : juce::Colour(30, 0, 0));
    g.fillEllipse(x, y, 6, 6);

    g.setColour(active ? juce::Colours::white : juce::Colour(100, 100, 100));
    g.setFont(juce::Font("Arial", 9.0f, juce::Font::bold));
    g.drawText(txt, x + 15, y - 2, 35, 10, juce::Justification::centredLeft);
  };

  int rateIdx = 0;
  if (auto *p = m_processor.getAPVTS().getRawParameterValue(recFreq))
    rateIdx = (int)(p->load() * 1.01f);

  drawRateLED(415, 35, rateIdx == 0, "32k");
  drawRateLED(415, 50, rateIdx == 1, "16k");
  drawRateLED(415, 65, rateIdx == 2, "8k");

  label("START / SPLICE", 475, 20, 170, 12);
  label("END POINT", 475, 75, 170, 12);
}

void PluginEditor::resized() {
  // Left: REC Section
  m_recLevelKnob.setBounds(55, 65, 45, 65);
  m_monitorKnob.setBounds(105, 65, 45, 65);

  m_saveBtn.setBounds(55, 140, 45, 25);
  m_loadBtn.setBounds(105, 140, 45, 25);
  m_newBtn.setBounds(55, 175, 75, 22);
  m_overdubBtn.setBounds(55, 202, 75, 22);

  // Middle: VU / RATE / MIDI
  m_inputMeter.setBounds(235, 25, 110, 60);
  m_rateUpBtn.setBounds(360, 35, 45, 40); // The Rate Toggle Switch

  int btnY = 150;
  m_keyTransBtn.setBounds(235, btnY + 30, 75, 25);
  m_oneShotBtn.setBounds(315, btnY + 30, 45, 25);
  m_loopingBtn.setBounds(365, btnY + 30, 45, 25);
  m_altBtn.setBounds(415, btnY + 30, 45, 25);

  // Right Top: Faders
  m_startFader.setBounds(500, 40, 160, 30);
  m_endFader.setBounds(500, 90, 160, 30);
  m_manualSpliceBtn.setBounds(540, 145, 80, 25);

  // Far Right: LFO / Filter
  const int kX = 690, kY1 = 40, kY2 = 140, kSz = 60, kStep = 75, kH = 75;
  m_lfoSpeedKnob.setBounds(kX, kY1, kSz, kH);
  m_lfoDepthKnob.setBounds(kX + kStep, kY1, kSz, kH);
  m_lfoDelayKnob.setBounds(kX + kStep * 2, kY1, kSz, kH);
  m_tuneKnob.setBounds(kX + kStep * 3, kY1, kSz, kH);

  m_filterKnob.setBounds(kX, kY2, kSz, kH);
  m_decayKnob.setBounds(kX + kStep, kY2, kSz, kH);
  m_levelKnob.setBounds(kX + kStep * 2, kY2, kSz, kH);
}

} // namespace S612
