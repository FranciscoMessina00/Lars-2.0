#include "SlothPlugin/TransportSeparation.h"

namespace audio_plugin {
void TransportSeparation::load(int indx, double sampleRate) {
  try {
    waveform.clear();
    stopFile(playButton2ParamID);
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