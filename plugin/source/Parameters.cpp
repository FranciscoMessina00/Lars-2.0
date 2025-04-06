/*
  ==============================================================================

    Parameters.cpp
    Created: 29 Mar 2025 12:14:17pm
    Author:  giuli

  ==============================================================================
*/

#include "SlothPlugin/Parameters.h"

template <typename T>
static void castParameter(juce::AudioProcessorValueTreeState& apvts,
                          const juce::ParameterID& id,
                          T& destination) {
  destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));
  jassert(destination);  // parameter does not exist or wrong type
}

Parameters::Parameters(juce::AudioProcessorValueTreeState& apvts) {
  castParameter(apvts, playButtonParamID, playButtonParam);
  castParameter(apvts, playButton2ParamID, playButtonParam2);

}

juce::AudioProcessorValueTreeState::ParameterLayout
Parameters::createParameterLayout() {
  juce::AudioProcessorValueTreeState::ParameterLayout layout;

  layout.add(std::make_unique<juce::AudioParameterBool>(playButtonParamID,
                                                        "Play", false));
  layout.add(std::make_unique<juce::AudioParameterBool>(playButton2ParamID,
                                                        "Play", false));

  return layout;
}

// void Parameters::update() noexcept
//{
//     playButton = playButtonParam->get();
// }