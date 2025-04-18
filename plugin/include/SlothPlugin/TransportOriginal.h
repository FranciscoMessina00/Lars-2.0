#pragma once

//#include "PluginProcessor.h"
#include "SlothPlugin/Parameters.h"
#include "SlothPlugin/TransportComponent.h"


namespace audio_plugin {
class TransportOriginal : public TransportComponent  {
public:
  TransportOriginal(juce::AudioProcessorValueTreeState& apvts,
                    Parameters& params)
      : TransportComponent(apvts, params, playButtonParamID) {}
  ~TransportOriginal() = default;

  void load();
  void load(const juce::String& path);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportOriginal)


};
}  // namespace audio_plugin