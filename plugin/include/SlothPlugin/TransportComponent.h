#pragma once

//#include "PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "SlothPlugin/Parameters.h"


namespace audio_plugin {
class TransportComponent  {
public:
  TransportComponent(juce::AudioProcessorValueTreeState& apvts,
                     Parameters& params,
                     juce::ParameterID id)
      : state(TransportState::Stopped),  
        sampleCount(0),                  
        fileName(""),                    
        formatReader(nullptr),
        fileSampleRate(44100.0),
        readerSource(nullptr),
        transport(),
        waveform(),
        formatManager(),
        params(params),
        apvts(apvts),
        paramId(id)
  {}
  ~TransportComponent() = default;

  void playFile();
  void stopFile();
  virtual void setSampleCount(int newSampleCount);
  std::atomic<int>& getSampleCount() { return sampleCount; };
  juce::String fileName;
  double getFileSampleRate() { return fileSampleRate; };
  juce::AudioBuffer<float>& getWaveform() {
    return waveform;
  };
  virtual bool isFileLoaded() const { return readerSource != nullptr; };

  juce::AudioProcessorValueTreeState& apvts;
  Parameters& params;
  juce::ParameterID paramId;

  juce::AudioTransportSource transport;
  static enum TransportState { Stopped, Starting, Stopping, Playing };
  TransportState state;
  juce::AudioFormatManager formatManager;
  juce::AudioFormatReader* formatReader{nullptr};
  std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
  juce::AudioBuffer<float> waveform;
  std::atomic<int> sampleCount;
  double fileSampleRate = 44100.0;

  void transportStateChanged(TransportState newState);


  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportComponent)


};
}  // namespace audio_plugin