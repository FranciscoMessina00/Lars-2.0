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
  static double getFileSampleRate();
  static void setFileSampleRate(double sampleRate);
  static unsigned int getFileBitDepth();
  static void setFileBitDepth(unsigned int bitDepth);

  static void deleteTempFiles();

  static std::vector<juce::String> separationNames;
  static std::vector<juce::String> separationPaths;

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
  

  void transportStateChanged(TransportState newState);

private:
  static double fileSampleRate;
  static unsigned int fileBitDepth;


  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportComponent)


};
}  // namespace audio_plugin