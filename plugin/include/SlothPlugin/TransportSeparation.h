#pragma once

//#include "PluginProcessor.h"
#include "SlothPlugin/Parameters.h"
#include "SlothPlugin/TransportComponent.h"
#include "SlothPlugin/BufferAudioSource.h"

namespace audio_plugin {
class TransportSeparation : public TransportComponent, public juce::DragAndDropContainer {
public:
  TransportSeparation(juce::AudioProcessorValueTreeState& apvts, Parameters& params)
      : TransportComponent(apvts, params, playButton2ParamID) {}
  ~TransportSeparation() = default;

  bool load(int indx);
  std::vector<juce::AudioBuffer<float>>& getSeparatedTracks() {
    return separations;
  };
  void reset();
  bool isFileLoaded() const override { return bufferReader != nullptr; };
  void setSampleCount(int newSampleCount) override;
  std::vector<juce::AudioBuffer<float>> separations;
  std::unique_ptr<BufferAudioSource> bufferReader;


  // Percorsi dei file esportati per ogni traccia
  //std::vector<juce::File> exportedTrackFiles;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportSeparation)

};
}  // namespace audio_plugin