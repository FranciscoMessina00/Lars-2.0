#pragma once
#include <juce_core/juce_core.h>  // Per ParameterID, String, ecc.
#include <juce_audio_processors/juce_audio_processors.h>  // Per AudioParameterBool e AudioProcessorValueTreeState

const juce::ParameterID playButtonParamID{"playButton", 1};
const juce::ParameterID playButton2ParamID{"playButton2", 1};

class Parameters {
public:
  Parameters(juce::AudioProcessorValueTreeState& apvts);
  static juce::AudioProcessorValueTreeState::ParameterLayout
  createParameterLayout();

  // void update() noexcept;
  /*void prepareToPlay(double sampleRate) noexcept;
  void reset() noexcept;*/
  bool playButton = false;
  bool playButton2 = false;
  juce::AudioParameterBool* playButtonParam;
  juce::AudioParameterBool* playButton2Param;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Parameters)
};