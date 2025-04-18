#pragma once

//#include "PluginProcessor.h"
#include "SlothPlugin/Parameters.h"
#include "SlothPlugin/TransportComponent.h"
#include "SlothPlugin/BufferAudioSource.h"

namespace audio_plugin {
class TransportSeparation : public TransportComponent {
public:
  TransportSeparation(juce::AudioProcessorValueTreeState& apvts, Parameters& params)
      : TransportComponent(apvts, params, playButton2ParamID) {}
  ~TransportSeparation() = default;

  void load(int indx, double sampleRate);
  std::vector<juce::AudioBuffer<float>>& getSeparatedTracks() {
    return trackBuffers;
  };
  bool isFileLoaded() const override { return bufferReader != nullptr; };
  void setSampleCount(int newSampleCount) override;
  std::vector<juce::AudioBuffer<float>> separations;
  std::unique_ptr<BufferAudioSource> bufferReader;
  std::vector<juce::AudioBuffer<float>> trackBuffers;
  std::vector<juce::String> separationNames;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportSeparation)

};
}  // namespace audio_plugin