#include "SlothPlugin/TransportSeparation.h"

namespace audio_plugin {
void TransportSeparation::load(int indx, double sampleRate) {
  waveform.clear();
  stopFile(playButton2ParamID);
  setSampleCount(0);

  // Crea bufferReader con il buffer separato
  bufferReader = std::make_unique<BufferAudioSource>(
      trackBuffers[indx], false);  // false = no loop di default

  // Configura il transport
  transport.setSource(bufferReader.get(), 0, nullptr, sampleRate);
  transportStateChanged(Stopped);

  // Aggiorna la waveform
  waveform.makeCopyOf(trackBuffers[indx]);
  // fileName2 = &separationNames[indx];

}

}  // namespace audio_plugin