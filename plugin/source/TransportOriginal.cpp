#include "SlothPlugin/TransportOriginal.h"


namespace audio_plugin {
bool TransportOriginal::load() {
  stopFile();
  setSampleCount(0);
  bool success = false;
  juce::FileChooser chooser{"Please load a file"};
  if (chooser.browseForFileToOpen()) {
    waveform.clear();
    TransportComponent::deleteTempFiles();
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
      TransportComponent::setFileSampleRate(readerSource->getAudioFormatReader()->sampleRate);
      TransportComponent::setFileBitDepth(
          readerSource->getAudioFormatReader()->bitsPerSample);
    }
    success = true;
  }
  return success;
}

bool TransportOriginal::load(const juce::String& path) {
  stopFile();
  setSampleCount(0);
  //DBG("Siamo dentro");
  auto file = juce::File(path);
  formatReader = formatManager.createReaderFor(file);
  bool success = false;
  if (formatReader != nullptr) {
    waveform.clear();
    TransportComponent::deleteTempFiles();
    std::unique_ptr<juce::AudioFormatReaderSource> tempSource(
        new juce::AudioFormatReaderSource(formatReader, true));
    transport.setSource(tempSource.get(), 0, nullptr, formatReader->sampleRate);
    transportStateChanged(Stopped);
    readerSource.reset(tempSource.release());
    auto sampleLenght = static_cast<int>(formatReader->lengthInSamples);
    waveform.setSize(2, sampleLenght);
    formatReader->read(&waveform, 0, sampleLenght, 0, true, true);
    TransportComponent::setFileSampleRate(readerSource->getAudioFormatReader()->sampleRate);
    TransportComponent::setFileBitDepth(
        readerSource->getAudioFormatReader()->bitsPerSample);
    stopFile();
    success = true;
  }
  return success;
}

}  // namespace audio_plugin