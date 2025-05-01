#pragma once

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
#include <SlothPlugin/TransportOriginal.h>
#include <SlothPlugin/TransportComponent.h>
#include <SlothPlugin/TransportSeparation.h>
#include <torch/torch.h>
#include <torch/script.h>

using namespace torch::indexing;

//#include <juce_header/JuceHeader.h>

namespace audio_plugin {
struct ErrorBroadcaster : private juce::AsyncUpdater,
                          public juce::ActionBroadcaster {
  // called on audio thread
  void postError(const juce::String& e) {
    lastMessage = e;
    triggerAsyncUpdate();
  }

private:
  juce::String lastMessage;

  // runs on the message thread
  void handleAsyncUpdate() override {
    sendActionMessage(lastMessage);
  }
};

class AudioPluginAudioProcessor : public juce::AudioProcessor {
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

  void saveSeparationIntoFile();

  bool isPlaying{false};
  bool isStopped{true};

  juce::AudioProcessorValueTreeState apvts{*this, nullptr, "Parameters",
                                           Parameters::createParameterLayout()};

  Parameters params;
  juce::String modelName = "mdx23c.pt";
  
  //double getFileSampleRate() { return fileSampleRate; };

  void process();
  
  TransportOriginal transportOriginal;
  TransportSeparation transportSeparation;

  ErrorBroadcaster errorBroadcaster;

  std::vector<std::string> instruments;

private:

  std::vector<juce::File> loadedFiles;
  int currentFileIndex = 0;

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
  //double fileSampleRate = 44100.0;
  double sampleRateRatio = 1.0;
  double coeff = 0.0;
  double newPositionInSeconds = 0;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)

};
}  // namespace audio_plugin
