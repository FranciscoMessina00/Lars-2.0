#pragma once

#include <Windows.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <SlothPlugin/Parameters.h>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <optional> // C++17
#include <memory>
#include <stdexcept>  // For std::runtime_error
#include <SlothPlugin/BufferAudioSource.h>
#include <torch/torch.h>
#include <torch/script.h>

using namespace torch::indexing;

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
  void playFile();
  void playFile2();
  void stopFile();
  void stopFile2();
  void saveSeparationIntoFile();

  juce::AudioBuffer<float>& getWaveform() {
    return waveform;
  };

  juce::AudioBuffer<float>& getWaveform2() { return waveform2; };
  //juce::AudioBuffer<float>& getWaveform(int section);
  bool isFileLoaded() const { return readerSource != nullptr; };
  bool isPlaying{false};
  bool isStopped{true};

  juce::AudioProcessorValueTreeState apvts{*this, nullptr, "Parameters",
                                           Parameters::createParameterLayout()};

  Parameters params;
  std::atomic<int>& getSampleCount() { return sampleCount; };
  std::atomic<int>& getSampleCount2() { return sampleCount2; };

  juce::String fileName = "";
  juce::String* fileName2 = nullptr;
  

  void setSampleCount(int newSampleCount);
  void setSampleCount2(int newSampleCount);
  double getFileSampleRate() { return fileSampleRate; };

  void process();
  

  //void nextFile();
  //void previousFile();

  //void toggleTransport(int activeSection);

  juce::AudioTransportSource transport;
  juce::AudioTransportSource transport2;

  void loadBuffer(int indx, double sampleRate);
  std::vector<juce::AudioBuffer<float>>& getSeparatedTracks() { return trackBuffers; };

private:
  enum TransportState { Stopped, Starting, Stopping, Playing };
  TransportState state;
  TransportState state2;
  juce::AudioFormatManager formatManager;
  juce::AudioFormatReader* formatReader{nullptr};
  std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
  std::unique_ptr<juce::AudioFormatReaderSource> readerSource2;
  juce::AudioBuffer<float> waveform;
  juce::AudioBuffer<float> waveform2;
  std::vector<juce::AudioBuffer<float>> separations;
  std::atomic<int> sampleCount = 0;
  std::atomic<int> sampleCount2 = 0;

  std::vector<juce::File> loadedFiles;
  int currentFileIndex = 0;

  void transportStateChanged(TransportState newState);
  void transportStateChanged2(TransportState newState);
  //void loadFileAtIndex(int index, int section);
  torch::Tensor audioToTensor(const juce::AudioBuffer<float>& buffer);
  std::vector<juce::AudioBuffer<float>> tensorToAudio(torch::Tensor tensor);
  torch::Tensor demix_track(
      double chunk_size,
      int num_overlap,
      int batch_size,
      const std::optional<std::string>& target_instrument,
      const std::vector<std::string>& instruments,
      torch::jit::Module& my_model,
      torch::Tensor mix,
      torch::Device device
  );
  bool saveAudioBufferToWav(juce::AudioBuffer<float>& bufferToSave,
                            juce::File& outputFile,
                            double sampleRate,
                            unsigned int bitDepth
  );
  double fileSampleRate = 44100.0;
  double sampleRateRatio = 1.0;
  double coeff = 0.0;
  double newPositionInSeconds = 0;

  
  std::unique_ptr<BufferAudioSource> bufferReader;

  void setupLibTorch();

  std::vector<juce::AudioBuffer<float>> trackBuffers;
  std::vector<juce::String> separationNames;


  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)

};
}  // namespace audio_plugin
