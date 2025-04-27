#include "SlothPlugin/TransportComponent.h"

namespace audio_plugin {

double TransportComponent::fileSampleRate = 44100.0;
unsigned int TransportComponent::fileBitDepth = 24;

void TransportComponent::setSampleCount(int newSampleCount) {
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
    if (sampleCount == 0 && (params.playButton || params.playButton2))
      transport.start();
  }
}

void TransportComponent::transportStateChanged(TransportState newState) {
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

void TransportComponent::playFile() {
  apvts.getParameter(paramId.getParamID())
      ->setValueNotifyingHost(1.0f);
  transportStateChanged(Starting);
  params.playButton = params.playButtonParam->get();
  params.playButton2 = params.playButton2Param->get();
}

void TransportComponent::stopFile() {
  apvts.getParameter(paramId.getParamID())
      ->setValueNotifyingHost(0.0f);
  transportStateChanged(Stopping);
  params.playButton = params.playButtonParam->get();
  params.playButton2 = params.playButton2Param->get();
}

void TransportComponent::setFileSampleRate(double sampleRate) 
{
  fileSampleRate = sampleRate;
}

double TransportComponent::getFileSampleRate() 
{
  return fileSampleRate;
}

void TransportComponent::setFileBitDepth(unsigned int bitDepth) 
{
  TransportComponent::fileBitDepth = bitDepth;
}

unsigned int TransportComponent::getFileBitDepth() 
{
  return fileBitDepth;
}

}  // namespace audio_plugin