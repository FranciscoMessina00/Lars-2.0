#include "SlothPlugin/PluginEditor.h"
#include "SlothPlugin/PluginProcessor.h"

namespace audio_plugin {
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      tracks(),
      original(p,
           p.transportOriginal.getWaveform(),
           p.transportOriginal.getSampleCount(),
           TransportComponent::getFileSampleRate(),
           p.transportOriginal.fileName, 
           p.transportOriginal),
      separation(
          p,
            p.transportSeparation.getWaveform(),
            p.transportSeparation.getSampleCount(),
            TransportComponent::getFileSampleRate(),
            p.transportSeparation.fileName,
            p.transportSeparation),  
      audioProcessor(p) {
  startTimerHz(30);

  playButton.setAlpha(0.3f);
  loadButton.setAlpha(0.3f);
  divideButton.setAlpha(0.3f);
  playButton2.setAlpha(0.3f);

  juce::StringArray items;
  items.add("Mdx23c - Fast");
  items.add("Mdx23c - Slow");
  trackSelector.addItemList(items, 1);
  trackSelector.setSelectedId(1, juce::dontSendNotification);
  trackSelector.onChange = [&]() {
    int selectedIndex = trackSelector.getSelectedItemIndex();
    juce::Logger::writeToLog("Selected index: " + juce::String(selectedIndex));
    
    switch (selectedIndex) {
    case 0:
        audioProcessor.modelName = "mdx23c.pt";
        setTrackButtons(6);
        juce::Logger::writeToLog("Selected model: mdx23c.pt");
        break;
    case 1:
        audioProcessor.modelName = "mdx23c_capstone.pt";
        setTrackButtons(5);
        juce::Logger::writeToLog("Selected model: mdx23c_capstone.pt");
        break;
    default:
        audioProcessor.modelName = "mdx23c.pt";
        break;
    }
    
  };

  loadButton.onClick = [&]() {
    if (audioProcessor.transportOriginal.load()) {
      audioProcessor.transportSeparation.reset();
      playButton2.setEnabled(false);
      separation.repaint();
    };
    //updateTransportButtons(audioProcessor.params.playButtonParam->get());
    playButton.setEnabled(audioProcessor.transportOriginal.isFileLoaded());
    divideButton.setEnabled(audioProcessor.transportOriginal.isFileLoaded());
    repaint();
  };

  // Bottone Play/Stop
  /*playButton.onClick = [&]() {
    if (audioProcessor.params.playButtonParam->get()) {
      updateTransportButtons(0, audioProcessor.params.playButtonParam->get());
      audioProcessor.transportOriginal.stopFile(playButtonParamID);
    }
    else {
      updateTransportButtons(0, audioProcessor.params.playButtonParam->get());
      audioProcessor.transportOriginal.playFile(playButtonParamID);
    }
  };*/

  playButton.onClick = [&]() {
    updateTransportButtons(0, audioProcessor.params.playButtonParam->get());
    audioProcessor.params.playButtonParam->get()
        ? audioProcessor.transportOriginal.stopFile()
        : (
           audioProcessor.transportOriginal.playFile(),
           audioProcessor.transportSeparation.stopFile()
          );
  };

  playButton2.onClick = [&]() {
    updateTransportButtons(1, audioProcessor.params.playButton2Param->get());
    audioProcessor.params.playButton2Param->get()
        ? audioProcessor.transportSeparation.stopFile()
        : (
           audioProcessor.transportSeparation.playFile(),
           audioProcessor.transportOriginal.stopFile()
          );
  };



  divideButton.onClick = [&]() {
      original.setMouseCursor(juce::MouseCursor::WaitCursor);
      separation.setMouseCursor(juce::MouseCursor::WaitCursor);
      loadButton.setEnabled(false);
      playButton2.setEnabled(false);
      divideButton.setEnabled(false);
      audioProcessor.transportSeparation.reset();
      audioProcessor.process();
  };

  full.setColour(juce::GroupComponent::outlineColourId,
                 juce::Colours::transparentBlack);
  second.setColour(juce::GroupComponent::outlineColourId,
                   juce::Colours::transparentBlack);

  playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
  playButton.setEnabled(audioProcessor.transportOriginal.isFileLoaded());
  playButton2.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
  playButton2.setEnabled(audioProcessor.transportSeparation.isFileLoaded());
  divideButton.setEnabled(audioProcessor.transportOriginal.isFileLoaded());
  full.addAndMakeVisible(original);
  full.addAndMakeVisible(loadButton);
  full.addAndMakeVisible(playButton);
  full.addAndMakeVisible(divideButton);
  full.addAndMakeVisible(trackSelector);
  second.addAndMakeVisible(separation);
  second.addAndMakeVisible(playButton2);

  setTrackButtons(6);
  

  addAndMakeVisible(full);
  addAndMakeVisible(second);

  // setResizable(true, true); // Abilita il ridimensionamento manuale
  // setResizeLimits(800, 100, 2000, 1600); // Min: 400x300, Max: 2000x1600
  setSize(700, 200);

  audioProcessor.params.playButtonParam->addListener(this);
  audioProcessor.params.playButton2Param->addListener(this);
  audioProcessor.eventBroadcaster.addActionListener(this);
  addMouseListener(this, true);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
  audioProcessor.params.playButtonParam->removeListener(this);
  audioProcessor.params.playButton2Param->removeListener(this);
  audioProcessor.eventBroadcaster.removeActionListener(this);
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
  
  //full.setBounds(area.removeFromTop(area.getHeight() / 2));
  //second.setBounds(area); 
  
  //juce::Logger::writeToLog(second.getBounds().toString());
  original.setBounds(full.getLocalBounds());
  separation.setBounds(second.getLocalBounds());
  loadButton.setBounds(getWidth() / 2 - 25, 5, buttonWidth, buttonHeight);
  playButton.setBounds(getWidth() / 2 + 25, 5, buttonWidth, buttonHeight);
  divideButton.setBounds(getWidth() / 2, 5, buttonWidth, buttonHeight);
  trackSelector.setBounds(getWidth() / 10, 5, 120, 20);
  playButton2.setBounds(getWidth() / 2, 5, buttonWidth, buttonHeight);
  //tracks[tracks.size() - 1]->setBounds(getWidth() - buttonWidth - 15, 5, buttonWidth, buttonHeight);
  for (int i = tracks.size() - 1; i >= 0; --i) {
    tracks[i]->setBounds(
        getWidth() - 15 - (buttonWidth + 5)* (tracks.size() - i), 5,
        buttonWidth, buttonHeight);
  }
}

//void AudioPluginAudioProcessorEditor::parameterValueChanged(int, float value) {
//  juce::Logger::writeToLog("Value: " + juce::String(value));
//    if (juce::MessageManager::getInstance()->isThisTheMessageThread()) {
//    updateTransportButtons(value != 0.0f);
//  } else {
//    juce::MessageManager::callAsync(
//        [this, value] { updateTransportButtons(value != 0.0f); });
//  }
//}

void AudioPluginAudioProcessorEditor::parameterValueChanged(int idx,
                                                            float value) {
  auto updateButton = [this, value, idx](bool isPlaying) {
    if (juce::MessageManager::getInstance()->isThisTheMessageThread()) {
      updateTransportButtons(idx, isPlaying);
    } else {
      juce::MessageManager::callAsync([this, value, idx] {
          updateTransportButtons(idx, value != 0.0f);
      });
    }
  };

  switch (idx) {
    case 0:  // Primo parametro (original)
      updateButton(value != 0.0f);
      break;
    case 1:  // Secondo parametro (separated)
      updateButton(value != 0.0f);
      break;
    default:
      break;
  }
}

void AudioPluginAudioProcessorEditor::actionListenerCallback(const juce::String& message) {
    if (message.startsWith("Error:")) {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                "Plugin Error", message);
    } else if (message.startsWith("Demix finished")) {
        loadButton.setEnabled(true);
        playButton2.setEnabled(true);
        divideButton.setEnabled(true);
        original.setMouseCursor(juce::MouseCursor::NormalCursor);
        separation.setMouseCursor(juce::MouseCursor::NormalCursor);
    }
}

void AudioPluginAudioProcessorEditor::updateTransportButtons(int sourceIndex, bool isPlaying) {
  switch (sourceIndex) {
    case 0:  // Original
      //playButton.setToggleState(isPlaying, juce::dontSendNotification);
      playButton.setEnabled(audioProcessor.transportOriginal.isFileLoaded());
      if (isPlaying)
      {
        playButton.setButtonText("S");
        playButton.setColour(juce::TextButton::buttonColourId,
                             juce::Colours::red);
      } else 
      {
        playButton.setButtonText("P");
        playButton.setColour(juce::TextButton::buttonColourId,
                             juce::Colours::green);
      }
      break;
    case 1:  // Separated
      //playButton2.setToggleState(isPlaying, juce::dontSendNotification);
      if (isPlaying) {
        playButton2.setButtonText("S");
        playButton2.setColour(juce::TextButton::buttonColourId,
                             juce::Colours::red);
      } else {
        playButton2.setButtonText("P");
        playButton2.setColour(juce::TextButton::buttonColourId,
                             juce::Colours::green);
      }
      break;
  }
}


//void AudioPluginAudioProcessorEditor::updateTransportButtons(bool status) {
//  // DBG("qualcosa è cambiato");
//
//  playButton.setEnabled(audioProcessor.transportOriginal.isFileLoaded());
//  divideButton.setEnabled(audioProcessor.transportOriginal.isFileLoaded());
//  playButton2.setEnabled(audioProcessor.transportSeparation.isFileLoaded());
//}


void AudioPluginAudioProcessorEditor::timerCallback() {
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

// In PluginEditor.cpp

void AudioPluginAudioProcessorEditor::mouseDrag(const juce::MouseEvent& event) {
  // Controlla se stiamo già trascinando per evitare di iniziare più volte
  if (isDraggingTrack)
    return;

  // Identifica su quale componente è iniziato il clic originale
  juce::Component* sourceComponent = event.originalComponent;

  // Controlla se il componente sorgente è uno dei nostri pulsanti 'tracks'
  for (int i = 0; i < tracks.size(); ++i) {
    if (sourceComponent == tracks[i].get()) {
      // Verifica che ci sia stato un movimento sufficiente per essere
      // considerato un drag
      if (event.mouseWasDraggedSinceMouseDown()) {
        juce::String fullPath;
        // Accedi in modo sicuro all'array nel processore
        if (i >= 0 &&
            i < audioProcessor.transportSeparation.separationPaths.size()) {
          fullPath = audioProcessor.transportSeparation.separationPaths[i];
        } else {
          juce::Logger::writeToLog("Error: Invalid index " + juce::String(i) +
                                   " for separationNames.");
          return;  // Indice non valido, esci
        }
        
        juce::File fileToDrag(fullPath);          // Crea un oggetto F
        if (fileToDrag.existsAsFile())  // Controlla che il percorso
                                                  // sia valido e il file esista
        {
          juce::Logger::writeToLog("Starting drag for track " +
                                   juce::String(i) +
                                   " file: " + fileToDrag.getFullPathName());

          juce::StringArray filesToDrag;
          filesToDrag.add(fileToDrag.getFullPathName()); 

          // Imposta i flag per indicare che stiamo trascinando
          isDraggingTrack = true;
          draggedTrackIndex = i;  // Potrebbe servire per feedback visivo

          // Avvia l'operazione di drag-and-drop esterna
          // Il 'false' significa che il file non può essere mosso (solo copiato
          // dalla DAW/OS)
          performExternalDragDropOfFiles(filesToDrag, false, sourceComponent);

          // NOTA: L'esecuzione si blocca qui finché il drag non finisce.
          // Dopo il drag, dobbiamo resettare i flag. Lo facciamo in mouseUp.

        } else {
          juce::Logger::writeToLog("Cannot start drag ... " +
                                   fileToDrag.getFullPathName());
        }
      }
      return;  // Esci dal loop una volta trovato il pulsante sorgente
    }
  }

  // Se il drag non è iniziato su un pulsante 'tracks', puoi gestire altri drag
  // qui se necessario
}

// È buona norma resettare i flag in mouseUp
void AudioPluginAudioProcessorEditor::mouseUp(const juce::MouseEvent& event) {
  if (isDraggingTrack) {
    juce::Logger::writeToLog("Drag finished for track " +
                             juce::String(draggedTrackIndex));
    isDraggingTrack = false;
    draggedTrackIndex = -1;
    // Potresti voler ripristinare l'aspetto del pulsante qui se lo hai
    // modificato durante il drag
  }
  // Gestisci altri eventi mouseUp se necessario
}

void AudioPluginAudioProcessorEditor::setTrackButtons(int length) {
    tracks.clear();

    for (int i = 0; i < length; i++) {
        tracks.push_back(std::make_unique<juce::TextButton>(juce::String(i + 1)));
        tracks[i]->onClick = [this, i] {
          if (i < audioProcessor.transportSeparation.getSeparatedTracks().size()) {
            audioProcessor.transportSeparation.load(i);
          }
        };
        tracks[i]->setAlpha(0.3f);
        tracks[i]->setEnabled(true);
        tracks[i]->setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        second.addAndMakeVisible(tracks[i].get());
    }
    resized();
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
