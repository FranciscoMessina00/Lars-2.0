#include "SlothPlugin/BufferAudioSource.h"

namespace audio_plugin {
// Costruttore
BufferAudioSource::BufferAudioSource(
    const juce::AudioBuffer<float>& audioBuffer,
    bool shouldLoop)
    : buffer(audioBuffer), position(0), loopingEnabled(shouldLoop) {}

// Imposta la prossima posizione di lettura
void BufferAudioSource::setNextReadPosition(juce::int64 newPosition) {
  position =
      juce::jlimit((juce::int64)0, (juce::int64)getTotalLength(), newPosition);
}

// Restituisce la posizione di lettura corrente
juce::int64 BufferAudioSource::getNextReadPosition() const {
  return position;
}

// Restituisce la lunghezza totale del buffer
juce::int64 BufferAudioSource::getTotalLength() const {
  return buffer.getNumSamples();
}

// Controlla se il loop è attivo
bool BufferAudioSource::isLooping() const {
  return loopingEnabled;
}

// Imposta il loop
void BufferAudioSource::setLooping(bool shouldLoop) {
  loopingEnabled = shouldLoop;
}

// Prepara alla riproduzione (override obbligatorio)
void BufferAudioSource::prepareToPlay(int samplesPerBlockExpected,
                                      double sampleRate) {
  // Nessuna configurazione necessaria
}

// Rilascia le risorse (override obbligatorio)
void BufferAudioSource::releaseResources() {
  // Nessuna risorsa specifica da rilasciare
}

// Fornisce i prossimi blocchi audio
void BufferAudioSource::getNextAudioBlock(
    const juce::AudioSourceChannelInfo& info) {
  if (position >= getTotalLength()) {
    info.clearActiveBufferRegion();
    return;  // Fine del buffer raggiunta
  }

  auto numSamples =
      juce::jmin((juce::int64)info.numSamples, getTotalLength() - position);

  for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
    if (info.buffer->getNumChannels() > channel) {
      info.buffer->copyFrom(channel, info.startSample,
                            buffer.getReadPointer(channel, (int)position),
                            (int)numSamples);
    }
  }

  position += numSamples;

  if (loopingEnabled && position >= getTotalLength()) {
    position = 0;  // Se in loop, riparte dall'inizio
  }
}
}  // namespace audio_plugin
