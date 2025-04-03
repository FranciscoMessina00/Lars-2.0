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
      params(apvts)
#endif
{
  formatManager.registerBasicFormats();
  state = Stopped;
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {
  formatReader = nullptr;
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
  transport.prepareToPlay(samplesPerBlock, sampleRate);

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

  if (readerSource.get() == nullptr) {
    buffer.clear();
    return;
  }

  double systemSampleRate = getSampleRate();
  sampleRateRatio = fileSampleRate / systemSampleRate;

  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  if (params.playButton)
    sampleCount += static_cast<long>(buffer.getNumSamples() * sampleRateRatio);
  // sampleCount = params.playButton ? sampleCount +=
  // static_cast<long>(buffer.getNumSamples() * sampleRateRatio) : 0;

  juce::AudioSourceChannelInfo channelInfo(&buffer, 0, buffer.getNumSamples());
  transport.getNextAudioBlock(channelInfo);

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

void AudioPluginAudioProcessor::loadFile() {
  stopFile();
  setSampleCount(0);
  juce::FileChooser chooser{"Please load a file"};
  if (chooser.browseForFileToOpen()) {
    auto file = chooser.getResult();
    auto myFile = std::make_unique<juce::File>(file);
    fileName = myFile->getFileNameWithoutExtension();
    formatReader = formatManager.createReaderFor(file);
    if (formatReader != nullptr) {
      std::unique_ptr<juce::AudioFormatReaderSource> tempSource(
          new juce::AudioFormatReaderSource(formatReader, true));
      transport.setSource(tempSource.get(), 0, nullptr,
                          formatReader->sampleRate);
      transportStateChanged(Stopped);
      readerSource.reset(tempSource.release());
      auto sampleLenght = static_cast<int>(formatReader->lengthInSamples);
      waveform.setSize(2, sampleLenght);
      formatReader->read(&waveform, 0, sampleLenght, 0, true, true);
      fileSampleRate = readerSource->getAudioFormatReader()->sampleRate;
    }
  }
}

void AudioPluginAudioProcessor::loadFile(const juce::String& path) {
  stopFile();
  setSampleCount(0);
  DBG("Siamo dentro");
  auto file = juce::File(path);
  formatReader = formatManager.createReaderFor(file);
  if (formatReader != nullptr) {
    std::unique_ptr<juce::AudioFormatReaderSource> tempSource(
        new juce::AudioFormatReaderSource(formatReader, true));
    transport.setSource(tempSource.get(), 0, nullptr, formatReader->sampleRate);
    transportStateChanged(Stopped);
    readerSource.reset(tempSource.release());
    auto sampleLenght = static_cast<int>(formatReader->lengthInSamples);
    waveform.setSize(2, sampleLenght);
    formatReader->read(&waveform, 0, sampleLenght, 0, true, true);
    fileSampleRate = readerSource->getAudioFormatReader()->sampleRate;
    stopFile();
  }
}

void AudioPluginAudioProcessor::playFile() {
  // isPlaying = true;
  apvts.getParameter(playButtonParamID.getParamID())
      ->setValueNotifyingHost(1.0f);
  transportStateChanged(Starting);
  params.playButton = params.playButtonParam->get();
}

void AudioPluginAudioProcessor::stopFile() {
  // isPlaying = false;
  apvts.getParameter(playButtonParamID.getParamID())
      ->setValueNotifyingHost(0.0f);
  transportStateChanged(Stopping);
  params.playButton = params.playButtonParam->get();
}

void AudioPluginAudioProcessor::transportStateChanged(TransportState newState) {
  if (newState != state) {
    state = newState;

    switch (state) {
      case Stopped:
        transport.setPosition(0.0f);
        break;

      case Playing:
        break;

      case Starting:
        transport.start();
        break;

      case Stopping:
        transport.stop();
        // transport.setPosition(0.0f);
        break;
    }
  }
}

void AudioPluginAudioProcessor::setSampleCount(int newSampleCount) {
  // Limita il valore di sampleCount alla lunghezza del file audio
  // auto maxSampleCount = static_cast<long>(readerSource->getTotalLength());
  // sampleCount = juce::jlimit(0L, maxSampleCount, newSampleCount);
  sampleCount = newSampleCount;

  // Calcola la nuova posizione in secondi
  if (readerSource.get() != nullptr) {
    auto targetPositionInSeconds =
        static_cast<double>(sampleCount) / fileSampleRate;

    // Applica lo smoothing
    // newPositionInSeconds += (targetPositionInSeconds - newPositionInSeconds)
    // * coeff;
    // Imposta la nuova posizione del trasporto
    transport.setPosition(targetPositionInSeconds);
    if (sampleCount == 0 && params.playButton)
      transport.start();
  }
}

void AudioPluginAudioProcessor::process() {
  // Do something
}

//==============================================================================
// This creates new instances of the plugin..
}  // namespace audio_plugin

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new audio_plugin::AudioPluginAudioProcessor();
}
