#pragma once

#include "PluginProcessor.h"
#include "torch/torch.h"
#include "torch/script.h"
#include "WaveThumbnail.h"
#include "RoundedCornersEffect.h"
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
  //juce::TextButton playButton{"P"};
  std::unique_ptr<juce::DrawableButton> playButton;
  
  //juce::TextButton playButton2{"P"};
  std::unique_ptr<juce::DrawableButton> playButton2;

  const int buttonWidth = 35;
  const int buttonHeight = 35;

  juce::Colour backgroundColour{45, 40, 47};

private:
  juce::Component full, second, top;
  std::unique_ptr<juce::DrawableButton> loadButton;
  std::unique_ptr<juce::ImageButton> divideButton;
  std::unique_ptr<juce::DrawableButton> saveButton;
  RoundedCornersEffect* rounded = new RoundedCornersEffect(10.0f);
  juce::ComboBox trackSelector{"Select Model"};
  /*std::array<juce::TextButton, 6> tracks = {
      juce::TextButton{"1"}, juce::TextButton{"2"}, juce::TextButton{"3"},
      juce::TextButton{"4"}, juce::TextButton{"5"}, juce::TextButton{"6"}
  };*/
  std::vector<std::unique_ptr<juce::ImageButton>> tracks;
  juce::TooltipWindow tooltipWindow{this, 800};
  
  AudioPluginAudioProcessor& audioProcessor;
  void updateTransportButtons(int sourceIndex, bool isPlaying);

  void mouseEnter(const juce::MouseEvent& event);
  void mouseExit(const juce::MouseEvent& event);
  void mouseDrag(const juce::MouseEvent& event) override;
  void mouseUp(const juce::MouseEvent& event);
  void actionListenerCallback(const juce::String& message) override;
  void setTrackButtons(int length);
  void parameterValueChanged(int idx, float value) override;
  void parameterGestureChanged(int, bool) override {}
  void setSvgButton(juce::String svg,
                    juce::DrawableButton* button);
  void setImageButton(juce::String image,
                      juce::ImageButton* button,
                      bool invertColours);

  WaveThumbnail original;
  WaveThumbnail separation;

  bool isDraggingTrack = false;  // Flag per evitare drag multipli
  int draggedTrackIndex = -1;    // Indice della traccia trascinata
  int selectedTrack = -1;
  std::vector<std::string> names;
  void refreshTrackButtons();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
}  // namespace audio_plugin
