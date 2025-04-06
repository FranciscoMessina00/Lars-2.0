#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "torch/torch.h"
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <SlothPlugin/Parameters.h>

//#include <juce_header/JuceHeader.h>

namespace audio_plugin {
class AudioPluginAudioProcessor : public juce::AudioProcessor,
                                  public juce::ChangeBroadcaster {
public:
  AudioPluginAudioProcessor();
  ~AudioPluginAudioProcessor() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& newName) override;

  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  void loadFile();
  void loadFile(const juce::String& path);
  void playFile(int section);
  void stopFile(int section);

  juce::AudioBuffer<float>& getWaveform(int section);
  bool isFileLoaded() const { return readerSource != nullptr; };
  bool isPlaying{false};
  bool isStopped{true};

  juce::AudioProcessorValueTreeState apvts{*this, nullptr, "Parameters",
                                           Parameters::createParameterLayout()};

  Parameters params;
  std::atomic<int>& getSampleCount() { return sampleCount; };

  juce::String fileName = "";
  juce::String fileName2 = "";
  

  void setSampleCount(int newSampleCount);
  double getFileSampleRate() { return fileSampleRate; };

  void process();

  void nextFile();
  void previousFile();

  void toggleTransport(int activeSection);

  juce::AudioTransportSource transport;
  juce::AudioTransportSource transport2;

private:
  enum TransportState { Stopped, Starting, Stopping, Playing };
  TransportState state;
  juce::AudioFormatManager formatManager;
  juce::AudioFormatReader* formatReader{nullptr};
  std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
  std::unique_ptr<juce::AudioFormatReaderSource> readerSource2;
  juce::AudioBuffer<float> waveform;
  juce::AudioBuffer<float> waveform2;
  std::atomic<int> sampleCount = 0;

  std::vector<juce::File> loadedFiles;
  int currentFileIndex = 0;

  void transportStateChanged(TransportState newState);
  void loadFileAtIndex(int index, int section);
  double fileSampleRate = 44100.0;
  double sampleRateRatio = 1.0;
  double coeff = 0.0;
  double newPositionInSeconds = 0;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};
}  // namespace audio_plugin
