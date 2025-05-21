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
struct EventBroadcaster : private juce::AsyncUpdater,
                          public juce::ActionBroadcaster {
  // called on audio thread
  void post(const juce::String& e) {
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

struct modelParams {
  double chunk_size;
  int num_overlap;
  int batch_size;
  std::optional<std::string> target_instrument;
  std::vector<std::string> instruments;

  modelParams(double chunkSize,
              int numOverlap,
              int batchSize,
              std::optional<std::string> targetInst = std::nullopt,
              std::vector<std::string> instList = {})
      : chunk_size(chunkSize),
        num_overlap(numOverlap),
        batch_size(batchSize),
        target_instrument(targetInst),
        instruments(instList) {}

};

class AudioPluginAudioProcessor
    : public juce::AudioProcessor,
      public std::enable_shared_from_this<AudioPluginAudioProcessor> {
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

  EventBroadcaster eventBroadcaster;

  modelParams* chosen = nullptr;
  torch::Tensor outputTensor;

  void handleInferenceResult(torch::Tensor result) {
    outputTensor = std::move(result);
    if(!doubleSeparation){
      transportSeparation.separations =
          tensorToAudio(outputTensor);  // Converti il tensore in AudioBuffer
      saveSeparationIntoFile();         // Salva il risultato in un file WAV
    } else {
      modelName = "mdx23c.pt";
      std::vector<juce::AudioBuffer<float>> separatedTracks =
          tensorToAudio(outputTensor);  // Converti il tensore in AudioBuffer
      drumBuffer = separatedTracks[2];
      juce::Logger::writeToLog("First part done, separating drums");
      process();
      return;
    }
    juce::Logger::writeToLog("Separation completed and saved.");
<<<<<<< Updated upstream
    //sendChangeMessage();
  }
  bool doubleSeparation = false;
=======
    // sendChangeMessage();
  };

  void saveSeparatedTracks(const juce::File& selectedFolder);

>>>>>>> Stashed changes
private:
  modelParams mdx_1;
  modelParams mdx_2;
  modelParams mdx_3;
  std::map<std::string, modelParams*> mdx_map = {
      {"mdx23c.pt", &mdx_1},
      {"mdx23c_capstone.pt", &mdx_2},
      {"mdx23c_inst_sep.pt", &mdx_3}
  };
  
  juce::AudioBuffer<float> drumBuffer;

  std::vector<juce::File> loadedFiles;
  int currentFileIndex = 0;

  torch::Tensor audioToTensor(const juce::AudioBuffer<float>& buffer);
  std::vector<juce::AudioBuffer<float>> tensorToAudio(torch::Tensor tensor);
  /*void demix_track(
      double chunk_size,
      int num_overlap,
      int batch_size,
      const std::optional<std::string>& target_instrument,
      const std::vector<std::string>& instruments,
      torch::jit::Module& my_model,
      torch::Tensor mix,
      torch::Device device);*/
  juce::ThreadPool threadPool;
  
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

class SeparateThread : public juce::ThreadPoolJob {
public:
  SeparateThread(AudioPluginAudioProcessor* ownerIn,
                 double chunk_size,
                 int num_overlap,
                 int batch_size,
                 const std::optional<std::string> target_instrument,
                 const std::vector<std::string> instruments,
                 torch::jit::Module my_model,
                 torch::Tensor mix,
                 torch::Device device)
      : juce::ThreadPoolJob("SeparationJobThread"),
        owner_(ownerIn),
        chunk_size(chunk_size),
        num_overlap(num_overlap),
        batch_size(batch_size),
        target_instrument(target_instrument),
        instruments(instruments),
        my_model(my_model),
        mix(mix),
        device(device) {}

  JobStatus runJob() override {
    juce::Logger::writeToLog("Inside Thread");
    JobStatus status = demix_track();

    if (owner_) {
      // move the result into the lambda so it outlives this scope
      auto finalResult = std::move(outputTensor);
      juce::MessageManager::callAsync(
          [owner = owner_, finalResult = std::move(finalResult)]() mutable {
            if (!owner->doubleSeparation) {
              owner->eventBroadcaster.post("Demix finished");
            }
            owner->handleInferenceResult(std::move(finalResult));
          });
    }
    return status;
  }

private:
  JobStatus demix_track();  // your long-running functions
  AudioPluginAudioProcessor* owner_;
  torch::Tensor outputTensor;
  double chunk_size;
  int num_overlap;
  int batch_size;
  const std::optional<std::string> target_instrument;
  const std::vector<std::string> instruments;
  torch::jit::Module my_model;
  torch::Tensor mix;
  torch::Device device;
};
}  // namespace audio_plugin
