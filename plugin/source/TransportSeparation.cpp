#include "SlothPlugin/TransportSeparation.h"

namespace audio_plugin {
void TransportSeparation::setSampleCount(int newSampleCount) {
  sampleCount = newSampleCount;

  // Calcola la nuova posizione in secondi
  if (bufferReader.get() != nullptr) {
    auto targetPositionInSeconds =
        static_cast<double>(sampleCount) / TransportComponent::getFileSampleRate();

    // Applica lo smoothing
    // newPositionInSeconds += (targetPositionInSeconds - newPositionInSeconds)
    // * coeff;
    // Imposta la nuova posizione del trasporto
    transport.setPosition(targetPositionInSeconds);
    if (sampleCount == 0 && (params.playButton || params.playButton2))
      transport.start();
  }
}
bool TransportSeparation::load(int indx) {
  bool success = false;
  waveform.clear();
    try {
    stopFile();
    setSampleCount(0);

    // Crea bufferReader con il buffer separato
    bufferReader = std::make_unique<BufferAudioSource>(separations[indx],
                                                       true);  // true = loop

    // Configura il transport
    transport.setSource(bufferReader.get(), 0, nullptr, TransportComponent::getFileSampleRate());
    transportStateChanged(Stopped);

    // Aggiorna la waveform
    waveform.makeCopyOf(separations[indx]);

  
    fileName = TransportComponent::separationNames[indx];
    success = true;
  } catch (const std::exception& e) {
    juce::Logger::writeToLog("Index out of range: " + juce::String(e.what()));
    fileName = "Unknown";
  }
  return success;
}

void TransportSeparation::reset() {
  //waveform.setNotClear();
  waveform = juce::AudioBuffer<float>();
  separations.clear();
  fileName = "";
}


}  // namespace audio_plugin