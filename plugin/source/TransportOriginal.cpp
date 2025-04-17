#include "SlothPlugin/TransportOriginal.h"


namespace audio_plugin {
void TransportOriginal::load() {
  stopFile(playButtonParamID);
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

void TransportOriginal::load(const juce::String& path) {
  stopFile(playButtonParamID);
  setSampleCount(0);
  //DBG("Siamo dentro");
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
    stopFile(playButtonParamID);
  }
}

}  // namespace audio_plugin