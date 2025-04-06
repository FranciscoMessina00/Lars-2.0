#pragma once

#include "PluginProcessor.h"
#include "torch/torch.h"
#include "torch/script.h"
#include "WaveThumbnail.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace audio_plugin {

class AudioPluginAudioProcessorEditor
    : public juce::AudioProcessorEditor,
      public juce::Timer,
      private juce::AudioProcessorParameter::Listener {
public:
  AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
  ~AudioPluginAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics&) override;
  void resized() override;

  void timerCallback() override;
  juce::TextButton playButton{"P"};
  juce::TextButton playButton2{"P"};

  int buttonWidth = 20;
  int buttonHeight = 15;

private:
  juce::GroupComponent full, second;
  juce::TextButton loadButton{"L"};
  juce::TextButton divideButton{"D"};
  juce::TextButton nextButton{"N"};
  juce::TextButton prevButton{"P"};

  AudioPluginAudioProcessor& audioProcessor;
  void updateTransportButtons(bool status);

  void mouseEnter(const juce::MouseEvent& event);
  void mouseExit(const juce::MouseEvent& event);

  void parameterValueChanged(int, float value) override;
  void parameterGestureChanged(int, bool) override {}
  void mouseDoubleClick(
      const juce::MouseEvent& event);
  WaveThumbnail wave;
  WaveThumbnail snare;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
}  // namespace audio_plugin
