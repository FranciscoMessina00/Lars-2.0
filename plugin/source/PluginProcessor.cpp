#include "SlothPlugin/PluginProcessor.h"
#include "SlothPlugin/PluginEditor.h"
#include "SlothPlugin/Parameters.h"


namespace audio_plugin {
/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
      params(apvts),
      transportOriginal(apvts, params),
      transportSeparation(apvts, params)
#endif
{
  transportOriginal.formatManager.registerBasicFormats();
  transportSeparation.state = TransportComponent::TransportState::Stopped;
  transportOriginal.state = TransportComponent::TransportState::Stopped;
  
  setupLibTorch();

}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {
  transportOriginal.formatReader = nullptr;
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const {
  return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram() {
  return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram(int index) {}

const juce::String AudioPluginAudioProcessor::getProgramName(int index) {
  return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index,
                                                  const juce::String& newName) {
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate,
                                              int samplesPerBlock) {
  transportOriginal.transport.prepareToPlay(samplesPerBlock, sampleRate); // Mettere anche l'altro transport
  transportSeparation.transport.prepareToPlay(samplesPerBlock, sampleRate);

  // coeff = 1.0 - std::exp(-1.0 / (0.1 * sampleRate));
}

void AudioPluginAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AudioPluginAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  if (transportOriginal.readerSource.get() == nullptr) {
    buffer.clear();
    return;
  }


  double systemSampleRate = getSampleRate();
  sampleRateRatio = fileSampleRate / systemSampleRate;

  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
    buffer.clear(i, 0, buffer.getNumSamples());
  }
  
  juce::AudioSourceChannelInfo channelInfo(&buffer, 0, buffer.getNumSamples());
  
  if (params.playButton) {
    transportOriginal.sampleCount += static_cast<int>(buffer.getNumSamples() * sampleRateRatio);
    transportOriginal.transport.getNextAudioBlock(channelInfo);
  }
    
  if (params.playButton2){
    transportSeparation.sampleCount += static_cast<int>(buffer.getNumSamples() * sampleRateRatio);
    transportSeparation.transport.getNextAudioBlock(channelInfo);
  }
    

  /*for (int channel = 0; channel < totalNumInputChannels; ++channel)
  {
      auto* channelData = buffer.getWritePointer (channel);


  }*/
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor() {
  return new AudioPluginAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void AudioPluginAudioProcessor::setStateInformation(const void* data,
                                                    int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
}

void AudioPluginAudioProcessor::process() {
  
  transportSeparation.separations.clear();  // Clear the separations vector
  torch::Tensor audioTensor = audioToTensor(transportOriginal.waveform);

  torch::jit::script::Module module;
  try {
    // Deserialize the ScriptModule from a file using torch::jit::load().
    // Trova il percorso del plugin VST3 o Standalone
    juce::File pluginFile =
        juce::File::getSpecialLocation(juce::File::currentExecutableFile);
    // Se è un VST3, il file trovato potrebbe essere il .vst3 stesso
    // Se è Standalone, è l'eseguibile .exe
    // Vogliamo la cartella che lo contiene
    juce::File modelFile = pluginFile.getParentDirectory().getChildFile(
        "mdx23c.pt");  // Nome del file modello

    juce::String modelPath = modelFile.getFullPathName();
    module = torch::jit::load(modelPath.toStdString());

  } catch (const c10::Error& e) {
    /*juce::Logger::writeToLog("Error loading the model: " +
                             juce::String(e.what()));*/
    errorBroadcaster.postError("Error loading the model:");
    return;
  }

  juce::Logger::writeToLog("ok\n");

  int num_samples = audioTensor.size(1);  // Get the number of samples

  // Generate a random tensor with values in the range [-1, 1]
  // torch::Tensor audio_tensor = torch::rand({ num_samples }, torch::kFloat32)
  // * 2 - 1; Create a 2-channel (stereo) audio tensor
  torch::Tensor output;
  try {
    instruments = {"kick", "snare", "toms", "hh",   "ride",  "crash"};
    output = demix_track(130560, 4, 1, std::nullopt, instruments, module,
                              audioTensor, torch::kCPU);
    // If the output is a tensor, convert it and use as needed
    auto sizes = output.sizes();
    // Create a string representation of the sizes
    juce::String sizeString = "Output Tensor sizes: [";
    for (size_t i = 0; i < sizes.size(); ++i) {
      sizeString += juce::String(sizes[i]);
      if (i < sizes.size() - 1)
        sizeString += ", ";
    }
    sizeString += "]";

    // Log the size information
    juce::Logger::writeToLog(
        sizeString +
        " Type of output: " + juce::String(c10::toString(output.scalar_type())));

  } catch (std::exception e) {
    errorBroadcaster.postError("Error in the demixing part");
    return;
  }

  transportSeparation.separations =
      tensorToAudio(output);  // Converti il tensore in AudioBuffer
  juce::Logger::writeToLog("Converted to AudioBuffer");
  saveSeparationIntoFile();        // Salva il risultato in un file WAV
}

torch::Tensor AudioPluginAudioProcessor::audioToTensor(const juce::AudioBuffer<float>& buffer) {
  int numChannels = buffer.getNumChannels();
  int numSamples = buffer.getNumSamples();

  // Create a vector of tensors, one per channel
  std::vector<torch::Tensor> channelTensors;
  for (int ch = 0; ch < numChannels; ++ch) {
    // Create a tensor from the channel's data.
    // Use from_blob and then clone to ensure the tensor owns its data.
    const float* channelData = buffer.getReadPointer(ch);
    torch::Tensor t = torch::from_blob((void*)channelData, {numSamples}, torch::kFloat).clone();
    channelTensors.push_back(t);
  }

  // Stack the channels to form a tensor with shape [Nchannels, Nsamples]
  torch::Tensor audioTensor = torch::stack(channelTensors);

  // Optionally, add a batch dimension if needed: [1, Nchannels, Nsamples]
  // audioTensor = audioTensor.unsqueeze(0);

  return audioTensor;
}


// Assuming this function is part of your AudioPluginAudioProcessor class
std::vector<juce::AudioBuffer<float>> AudioPluginAudioProcessor::tensorToAudio(
    torch::Tensor tensor)  // Pass by value is often okay for Tensors if you
                           // modify it (like .contiguous())
{
    // Initialize empty output

  // --- 1. Input Validation ---

  // Check if tensor is defined (not null/empty)
  if (!tensor.defined()) {
      errorBroadcaster.postError(
        "Error: tensorToAudio received an undefined tensor.");
    // DBG("Error: tensorToAudio received an undefined tensor.");

    return transportSeparation.trackBuffers;  // Return empty vector
  }

  // Check if tensor is on CPU, move if necessary
  if (tensor.is_cuda()) {
    juce::Logger::writeToLog(
        "Info: Tensor is on CUDA, moving to CPU for audio buffer conversion.");
    // DBG("Info: Tensor is on CUDA, moving to CPU for audio buffer
    // conversion.");
    try {
      tensor = tensor.cpu();
    } catch (const c10::Error& e) {
      // c10::Error is the base exception type for LibTorch/PyTorch C++ errors
      errorBroadcaster.postError("Error moving tensor to CPU: " +
                               juce::String(e.what()));
      // DBG("Error moving tensor to CPU: " + juce::String(e.what()));
      return transportSeparation.trackBuffers;  // Cannot proceed
    }
  }

  // Check data type - MUST be float
  if (tensor.scalar_type() != torch::kFloat) {
    errorBroadcaster.postError(
        "Error: Input tensor must be of type float (torch::kFloat). Actual "
        "type: " +
        juce::String(c10::toString(tensor.scalar_type())));
    // DBG("Error: Input tensor must be of type float (torch::kFloat).");
    // Optionally, you could try converting it: tensor =
    // tensor.to(torch::kFloat); But it's often better to enforce the correct
    // type upstream.
    return transportSeparation.trackBuffers;
  }

  // Check dimensions - MUST be 3D
  if (tensor.dim() != 3) {
    // Create a string representation of the shape for logging
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < tensor.dim(); ++i) {
      ss << tensor.size(i) << (i == tensor.dim() - 1 ? "" : ", ");
    }
    ss << "]";
    errorBroadcaster.postError(
        "Error: Input tensor must have 3 dimensions ([Ntracks, "
        "NchannelsPerTrack, NumSamples]). Actual shape: " +
        juce::String(ss.str()));
    // DBG(...); // Debug version
    return transportSeparation.trackBuffers;
  }

  // --- 2. Ensure Contiguity ---
  // Call this *after* potential .cpu() or .to() calls
  // Use try-catch as contiguous() can sometimes fail (e.g., out of memory)
  try {
    if (!tensor.is_contiguous()) {
      juce::Logger::writeToLog(
          "Info: Tensor was not contiguous, making it contiguous.");
      // DBG("Info: Tensor was not contiguous, making it contiguous.");
      tensor = tensor.contiguous();
    }
  } catch (const c10::Error& e) {
    errorBroadcaster.postError("Error making tensor contiguous: " +
                             juce::String(e.what()));
    // DBG("Error making tensor contiguous: " + juce::String(e.what()));
    return transportSeparation.trackBuffers;
  }

  // --- 3. Get Dimensions (Safer now after checks) ---
  const int numTracks = tensor.size(0);
  const int channelsPerTrack = tensor.size(1);
  const int numSamples = tensor.size(2);

  // Basic check for non-negative dimensions (numSamples can be 0)
  if (numTracks < 0 || channelsPerTrack < 0 || numSamples < 0) {
    errorBroadcaster.postError(
        "Error: Tensor dimensions are negative. This should not happen.");
    // DBG("Error: Tensor dimensions are negative.");
    return transportSeparation.trackBuffers;
  }
  // Handle case where there's nothing to process
  if (numTracks == 0 || channelsPerTrack == 0) {
    juce::Logger::writeToLog(
        "Info: Tensor has 0 tracks or 0 channels per track. Returning empty "
        "buffer list.");
    // DBG("Info: Tensor has 0 tracks or 0 channels per track.");
    return transportSeparation.trackBuffers;  // Nothing to copy
  }

  juce::Logger::writeToLog(
      "Processing Tensor Shape: [" + juce::String(numTracks) + ", " +
      juce::String(channelsPerTrack) + ", " + juce::String(numSamples) + "]");
  // DBG(...);

  // Reserve space in the output vector
  transportSeparation.trackBuffers.reserve(numTracks);

  // --- 4. Data Copy Loop (with error handling) ---
  try {
    for (int track = 0; track < numTracks; ++track) {
      // Create an AudioBuffer for this track
      // JUCE buffer constructor handles numSamples = 0 gracefully (allocates no
      // samples)
      juce::AudioBuffer<float> buffer(channelsPerTrack, numSamples);

      // Skip copy loop if there are no samples (saves unnecessary work)
      if (numSamples == 0) {
        transportSeparation.trackBuffers.push_back(
            std::move(buffer));  // Add empty buffer
        continue;                                   // Go to next track
      }

      // For each channel in this track, copy the tensor data
      for (int ch = 0; ch < channelsPerTrack; ++ch) {
        // Get the 1D slice for this channel [track, ch, :]
        // Indexing a contiguous 3D tensor like this typically results
        // in a contiguous 1D slice, so extra .contiguous() here is usually not
        // needed.
        torch::Tensor channel_slice = tensor.index({track, ch});

        // Get pointer to the source tensor data for this channel
        // Use const float* as we are only reading from the tensor
        const float* src = channel_slice.data_ptr<float>();

        // Sanity check: data_ptr should not be null if tensor/slice is valid
        if (src == nullptr) {
          juce::Logger::writeToLog(
              "Critical Error: data_ptr<float>() returned null for track " +
              juce::String(track) + ", channel " + juce::String(ch) +
              ". This should not happen after checks.");
          // DBG(...);
          throw std::runtime_error(
              "Failed to get data pointer from tensor slice.");  // Throw to
                                                                 // exit loops
        }

        // Get pointer to the destination JUCE buffer channel
        float* dest = buffer.getWritePointer(ch);

        // Perform the memory copy
        std::memcpy(dest, src, numSamples * sizeof(float));
      }
      // Move the filled buffer into the vector
      transportSeparation.trackBuffers.push_back(std::move(buffer));
    }
  } catch (const c10::Error& e) {  // Catch LibTorch specific errors during loop
    errorBroadcaster.postError(
        "LibTorch Error during tensor data copying loop: " +
        juce::String(e.what()));
    // DBG(...);
    transportSeparation.trackBuffers
        .clear();         // Ensure partial results are not returned
    return transportSeparation.trackBuffers;  // Return empty on error
  } catch (const std::exception& e) {  // Catch other potential errors (like the
                                       // runtime_error above)
    errorBroadcaster.postError(
        "Standard Exception during tensor data copying loop: " +
        juce::String(e.what()));
    // DBG(...);
    transportSeparation.trackBuffers.clear();
    return transportSeparation.trackBuffers;  // Return empty on error
  }

  juce::Logger::writeToLog("Tensor to Audio conversion successful.");
  // DBG("Tensor to Audio conversion successful.");

  
  return transportSeparation.trackBuffers;
}

torch::Tensor AudioPluginAudioProcessor::demix_track(
    double chunk_size,  // C in your code (as a float/double)
    int num_overlap,    // N
    int batch_size,
    const std::optional<std::string>&
        target_instrument,  // if provided, demix only one instrument
    const std::vector<std::string>& instruments,  // list of instruments
    torch::jit::Module& my_model,                 // your loaded model
    torch::Tensor mix,  // mix tensor, shape: [channels, samples]
    torch::Device device) {
  // Calculate parameters
  double C = chunk_size;
  int N = num_overlap;
  int step = static_cast<int>(C / N);  // integer division
  int border = static_cast<int>(C) - step;
  torch::Tensor estimated_sources;
  // Remember the original length (assumed along dimension 1)
  int length_init = mix.size(1);

  if ((length_init > 2 * border) && (border > 0)) {
    mix = torch::nn::functional::pad(
        mix, torch::nn::functional::PadFuncOptions({border, border})
                 .mode(torch::kReflect));
  }

  {
    torch::NoGradGuard no_grad;
    std::vector<int64_t> req_shape;
    if (target_instrument.has_value()) {  // Check if target_instrument is set
      req_shape = {1, mix.size(0), mix.size(1)};
    } else {
      req_shape = {static_cast<int64_t>(instruments.size()), mix.size(0),
                   mix.size(1)};
    }
    mix = mix.to(device);

    auto result = torch::zeros(req_shape, torch::kFloat32)
                      .to(device);  // Initialize result tensor
    auto counter = torch::zeros(req_shape, torch::kFloat32).to(device);

    std::vector<torch::Tensor> batch_data;  // Vector to store tensors
    std::vector<std::pair<int, int>>
        batch_locations;  // Vector to store (int, int) pairs

    int i = 0;
    estimated_sources = torch::zeros(req_shape, torch::kFloat32).to(device);
    juce::Logger::writeToLog("Entering elaboration loop\n");
    while (i < mix.size(1)) {
      torch::Tensor part =
          mix.index({Slice(), Slice(i, i + static_cast<int>(C))});
      int length = part.size(1);
      if (length < C) {
        if (length > 1 + static_cast<int>(C / 2)) {
          part = torch::nn::functional::pad(
              part, torch::nn::functional::PadFuncOptions(
                        {0, static_cast<int>(C) - length})
                        .mode(torch::kReflect));
        } else {
          part = torch::nn::functional::pad(
              part, torch::nn::functional::PadFuncOptions(
                        {0, static_cast<int>(C) - length, 0, 0})
                        .mode(torch::kConstant)
                        .value(0.0));
        }
      }
      batch_data.push_back(part);
      batch_locations.push_back({i, length});
      i += step;

      if ((batch_data.size() >= batch_size) || (i >= mix.size(1))) {
        auto arr = torch::stack(batch_data);
        torch::Tensor x = my_model.forward({arr}).toTensor();
        for (int j = 0; j < batch_locations.size(); j++) {
          int start = batch_locations[j].first;
          int l = batch_locations[j].second;

          // result[..., start:start+l] += x[j][..., :l]
          result.index_put_({Ellipsis, Slice(start, start + l)},
                            result.index({Ellipsis, Slice(start, start + l)}) +
                                x.index({j, Ellipsis, Slice(0, l)}));

          // counter[..., start:start+l] += 1.
          counter.index_put_(
              {Ellipsis, Slice(start, start + l)},
              counter.index({Ellipsis, Slice(start, start + l)}) + 1.0);
        }

        // Reset batch_data and batch_locations
        batch_data.clear();
        batch_locations.clear();
      }

      //estimated_sources = result / counter;
    }

    // Avoid division by zero where counter is still zero
    counter.masked_fill_(counter == 0.0,
                         1e-8);  // Or add a small epsilon everywhere
    estimated_sources = result / counter;

    // --- Handle NaNs (using the corrected method) ---
    torch::Tensor nan_mask = estimated_sources.isnan();
    estimated_sources.masked_fill_(nan_mask, 0.0);
  }

  //estimated_sources =
  //    estimated_sources.isnan().masked_fill(estimated_sources.isnan(), 0.0);

  // Remove padding if the conditions are met
  if (length_init > 2 * border && border > 0) {
    estimated_sources = estimated_sources.index(
        {Slice(), Slice(), Slice(border, length_init + border)});
  }

  return estimated_sources;
}

bool AudioPluginAudioProcessor::saveAudioBufferToWav(
    juce::AudioBuffer<float>& bufferToSave,
                          juce::File& outputFile,
                          double sampleRate,
                          unsigned int bitDepth)  // Default to 16-bit
{
  // 1. Ensure the output directory exists
  outputFile.getParentDirectory()
      .createDirectory();  // Create parent directory if it doesn't exist

  // 2. Create a WavAudioFormat object
  juce::WavAudioFormat wavFormat;

  // 3. Create an output stream (use unique_ptr for RAII)
  std::unique_ptr<juce::FileOutputStream> fileStream(
      outputFile.createOutputStream());

  if (fileStream == nullptr) {
    errorBroadcaster.postError(
        "Error: Could not create output stream for file: " +
        outputFile.getFullPathName());
    // You might want to use DBG() for debug builds or show an alert window
    // DBG ("Error: Could not create output stream for file: " +
    // outputFile.getFullPathName());
    return false;
  }

  // 4. Create an AudioFormatWriter (use unique_ptr for RAII)
  // Get buffer details
  const int numChannels = bufferToSave.getNumChannels();
  const int numSamples = bufferToSave.getNumSamples();

  std::unique_ptr<juce::AudioFormatWriter> writer(wavFormat.createWriterFor(
      fileStream.get(),           // The stream to write to
      sampleRate,                 // Sample rate
      (unsigned int)numChannels,  // Number of channels
      bitDepth,                   // Bit depth
      {},                         // Metadata (empty juce::StringPairArray)
      0)                          // Quality setting (0 for WAV)
  );

  if (writer == nullptr) {
    errorBroadcaster.postError("Error: Could not create WAV writer for file: " +
                             outputFile.getFullPathName());
    // DBG ("Error: Could not create WAV writer for file: " +
    // outputFile.getFullPathName()); fileStream will be deleted automatically
    // by its unique_ptr going out of scope here
    return false;
  }

  // --- Important ---
  // The writer now owns the stream, so release it from the unique_ptr
  // to prevent double deletion when both unique_ptrs go out of scope.
  fileStream.release();

  // 5. Write the buffer data to the file
  //    writeFromAudioSampleBuffer expects a const AudioBuffer<SampleType>&,
  //    the start sample index, and the number of samples to write.
  bool success =
      writer->writeFromAudioSampleBuffer(bufferToSave, 0, numSamples);

  if (!success) {
    errorBroadcaster.postError(
        "Error: Failed to write audio data to WAV file: " +
                             outputFile.getFullPathName());
    // DBG ("Error: Failed to write audio data to WAV file: " +
    // outputFile.getFullPathName()); Writer (and the stream it owns) will be
    // deleted automatically by unique_ptr
    return false;
  }

  // 6. Cleanup (handled automatically by unique_ptr for the writer)
  // The writer will flush and close the stream in its destructor.
  // You could explicitly call writer->flush() here if needed, but it's usually
  // not necessary.

  juce::Logger::writeToLog("Successfully saved WAV file: " +
                           outputFile.getFullPathName());
  // DBG ("Successfully saved WAV file: " + outputFile.getFullPathName());
  return true;
}


void AudioPluginAudioProcessor::saveSeparationIntoFile() {
  transportSeparation.separationNames.clear();
  juce::File documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    // --- Direct Call Example ---
  double sampleRate = getSampleRate();  // Get from your processor if applicable
  unsigned int bitDepth = 24;           // Example: 24-bit

  for (size_t i = 0; i < transportSeparation.separations.size(); ++i) {
    // Create a unique output file name for each track.
    juce::String fileName =
        transportOriginal.fileName + "_" + instruments[i];
    juce::File outputFile = documentsDir.getChildFile(fileName + ".wav");
    transportSeparation.separationNames.push_back(fileName);

    if (!transportSeparation.trackBuffers.empty()) {
      // Carica la prima traccia (es. kick drum) direttamente qui
      transportSeparation.load(
          0,
          fileSampleRate);  // Usa il sample rate del plugin
    }

    // Attempt to save the current audio buffer into a WAV file.
    if (saveAudioBufferToWav(transportSeparation.separations[i], outputFile,
                             sampleRate,
                             bitDepth)) {
      juce::Logger::writeToLog("Saved: " + fileName);
    } else {
      errorBroadcaster.postError("Failed to save: " + fileName);
    }
  }
}

void AudioPluginAudioProcessor::setupLibTorch() {
  // Ottieni la directory del plugin
  juce::File pluginFile =
      juce::File::getSpecialLocation(juce::File::currentExecutableFile);
  juce::File pluginDir = pluginFile.getParentDirectory();
  juce::String libTorchDllPath = pluginDir.getFullPathName();

  // Converti il percorso in formato wide string richiesto da SetDllDirectoryW
  const wchar_t* pathW = libTorchDllPath.toWideCharPointer();

  // Imposta la directory di ricerca DLL per questo processo
  // Windows cercherà in questa directory *prima* delle directory standard
  if (SetDllDirectoryW(pathW)) {
    juce::Logger::writeToLog(
        "Impostata directory di ricerca DLL per LibTorch: " + libTorchDllPath);
  } else {
    // Ottieni l'errore di Windows
    DWORD error = GetLastError();
    juce::String errorMessage =
        juce::String(
            "Errore nell'impostare la directory di ricerca DLL (Codice: ") +
        juce::String(error) + ")";
    errorBroadcaster.postError(errorMessage);
    // Potresti voler gestire questo errore in modo più robusto
  }
}


//void AudioPluginAudioProcessor::loadFileAtIndex(int index, int section) {
//  if (index >= 0 && index < loadedFiles.size()) {
//    auto file = loadedFiles[index];
//    auto myFile = std::make_unique<juce::File>(file);
//
//    formatReader = formatManager.createReaderFor(file);
//    if (formatReader != nullptr) {
//      std::unique_ptr<juce::AudioFormatReaderSource> tempSource(
//          new juce::AudioFormatReaderSource(formatReader, true));
//
//      if (section == 1) {  // Sezione superiore
//        transport.setSource(tempSource.get(), 0, nullptr,
//                             formatReader->sampleRate);
//        readerSource.reset(tempSource.release());
//        waveform.setSize(2, formatReader->lengthInSamples);
//        formatReader->read(&waveform, 0, formatReader->lengthInSamples, 0,
//                           true, true);
//        fileName = myFile->getFileNameWithoutExtension();
//      } else if (section == 2) {  // Sezione inferiore
//        transport2.setSource(tempSource.get(), 0, nullptr,
//                             formatReader->sampleRate);
//        readerSource2.reset(tempSource.release());
//        waveform2.setSize(2, formatReader->lengthInSamples);
//        formatReader->read(&waveform2, 0, formatReader->lengthInSamples, 0,
//                           true, true);
//        fileName2 = myFile->getFileNameWithoutExtension();
//      }
//
//      fileSampleRate = readerSource->getAudioFormatReader()->sampleRate;
//    }
//  }
//}



//void AudioPluginAudioProcessor::nextFile() {
//  if (!loadedFiles.empty()) {
//    currentFileIndex =
//        (currentFileIndex + 1) % loadedFiles.size();  // Cicla all'inizio
//    loadFileAtIndex(currentFileIndex, 2);
//  }
//}

//void AudioPluginAudioProcessor::previousFile() {
//  if (!loadedFiles.empty()) {
//    currentFileIndex = (currentFileIndex - 1 + loadedFiles.size()) %
//                       loadedFiles.size();  // Cicla alla fine
//    loadFileAtIndex(currentFileIndex, 2);
//  }
//}

//juce::AudioBuffer<float>& AudioPluginAudioProcessor::getWaveform(int section) {
//  if (section == 1) {
//    return waveform;  // Forma d'onda per la sezione superiore
//  } else if (section == 2) {
//    return waveform2;  // Forma d'onda per la sezione inferiore
//  }
//  return waveform;  // Default
//}

}  // namespace audio_plugin

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new audio_plugin::AudioPluginAudioProcessor();
}


