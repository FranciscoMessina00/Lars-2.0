/*
  ==============================================================================

    WaveThumbnail.cpp
    Created: 29 Mar 2025 4:49:19pm
    Author:  giuli

  ==============================================================================
*/


#include "SlothPlugin/WaveThumbnail.h"

//==============================================================================
namespace audio_plugin {
WaveThumbnail::WaveThumbnail(
    AudioPluginAudioProcessor& p,
    const juce::AudioBuffer<float>& bufferToDraw,  // Fix posizione
    std::atomic<int>& sampleCountRef,
    double sampleRate, const juce::String& name, TransportComponent& transportRef)
    : audioProcessor(p),
      bufferRef(bufferToDraw),
      sampleCount(sampleCountRef),
      fileSampleRate(sampleRate),
      fileName(name),
      transport(transportRef) {
  setInterceptsMouseClicks(true, false);
}


WaveThumbnail::~WaveThumbnail() {}

void WaveThumbnail::paint(juce::Graphics& g) {
  if (bufferRef.getNumSamples() == 0)
    return;

  // 1. Calcolo durata audio
  const float duration = bufferRef.getNumSamples() / fileSampleRate;

  // 2. Disegno waveform (separato dalla logica della sezione)
  drawWaveform(g);

  // 3. Disegno playhead (usa sampleCount interno al componente)
  const float playHeadPosX = juce::jmap<float>(
      sampleCount.load(), 0, bufferRef.getNumSamples(), 0, getWidth());

  g.setColour(juce::Colours::white);
  g.drawLine(playHeadPosX, 0, playHeadPosX, getHeight(), 2.0f);
  if (playHeadPosX >= getWidth()) {
    transport.setSampleCount(0);
  }
}


void WaveThumbnail::resized() {
  // This method is where you should set the bounds of any child
  // components that your component contains..
}

bool WaveThumbnail::isInterestedInFileDrag(const juce::StringArray& files) {
  for (auto file : files) { // Ciclo for evitabile? Lo fa fuori
    if (file.contains(".wav") || file.contains(".mp3") ||
        file.contains(".aif") || file.contains(".flac"))
      return true;
  }

  return false;
}

void WaveThumbnail::filesDropped(const juce::StringArray& files, int x, int y) {
  for (auto file : files) {
    if (isInterestedInFileDrag(file)) {
      auto myFile = std::make_unique<juce::File>(file);
      audioProcessor.transportOriginal.fileName = myFile->getFileNameWithoutExtension();
      audioProcessor.transportOriginal.load(file);
    }
  }
  repaint();
}

float WaveThumbnail::mapLinear(float value,
                               float inputMin,
                               float inputMax,
                               float outputMin,
                               float outputMax) {
  // Calcolo la proporzione
  float normalized = (value - inputMin) / (inputMax - inputMin);

  // Applico la proporzione all'intervallo di output
  float mappedValue = outputMin + normalized * (outputMax - outputMin);

  // Ritorna il valore all'interno del range di output
  return mappedValue;
}

void WaveThumbnail::mouseDown(const juce::MouseEvent& event) {
  // Verifica che il click sia avvenuto all'interno dei bounds di questo
  // componente
  /*juce::Logger::writeToLog("--- mouseDown ---");
  juce::Logger::writeToLog("Local bounds: " + getLocalBounds().toString());
  juce::Logger::writeToLog("Mouse position: " + event.getPosition().toString());
  juce::Logger::writeToLog("Parent bounds: " +
                           getParentComponent()->getLocalBounds().toString());*/

  if (!getLocalBounds().contains(event.getPosition())) {
    return;
  }

  auto mousePosition = event.getPosition().getX();
  
  // Imposta la nuova posizione della testina
  setPlayHeadPositionFromMouse(mousePosition);

}

void WaveThumbnail::mouseDrag(const juce::MouseEvent& event) {
  // Verifica che il drag stia avvenendo all'interno dei bounds di questo
  // componente
  if (!getLocalBounds().contains(event.getPosition())) {
    return;
  }

  auto mousePosition = event.getPosition().getX();
  auto y = event.getPosition().getY();

  // Imposta la nuova posizione della testina
  setPlayHeadPositionFromMouse(mousePosition);
}

void WaveThumbnail::setPlayHeadPositionFromMouse(int mouseX) {
  // Assicurati che il valore sia all'interno dei limiti del componente
  mouseX = juce::jlimit(0, getWidth(), mouseX);

  // Calcola il nuovo sampleCount in base alla posizione del mouse
  auto positionRatio = static_cast<float>(mouseX) / getWidth();
  /*auto newSampleCount = static_cast<long>(
      positionRatio * audioProcessor.getWaveform(section).getNumSamples());*/
  auto newSampleCount = static_cast<long>(
      positionRatio * transport.getWaveform().getNumSamples());

  // Imposta il nuovo sampleCount
  transport.setSampleCount(newSampleCount);

  // Forza il ridisegno del componente
  repaint();
}

juce::String WaveThumbnail::formatTime(float seconds, bool isLong) {
  if (seconds < 60.f && isLong) {
    int sec = static_cast<int>(seconds);
    int millis = static_cast<int>(((seconds - sec) * 1000.0f));

    return juce::String(sec) + "." + juce::String(millis).paddedLeft('0', 3);
  }
  /*else if (seconds < 60.f) {
      return "00:" + juce::String(static_cast<int>(seconds));
  }*/
  else {
    int minutes = seconds / 60;  // Calcola i minuti
    int remainingSeconds =
        static_cast<int>(seconds) % 60;  // Calcola i secondi rimanenti
    return juce::String(minutes) + ":" +
           juce::String(remainingSeconds)
               .paddedLeft('0', 2);  // Formato "minuti.secondi"
  }
}

float WaveThumbnail::cubicInterpolate(float y0,
                                      float y1,
                                      float y2,
                                      float y3,
                                      float t) {
  float a = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
  float b = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
  float c = -0.5f * y0 + 0.5f * y2;
  float d = y1;

  return a * t * t * t + b * t * t + c * t + d;
}

void WaveThumbnail::drawWaveform(juce::Graphics& g) {
  const auto& waveform = bufferRef;
  if (waveform.getNumSamples() == 0)
    return;

  // 1. Parametri iniziali
  const int width = getWidth();
  const int height = getHeight();
  const float duration = static_cast<float>(waveform.getNumSamples()) /
                         static_cast<float>(fileSampleRate);
  bool withMillis = true;
  if (duration > 60)
    withMillis = false;

  // 2. Disegno linee temporali
  g.setColour(juce::Colours::white.withAlpha(0.5f));
  g.setFont(float(getWidth()) / 80.0f);
  g.drawLine(0, getHeight(), getWidth(), getHeight(), 1.f);

  for (int i = getWidth() / 10; i < getWidth(); i += getWidth() / 10) {
    auto x = juce::jmap<float>(i, 0.0, getWidth(), 0.0, (duration));
    juce::String timeLabel = formatTime(x, withMillis);
    int labelWidth = g.getCurrentFont().getStringWidth(timeLabel);
    if (labelWidth + i < getWidth()) {
      g.drawText(timeLabel, i - 20, getHeight() - 15, 40, 10,
                 juce::Justification::centred);
      g.drawLine(i, getHeight() - 5, i, getHeight(), 1.0f);
    }
  }

  // 3. Punti audio grezzi
  const float ratio = waveform.getNumSamples() / static_cast<float>(width);
  const auto* bufferL = waveform.getReadPointer(0);
  const auto* bufferR =
      waveform.getNumChannels() > 1 ? waveform.getReadPointer(1) : bufferL;

  std::vector<float> pointsL, pointsR;
  for (int sample = 0; sample < waveform.getNumSamples(); sample += ratio) {
    pointsL.push_back(bufferL[sample]);
    pointsR.push_back(bufferR[sample]);
  }

  // 4. Costruzione path con interpolazione originale
  juce::Path pathL, pathR;
  pathL.startNewSubPath(0, height / 2.0f);
  pathR.startNewSubPath(0, height / 2.0f);

  for (int i = 0; i < pointsL.size(); ++i) {
    // Preparazione punti interpolazione (L)
    const float prevL = (i > 0) ? pointsL[i - 1] : pointsL[i];
    const float currL = pointsL[i];
    const float nextL = (i < pointsL.size() - 1) ? pointsL[i + 1] : currL;
    const float nextNextL = (i < pointsL.size() - 2) ? pointsL[i + 2] : nextL;

    // Preparazione punti interpolazione (R)
    const float prevR = (i > 0) ? pointsR[i - 1] : pointsR[i];
    const float currR = pointsR[i];
    const float nextR = (i < pointsR.size() - 1) ? pointsR[i + 1] : currR;
    const float nextNextR = (i < pointsR.size() - 2) ? pointsR[i + 2] : nextR;

    // Interpolazione cubica per ogni step
    for (float t = 0.0f; t < 1.0f; t += 0.1f) {
      const float samplePos = static_cast<float>(i) + t;
      const float x =
          juce::jmap(samplePos, 0.0f, static_cast<float>(pointsL.size()), 0.0f,
                     static_cast<float>(width));

      // Canale L
      const float interpL = cubicInterpolate(prevL, currL, nextL, nextNextL, t);
      const float yL = juce::jmap(
          interpL, -1.0f, 1.0f, 5.0f + g.getCurrentFont().getHeight(),
          height - 5.0f - g.getCurrentFont().getHeight());
      pathL.lineTo(x, yL);

      // Canale R
      const float interpR = cubicInterpolate(prevR, currR, nextR, nextNextR, t);
      const float yR = juce::jmap(
          interpR, -1.0f, 1.0f, 5.0f + g.getCurrentFont().getHeight(),
          height - 5.0f - g.getCurrentFont().getHeight());
      pathR.lineTo(x, yR);
    }
  }

  // 5. Disegno finali
  g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.3f, 0.3f, 0.5f));
  g.strokePath(pathL, juce::PathStrokeType(1.0f));

  g.setColour(juce::Colour::fromFloatRGBA(0.3f, 1.0f, 0.3f, 0.5f));
  g.strokePath(pathR, juce::PathStrokeType(1.0f));

  // 6. Nome file in alto a sinistra
  g.setColour(juce::Colours::white.withAlpha(0.7f));
  g.setFont(11.0f);
  g.drawFittedText(fileName, getLocalBounds().reduced(7, 7),
                   juce::Justification::topLeft, 1);
}


void WaveThumbnail::mouseDoubleClick(
    const juce::MouseEvent& event) {
  // Riporta la traccia all'inizio
  transport.setSampleCount(0);
  repaint();
}


}  // namespace audio_plugin
