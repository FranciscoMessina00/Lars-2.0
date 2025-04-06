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
WaveThumbnail::WaveThumbnail(AudioPluginAudioProcessor& p, int section) : audioProcessor(p), section(section) {
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.
}

WaveThumbnail::~WaveThumbnail() {}

void WaveThumbnail::paint(juce::Graphics& g) {
  auto waveform = audioProcessor.getWaveform(section);

  if (waveform.getNumSamples() > 0) {
    juce::Path p, q;
    audioPointsL.clear();
    audioPointsR.clear();

    float durationInSeconds =
        static_cast<float>(audioProcessor.getWaveform(section).getNumSamples()) /
        static_cast<float>(audioProcessor.getFileSampleRate());
    bool withMillis = true;
    if (durationInSeconds > 60)
      withMillis = false;
    auto ratio = waveform.getNumSamples() / (getWidth());
    auto bufferL = waveform.getReadPointer(0);
    auto bufferR = waveform.getReadPointer(1);

    g.setColour(juce::Colours::white.withAlpha(0.5f));
    g.setFont(float(getWidth()) / 80.0f);
    g.drawLine(0, getHeight(), getWidth(), getHeight(), 1.f);
    auto textBounds = getLocalBounds().reduced(7, 7);
    // Iterazione per disegnare le tacche
    for (int i = getWidth() / 10; i < getWidth(); i += getWidth() / 10) {
      auto x = juce::jmap<float>(i, 0.0, getWidth(), 0.0, (durationInSeconds));
      juce::String timeLabel = formatTime(x, withMillis);
      int labelWidth = g.getCurrentFont().getStringWidth(timeLabel);
      if (labelWidth + i < getWidth()) {
        g.drawText(timeLabel, i - 20, getHeight() - 15, 40, 10,
                   juce::Justification::centred);
        g.drawLine(i, getHeight() - 5, i, getHeight(), 1.0f);
      }
    }

    // Estrai i punti per i due canali
    for (int sample = 0; sample < waveform.getNumSamples(); sample += ratio) {
      audioPointsL.push_back(bufferL[sample]);
      audioPointsR.push_back(bufferR[sample]);
    }

    // Crea i percorsi per i due canali
    p.startNewSubPath(0, getHeight() / 2);  // Canale L
    q.startNewSubPath(0, getHeight() / 2);  // Canale R

    for (int sample = 0;
         sample < std::min(audioPointsL.size(), audioPointsR.size());
         ++sample) {
      // Ottieni i punti adiacenti per l'interpolazione (canale L)
      float y0L =
          (sample > 0) ? audioPointsL[sample - 1] : audioPointsL[sample];
      float y1L = audioPointsL[sample];
      float y2L = (sample < audioPointsL.size() - 1) ? audioPointsL[sample + 1]
                                                     : audioPointsL[sample];
      float y3L = (sample < audioPointsL.size() - 2) ? audioPointsL[sample + 2]
                                                     : audioPointsL[sample];

      // Ottieni i punti adiacenti per l'interpolazione (canale R)
      float y0R =
          (sample > 0) ? audioPointsR[sample - 1] : audioPointsR[sample];
      float y1R = audioPointsR[sample];
      float y2R = (sample < audioPointsR.size() - 1) ? audioPointsR[sample + 1]
                                                     : audioPointsR[sample];
      float y3R = (sample < audioPointsR.size() - 2) ? audioPointsR[sample + 2]
                                                     : audioPointsR[sample];

      // Sovracampionamento
      for (float t = 0.0f; t < 1.0f; t += 0.1f) {
        // Interpolazione cubica per il canale L
        float interpolatedPointL = cubicInterpolate(y0L, y1L, y2L, y3L, t);
        auto pointL =
            juce::jmap<float>(interpolatedPointL, -1.0f, 1.0f,
                              getHeight() - 5 - textBounds.getHeight(),
                              5 + textBounds.getHeight());

        // Interpolazione cubica per il canale R
        float interpolatedPointR = cubicInterpolate(y0R, y1R, y2R, y3R, t);
        auto pointR =
            juce::jmap<float>(interpolatedPointR, -1.0f, 1.0f,
                              getHeight() - 5 - textBounds.getHeight(),
                              5 + textBounds.getHeight());

        // Aggiungi i punti ai percorsi
        p.lineTo(sample + t, pointL);
        q.lineTo(sample + t, pointR);
      }
    }
    // Disegna i canali con trasparenza
    g.setColour(juce::Colour::fromFloatRGBA(1.0f, 0.3f, 0.3f,
                                            0.5f));  // Rosso più luminoso
    g.strokePath(p, juce::PathStrokeType(1.f));

    g.setColour(juce::Colour::fromFloatRGBA(0.3f, 1.0f, 0.3f,
                                            0.5f));  // Verde più luminoso
    g.strokePath(q, juce::PathStrokeType(1.f));

    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.setFont(11.0f);

    if (section == 1) 
    {
      g.drawFittedText(audioProcessor.fileName, textBounds,
                       juce::Justification::topLeft, 1);
    }

    else 
    {
      g.drawFittedText(audioProcessor.fileName2, textBounds,
                       juce::Justification::topLeft, 1);
    }

    auto playHeadPos =
        mapLinear(static_cast<float>(audioProcessor.getSampleCount()), 0.0f,
                  static_cast<float>(waveform.getNumSamples()), 0.0f,
                  static_cast<float>(getWidth()));

    g.setColour(juce::Colours::white);
    g.drawLine(playHeadPos, 0, playHeadPos, getHeight(), 2);
    if (playHeadPos > getWidth()) {
      audioProcessor.setSampleCount(0);
      audioProcessor.playFile(section);
    }

    /*DBG(audioProcessor.getSampleCount());
    DBG(waveform.getNumSamples());*/
  }
}

void WaveThumbnail::resized() {
  // This method is where you should set the bounds of any child
  // components that your component contains..
}

bool WaveThumbnail::isInterestedInFileDrag(const juce::StringArray& files) {
  for (auto file : files) {
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
      audioProcessor.fileName = myFile->getFileNameWithoutExtension();
      audioProcessor.loadFile(file);
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
  // Ottieni la posizione del click in coordinate locali
  auto mousePosition = event.getPosition().getX();
  // Imposta la nuova posizione della testina
  setPlayHeadPositionFromMouse(mousePosition);
}

void WaveThumbnail::mouseDrag(const juce::MouseEvent& event) {
  // Aggiorna la posizione della testina durante il trascinamento
  auto mousePosition = event.getPosition().getX();
  setPlayHeadPositionFromMouse(mousePosition);
}

void WaveThumbnail::setPlayHeadPositionFromMouse(int mouseX) {
  // Assicurati che il valore sia all'interno dei limiti del componente
  mouseX = juce::jlimit(0, getWidth(), mouseX);

  // Calcola il nuovo sampleCount in base alla posizione del mouse
  auto positionRatio = static_cast<float>(mouseX) / getWidth();
  auto newSampleCount = static_cast<long>(
      positionRatio * audioProcessor.getWaveform(section).getNumSamples());

  // Imposta il nuovo sampleCount
  audioProcessor.setSampleCount(newSampleCount);

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
}  // namespace audio_plugin
