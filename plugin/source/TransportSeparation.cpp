#include "SlothPlugin/TransportSeparation.h"

namespace audio_plugin {
void TransportSeparation::setSampleCount(int newSampleCount) {
  sampleCount = newSampleCount;

  // Calcola la nuova posizione in secondi
  if (bufferReader.get() != nullptr) {
    auto targetPositionInSeconds =
        static_cast<double>(sampleCount) / fileSampleRate;

    // Applica lo smoothing
    // newPositionInSeconds += (targetPositionInSeconds - newPositionInSeconds)
    // * coeff;
    // Imposta la nuova posizione del trasporto
    transport.setPosition(targetPositionInSeconds);
    if (sampleCount == 0 && (params.playButton || params.playButton2))
      transport.start();
  }
}
void TransportSeparation::load(int indx, double sampleRate) {
  try {
    waveform.clear();
    stopFile();
    setSampleCount(0);

    // Crea bufferReader con il buffer separato
    bufferReader = std::make_unique<BufferAudioSource>(
        trackBuffers[indx], true);  // true = loop

    // Configura il transport
    transport.setSource(bufferReader.get(), 0, nullptr, sampleRate);
    transportStateChanged(Stopped);

    // Aggiorna la waveform
    waveform.makeCopyOf(trackBuffers[indx]);

  
    fileName = separationNames[indx];
  } catch (const std::exception& e) {
    juce::Logger::writeToLog("Index out of range: " + juce::String(e.what()));
    fileName = "Unknown";
  }

}

}  // namespace audio_plugin