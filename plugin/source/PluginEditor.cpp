#include "SlothPlugin/PluginEditor.h"
#include "SlothPlugin/PluginProcessor.h"

namespace audio_plugin {
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      tracks(),
      wave(p,
           p.transportOriginal.getWaveform(),
           p.transportOriginal.getSampleCount(),
           p.getFileSampleRate(),
           p.transportOriginal.fileName),
      snare(p,
            p.transportSeparation.getWaveform(),
            p.transportSeparation.getSampleCount(),
            p.getFileSampleRate(),
            p.transportSeparation.fileName),  // Qui andrebbe messo filename2 ma
                                              // la logica funziona
                          // ancora male
      audioProcessor(p) {
  startTimerHz(30);

  playButton.setAlpha(0.3f);
  loadButton.setAlpha(0.3f);
  divideButton.setAlpha(0.3f);
  playButton2.setAlpha(0.3f);

  loadButton.onClick = [&]() {
    audioProcessor.transportOriginal.load();
    updateTransportButtons(audioProcessor.params.playButtonParam->get());
    repaint();
  };

  // Bottone Play/Stop
  playButton.onClick = [&]() {
    if (audioProcessor.params.playButtonParam->get()) {
      audioProcessor.transportOriginal
          .stopFile(playButtonParamID);  // Se è in riproduzione, ferma
    } else {
      audioProcessor.transportOriginal
          .playFile(playButtonParamID);  // Se è fermo, avvia la riproduzione
    }
  };

  playButton2.onClick = [&]() {
    if (audioProcessor.params.playButton2Param->get()) {
      audioProcessor.transportSeparation.stopFile(
          playButton2ParamID);  // Se è in riproduzione, ferma
    } else {
      audioProcessor.transportSeparation.stopFile(
          playButton2ParamID);  // Se è in riproduzione, ferma
    }
  };



  divideButton.onClick = [&]() { audioProcessor.process(); };

  full.setColour(juce::GroupComponent::outlineColourId,
                 juce::Colours::transparentBlack);
  second.setColour(juce::GroupComponent::outlineColourId,
                   juce::Colours::transparentBlack);

  playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
  playButton.setEnabled(audioProcessor.transportOriginal.isFileLoaded());
  playButton2.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
  playButton2.setEnabled(audioProcessor.transportOriginal.isFileLoaded());
  divideButton.setEnabled(audioProcessor.transportOriginal.isFileLoaded());
  full.addAndMakeVisible(wave);
  full.addAndMakeVisible(loadButton);
  full.addAndMakeVisible(playButton);
  full.addAndMakeVisible(divideButton);
  second.addAndMakeVisible(snare);
  second.addAndMakeVisible(playButton2);

  for (int i = 0; i < 6; i++) {
    tracks.push_back(std::make_unique<juce::TextButton>(juce::String(i + 1)));
    tracks[i]->onClick = [this, i] {
      if (i < audioProcessor.transportSeparation.getSeparatedTracks().size()) {
        audioProcessor.transportSeparation.load(i,
                                  audioProcessor.getFileSampleRate());
      }
    };
    tracks[i]->setAlpha(0.3f);
    tracks[i]->setEnabled(true);
    second.addAndMakeVisible(tracks[i].get());
  }
  

  addAndMakeVisible(full);
  addAndMakeVisible(second);

  // setResizable(true, true); // Abilita il ridimensionamento manuale
  // setResizeLimits(800, 100, 2000, 1600); // Min: 400x300, Max: 2000x1600
  setSize(700, 200);

  audioProcessor.params.playButtonParam->addListener(this);
  audioProcessor.params.playButton2Param->addListener(this);
  addMouseListener(this, true);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
  audioProcessor.params.playButtonParam->removeListener(this);
  audioProcessor.params.playButton2Param->removeListener(this);
  removeMouseListener(this);
  stopTimer();
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);
}

void AudioPluginAudioProcessorEditor::resized() {
  full.setBounds(0, 0, getWidth(), getHeight() / 2);
  second.setBounds(0, full.getBottom(), getWidth(), getHeight() / 2);
  wave.setBounds(full.getLocalBounds());
  snare.setBounds(second.getLocalBounds());
  loadButton.setBounds(getWidth() / 2 - 25, 5, buttonWidth, buttonHeight);
  playButton.setBounds(getWidth() / 2 + 25, 5, buttonWidth, buttonHeight);
  divideButton.setBounds(getWidth() / 2, 5, buttonWidth, buttonHeight);
  playButton2.setBounds(getWidth() / 2, 5, buttonWidth, buttonHeight);
  //tracks[tracks.size() - 1]->setBounds(getWidth() - buttonWidth - 15, 5, buttonWidth, buttonHeight);
  for (int i = tracks.size() - 1; i >= 0; --i) {
    tracks[i]->setBounds(
        getWidth() - 15 - (buttonWidth + 5)* (tracks.size() - i), 5,
        buttonWidth, buttonHeight);
  }
}

void AudioPluginAudioProcessorEditor::parameterValueChanged(int, float value) {
  if (juce::MessageManager::getInstance()->isThisTheMessageThread()) {
    updateTransportButtons(value != 0.0f);
  } else {
    juce::MessageManager::callAsync(
        [this, value] { updateTransportButtons(value != 0.0f); });
  }
}

void AudioPluginAudioProcessorEditor::updateTransportButtons(bool status) {
  // DBG("qualcosa è cambiato");

  playButton.setEnabled(audioProcessor.transportOriginal.isFileLoaded());
  divideButton.setEnabled(audioProcessor.transportOriginal.isFileLoaded());

  if (status) {
    playButton.setButtonText("S");
    playButton.setColour(juce::TextButton::buttonColourId,
                         juce::Colours::red);  // Colore rosso per Stop
  } else {
    playButton.setButtonText("P");
    playButton.setColour(juce::TextButton::buttonColourId,
                         juce::Colours::green);  // Colore verde per Play
  }
}


void AudioPluginAudioProcessorEditor::timerCallback() {
  repaint();
}

void AudioPluginAudioProcessorEditor::mouseDoubleClick(
    const juce::MouseEvent& event) {
  // Ottieni un riferimento al Processor
  auto* processor = getAudioProcessor(); // Non viene usato... Rimuovere?

  // Riporta la traccia all'inizio
  audioProcessor.transportOriginal.setSampleCount(0);

  // Aggiorna l'interfaccia utente (se necessario)
  repaint();
}

void AudioPluginAudioProcessorEditor::mouseEnter(const juce::MouseEvent& event) {
    // Se il mouse è sopra un bottone, rendilo opaco
    if (event.eventComponent == &loadButton ||
        event.eventComponent == &playButton ||
        event.eventComponent == &divideButton ||
        event.eventComponent == &playButton2) 
    {
        event.eventComponent->setAlpha(1.0f);
        event.eventComponent->repaint();
    }
    else 
    {
        // Controlla se è uno dei pulsanti delle tracce
        for (auto& button : tracks) 
        {
            if (event.eventComponent == button.get()) 
            {
                button->setAlpha(1.0f);
                button->repaint();
                break;
            }
        }
    }
}

void AudioPluginAudioProcessorEditor::mouseExit(const juce::MouseEvent& event) {
    // Se il mouse esce da un bottone, rendilo trasparente
    if (event.eventComponent == &loadButton ||
        event.eventComponent == &playButton ||
        event.eventComponent == &divideButton ||
        event.eventComponent == &playButton2) 
    {
        event.eventComponent->setAlpha(0.3f);
        event.eventComponent->repaint();
    }
    else 
    {
        // Controlla se è uno dei pulsanti delle tracce
        for (auto& button : tracks) 
        {
            if (event.eventComponent == button.get()) 
            {
                button->setAlpha(0.3f);
                button->repaint();
                break;
            }
        }
    }
}


//void AudioPluginAudioProcessorEditor::mouseEnter(
//    const juce::MouseEvent& event) {
//  // Se il mouse è sopra un bottone, rendilo opaco
//  if (event.eventComponent == &loadButton ||
//      event.eventComponent == &playButton ||
//      event.eventComponent == &divideButton ||
//      event.eventComponent == &playButton2) {
//    event.eventComponent->setAlpha(1.0f);
//    event.eventComponent->repaint();
//  }
//}
//
//void AudioPluginAudioProcessorEditor::mouseExit(const juce::MouseEvent& event) {
//  // Se il mouse esce da un bottone, rendilo trasparente
//  if (event.eventComponent == &loadButton ||
//      event.eventComponent == &playButton ||
//      event.eventComponent == &divideButton ||
//      event.eventComponent == &playButton2) {
//    event.eventComponent->setAlpha(0.3f);
//    event.eventComponent->repaint();
//  }
//}

//void AudioPluginAudioProcessor::toggleTransport(int activeSection) {
//  if (activeSection == 1) {
//    if (params.playButtonParam->get()) {
//      stopFile(1);
//    } else {
//      playFile(1);
//      stopFile(2);  // Ferma la sezione inferiore
//    }
//  } else if (activeSection == 2) {
//    if (params.playButton2Param->get()) {
//      stopFile(2);
//    } else {
//      playFile(2);
//      stopFile(1);  // Ferma la sezione superiore
//    }
//  }
//}

}  // namespace audio_plugin
