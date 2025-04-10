#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>


namespace audio_plugin {

class BufferAudioSource : public juce::PositionableAudioSource {
public:
  // Costruttore: accetta un audio buffer e abilita opzionalmente il looping
  BufferAudioSource(const juce::AudioBuffer<float>& audioBuffer,
                    bool shouldLoop = false);

  // Override dei metodi di PositionableAudioSource
  void setNextReadPosition(juce::int64 newPosition) override;
  juce::int64 getNextReadPosition() const override;
  juce::int64 getTotalLength() const override;
  bool isLooping()
      const override;  // << Correzione: Sovrascrive il metodo corretto
  void setLooping(bool shouldLoop) override;

  void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
  void releaseResources() override;
  void getNextAudioBlock(const juce::AudioSourceChannelInfo& info) override;

private:
  const juce::AudioBuffer<float>& buffer;  // Riferimento al buffer in memoria
  juce::int64 position;  // Posizione corrente di lettura nel buffer
  bool loopingEnabled;        // Flag per il loop

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BufferAudioSource)
};

}  // namespace audio_plugin