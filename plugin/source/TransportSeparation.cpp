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
    bufferReader = std::make_unique<BufferAudioSource>(
        trackBuffers[indx], true);  // true = loop

    // Configura il transport
    transport.setSource(bufferReader.get(), 0, nullptr, TransportComponent::getFileSampleRate());
    transportStateChanged(Stopped);

    // Aggiorna la waveform
    waveform.makeCopyOf(trackBuffers[indx]);

  
    fileName = TransportComponent::separationNames[indx];
    success = true;
  } catch (const std::exception& e) {
    juce::Logger::writeToLog("Index out of range: " + juce::String(e.what()));
    fileName = "Unknown";
  }
  return success;
}

void TransportSeparation::reset() {
  waveform.setNotClear();
  waveform.clear();
  trackBuffers.clear();
  separations.clear(); // Non so perché ce ne sono due... sono da unire, fanno la stessa cosa ma vengono chiamati in punti diversi
  fileName = "";
}


}  // namespace audio_plugin