/*
  ==============================================================================

    WaveThumbnail.h
    Created: 29 Mar 2025 4:49:19pm
    Author:  giuli

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"
#include <juce_core/juce_core.h>              // Per String, StringArray, ecc.
#include <juce_graphics/juce_graphics.h>      // Per Graphics, Colours, ecc.
#include <juce_gui_basics/juce_gui_basics.h>  // Per Component, MouseEvent, ecc.


//==============================================================================
/*
 */
namespace audio_plugin {
class WaveThumbnail : public juce::Component,
                      public juce::FileDragAndDropTarget {
public:
  WaveThumbnail(AudioPluginAudioProcessor& p, int section);
  ~WaveThumbnail() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  void filesDropped(const juce::StringArray& files, int x, int y);
  bool isInterestedInFileDrag(const juce::StringArray& files);

private:
  std::vector<float> audioPointsL;
  std::vector<float> audioPointsR;

  AudioPluginAudioProcessor& audioProcessor;
  float mapLinear(float value,
                  float inputMin,
                  float inputMax,
                  float outputMin,
                  float outputMax);
  void mouseDown(const juce::MouseEvent& event);
  void mouseDrag(const juce::MouseEvent& event);
  void setPlayHeadPositionFromMouse(int mouseX);
  juce::String formatTime(float seconds, bool isLong);
  float cubicInterpolate(float y0,
                                        float y1,
                                        float y2,
                                        float y3,
                                        float t);

  int section;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveThumbnail)
};
}  // namespace audio_plugin
