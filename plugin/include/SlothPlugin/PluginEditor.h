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
      public juce::DragAndDropContainer,
      private juce::AudioProcessorParameter::Listener,
      private juce::ActionListener
{
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
  juce::ComboBox trackSelector{"Select Model"};
  /*std::array<juce::TextButton, 6> tracks = {
      juce::TextButton{"1"}, juce::TextButton{"2"}, juce::TextButton{"3"},
      juce::TextButton{"4"}, juce::TextButton{"5"}, juce::TextButton{"6"}
  };*/
  std::vector<std::unique_ptr<juce::TextButton>> tracks;

  
  AudioPluginAudioProcessor& audioProcessor;
  void updateTransportButtons(int sourceIndex, bool isPlaying);

  void mouseEnter(const juce::MouseEvent& event);
  void mouseExit(const juce::MouseEvent& event);
  void mouseDrag(const juce::MouseEvent& event) override;
  void mouseUp(const juce::MouseEvent& event);
  void actionListenerCallback(const juce::String& message) override {
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                           "Plugin Error", message);
  }
  void setTrackButtons(int length);
  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}

  WaveThumbnail original;
  WaveThumbnail separation;

  bool isDraggingTrack = false;  // Flag per evitare drag multipli
  int draggedTrackIndex = -1;    // Indice della traccia trascinata

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
}  // namespace audio_plugin
