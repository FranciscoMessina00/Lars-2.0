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
  playButton = std::make_unique<juce::DrawableButton>("playButton", juce::DrawableButton::ImageFitted);
  setSvgButton("play_button.svg", playButton.get());
  playButton->setTooltip("Click to Play/Stop");
  playButton->setAlpha(0.3f);

  loadButton = std::make_unique<juce::DrawableButton>("loadButton", juce::DrawableButton::ImageFitted);
  setSvgButton("upload_button.svg", loadButton.get());
  loadButton->setTooltip("Click to Load Audio File");
  loadButton->setAlpha(0.3f);
  
  divideButton = std::make_unique<juce::ImageButton>("divideButton");
  setImageButton("divide_image.png", divideButton.get(), false);
  divideButton->setTooltip("Click to Separate Tracks");
  divideButton->setAlpha(0.7f);
  
  playButton2 = std::make_unique<juce::DrawableButton>("playButton2", juce::DrawableButton::ImageFitted);
  setSvgButton("play_button.svg", playButton2.get());
  playButton2->setTooltip("Click to Play/Stop Separated Track");
  playButton2->setAlpha(0.3f);
  
  saveButton = std::make_unique<juce::DrawableButton>("saveButton", juce::DrawableButton::ImageFitted);
  setSvgButton("download_button.svg", saveButton.get());
  saveButton->setTooltip("Click to Save Separated Tracks");
  saveButton->setAlpha(0.3f);

  juce::StringArray items;
  items.add("Mdx23c - Small");
  items.add("Mdx23c - Large");
  items.add("Mdx23c - Music drum sep");
  trackSelector.addItemList(items, 1);
  trackSelector.setSelectedId(1, juce::dontSendNotification);
  trackSelector.setJustificationType(juce::Justification::centred);
  trackSelector.onChange = [&]() {
    int selectedIndex = trackSelector.getSelectedItemIndex();
    juce::Logger::writeToLog("Selected index: " + juce::String(selectedIndex));
    
    switch (selectedIndex) {
    case 0:
        audioProcessor.modelName = "mdx23c.pt";
        juce::Logger::writeToLog("Selected model: mdx23c.pt");
        break;
    case 1:
        audioProcessor.modelName = "mdx23c_capstone.pt";
        juce::Logger::writeToLog("Selected model: mdx23c_capstone.pt");
        break;
    case 2:
        audioProcessor.modelName = "mdx23c_inst_sep.pt";
        juce::Logger::writeToLog("Selected model: mdx23c_inst_sep.pt");
        break;
    default:
        audioProcessor.modelName = "mdx23c.pt";
        break;
    }
    
  };
  trackSelector.setTooltip("Select the model to use for separation");
  trackSelector.setColour(juce::ComboBox::backgroundColourId,
                          juce::Colours::black);

  loadButton->onClick = [&]() {
    if (audioProcessor.transportOriginal.load()) {
      audioProcessor.transportSeparation.reset();
      playButton2->setEnabled(false);
      separation.repaint();
      selectedTrack = -1;  // Reset selected track when loading a new file
      refreshTrackButtons();
    };
    //updateTransportButtons(audioProcessor.params.playButtonParam->get());
    playButton->setEnabled(audioProcessor.transportOriginal.isFileLoaded());
    divideButton->setEnabled(audioProcessor.transportOriginal.isFileLoaded());
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

  playButton->onClick = [&]() {
    updateTransportButtons(0, audioProcessor.params.playButtonParam->get());
    audioProcessor.params.playButtonParam->get()
        ? audioProcessor.transportOriginal.stopFile()
        : (
           audioProcessor.transportOriginal.playFile(),
           audioProcessor.transportSeparation.stopFile()
          );
  };

  playButton2->onClick = [&]() {
    updateTransportButtons(1, audioProcessor.params.playButton2Param->get());
    audioProcessor.params.playButton2Param->get()
        ? audioProcessor.transportSeparation.stopFile()
        : (
           audioProcessor.transportSeparation.playFile(),
           audioProcessor.transportOriginal.stopFile()
          );
  };



  divideButton->onClick = [&]() {
      original.setMouseCursor(juce::MouseCursor::WaitCursor);
      separation.setMouseCursor(juce::MouseCursor::WaitCursor);
      loadButton->setEnabled(false);
      playButton2->setEnabled(false);
      divideButton->setEnabled(false);
      saveButton->setEnabled(false);
      audioProcessor.transportSeparation.reset();
      selectedTrack = -1;  // Reset selected track when starting separation
      refreshTrackButtons();
      audioProcessor.process();
  };

  saveButton->onClick = [&]() {
    juce::FileChooser fileChooser("Select a folder to save separated tracks");

    if (fileChooser.browseForDirectory()) {
      auto selectedFolder = fileChooser.getResult();

      // Chiamata al processor per salvare
      audioProcessor.saveSeparatedTracks(selectedFolder);
    }
  };

  /*top.setOpaque(false);
  top.setVisible(true);
  top.setInterceptsMouseClicks(false, true);*/
  
  full.setComponentEffect(rounded);
  second.setComponentEffect(rounded);
  addAndMakeVisible(top);
  addAndMakeVisible(full);
  addAndMakeVisible(second);

  playButton->setColour(juce::TextButton::buttonColourId, juce::Colours::green);
  playButton->setEnabled(audioProcessor.transportOriginal.isFileLoaded());
  playButton2->setColour(juce::TextButton::buttonColourId,
                         juce::Colours::green);
  saveButton->setColour(juce::TextButton::buttonColourId, juce::Colours::yellow);
  playButton2->setEnabled(audioProcessor.transportSeparation.isFileLoaded());
  saveButton->setEnabled(audioProcessor.transportSeparation.isFileLoaded());
  divideButton->setEnabled(audioProcessor.transportOriginal.isFileLoaded());
  full.addAndMakeVisible(original);
  full.addAndMakeVisible(*loadButton);
  full.addAndMakeVisible(*playButton);
  full.addAndMakeVisible(*divideButton);
  top.addAndMakeVisible(trackSelector);
  second.addAndMakeVisible(separation);
  second.addAndMakeVisible(*playButton2);
  second.addAndMakeVisible(*saveButton);

  selectedTrack = -1;  // Inizializza il track selezionata a -1
  setTrackButtons(6);
  
  

  // setResizable(true, true); // Abilita il ridimensionamento manuale
  // setResizeLimits(800, 100, 2000, 1600); // Min: 400x300, Max: 2000x1600
  setSize(1000, 400);

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
  g.fillAll(backgroundColour);

  g.setColour(juce::Colour(0xFF0E0E0E));
  g.fillRoundedRectangle(full.getBounds().toFloat(), 10.0f);
  g.fillRoundedRectangle(second.getBounds().toFloat(), 10.0f);
}

void AudioPluginAudioProcessorEditor::resized() {

  const int w = getWidth();
  const int h = getHeight();
  const int m = 10.f;
  const int unitH = h / 7;
  const int boxH = 3 * unitH;

  top.setBounds(0, 0, w, h);

  full.setBounds(m, unitH, w - 2 * m, boxH - m);
  second.setBounds(m, full.getBottom() + m, w - 2 * m, boxH - m);
  
  //full.setBounds(area.removeFromTop(area.getHeight() / 2));
  //second.setBounds(area); 
  
  //juce::Logger::writeToLog(second.getBounds().toString());
  original.setBounds(full.getLocalBounds());
  separation.setBounds(second.getLocalBounds());
  loadButton->setBounds(full.getWidth() / 2 - buttonWidth, 5, buttonWidth, buttonHeight);
  playButton->setBounds(full.getWidth() / 2 + buttonWidth, 5, buttonWidth, buttonHeight);
  divideButton->setBounds(full.getWidth() / 2, 5, buttonWidth, buttonHeight);
  trackSelector.setBounds((getWidth() / 2) - (getWidth() / 8),
                          (getHeight() / 7) / 4, getWidth() / 4,
                          (getHeight() / 7) / 2);
  playButton2->setBounds(second.getWidth() / 2 + (buttonWidth / 2), 5,
                         buttonWidth, buttonHeight);
  saveButton->setBounds(second.getWidth() / 2 - (buttonWidth / 2), 5, buttonWidth,
                        buttonHeight);
  //tracks[tracks.size() - 1]->setBounds(getWidth() - buttonWidth - 15, 5, buttonWidth, buttonHeight);
  for (int i = tracks.size() - 1; i >= 0; --i) {
    tracks[i]->setBounds(
        second.getWidth() - 15 - (buttonWidth + 5) * (tracks.size() - i), 5,
        buttonWidth, buttonHeight);
  }
  repaint();
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
        loadButton->setEnabled(true);
        playButton2->setEnabled(true);
        saveButton->setEnabled(true);
        divideButton->setEnabled(true);
        original.setMouseCursor(juce::MouseCursor::NormalCursor);
        separation.setMouseCursor(juce::MouseCursor::NormalCursor);
        setTrackButtons(audioProcessor.transportSeparation.separations.size());
        selectedTrack = 0;
        refreshTrackButtons();
    }
}

void AudioPluginAudioProcessorEditor::updateTransportButtons(int sourceIndex, bool isPlaying) {
  switch (sourceIndex) {
    case 0:  // Original
      //playButton.setToggleState(isPlaying, juce::dontSendNotification);
      playButton->setEnabled(audioProcessor.transportOriginal.isFileLoaded());
      divideButton->setEnabled(audioProcessor.transportOriginal.isFileLoaded());
      if (isPlaying)
      {
        playButton->setButtonText("S");
        playButton->setColour(juce::TextButton::buttonColourId,
                             juce::Colours::red);
        setSvgButton("pause_button.svg", playButton.get());
      } else 
      {
        playButton->setButtonText("P");
        playButton->setColour(juce::TextButton::buttonColourId,
                             juce::Colours::green);
        setSvgButton("play_button.svg", playButton.get());
      }
      break;
    case 1:  // Separated
      //playButton2.setToggleState(isPlaying, juce::dontSendNotification);
      if (isPlaying) {
        playButton2->setButtonText("S");
        playButton2->setColour(juce::TextButton::buttonColourId,
                             juce::Colours::red);
        setSvgButton("pause_button.svg", playButton2.get());
      } else {
        playButton2->setButtonText("P");
        playButton2->setColour(juce::TextButton::buttonColourId,
                             juce::Colours::green);
        setSvgButton("play_button.svg", playButton2.get());
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
    if (event.eventComponent == loadButton.get() ||
        event.eventComponent == playButton.get() ||
        event.eventComponent == playButton2.get() ||
        event.eventComponent == saveButton.get()) 
    {
        event.eventComponent->setAlpha(1.0f);
        event.eventComponent->repaint();
    } else if (event.eventComponent == divideButton.get()) {
      event.eventComponent->setAlpha(0.8f);
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
    if (event.eventComponent == loadButton.get() ||
        event.eventComponent == playButton.get() ||
        event.eventComponent == playButton2.get() ||
        event.eventComponent == saveButton.get()) 
    {
        event.eventComponent->setAlpha(0.3f);
        event.eventComponent->repaint();
    } else if (event.eventComponent == divideButton.get()) {
      event.eventComponent->setAlpha(0.7f);
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
          try {
            performExternalDragDropOfFiles(filesToDrag, false, sourceComponent);
          } catch (const std::exception& e) {
            juce::Logger::writeToLog("Drag failed: " + juce::String(e.what()));
          }

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

void AudioPluginAudioProcessorEditor::refreshTrackButtons() {
  // we assume `tracks`, `names`, and `length` are still valid here,
  // so you might want to also store `names` and `trackCount` as members.

  for (int j = 0; j < (int)tracks.size(); ++j) {
    // figure out the correct filename for this button:
    const auto& nm = names[j];
    juce::String file;
    if (nm == "Cymbals")
      file = "Crash.png";
    else
      file = nm + ".png";

    // invert only if this is the selected track:
    setImageButton(file, tracks[j].get(), (j == selectedTrack));
  }
}


void AudioPluginAudioProcessorEditor::setTrackButtons(int length) {
    tracks.clear();
    if (length == 5) {
      names = {"Kick", "Snare", "Toms", "Hi-Hat", "Cymbals"};
    }
    else if (length == 6) {
      names = {"Kick", "Snare", "Toms", "Hi-Hat", "Ride", "Crash"};
    }
    else {
        //Log error
      juce::Logger::writeToLog("Error: Invalid length for track buttons: " +
                               juce::String(length));
    
    }

    for (int i = 0; i < length; i++) {
        tracks.push_back(std::make_unique<juce::ImageButton>(names[i]));
        std::string fileName;
        if (names[i].compare("Cymbals") == 0) {
            fileName = "Crash.png"; 
        } else {
            fileName = names[i] + ".png";
        }
        setImageButton(fileName, tracks[i].get(), false);
        tracks[i]->setTooltip(names[i]);
        tracks[i]->onClick = [this, i] {

            if (i < audioProcessor.transportSeparation.getSeparatedTracks().size()) {
                selectedTrack = i;
                audioProcessor.transportSeparation.load(i); 
                refreshTrackButtons();
            }
        };
        tracks[i]->setAlpha(0.3f);
        tracks[i]->setEnabled(true);
        tracks[i]->setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        second.addAndMakeVisible(tracks[i].get());
    }
    resized();
}

void AudioPluginAudioProcessorEditor::setSvgButton(juce::String svg, juce::DrawableButton* button) {
  auto res = getSvgResource(svg);
  if (res.data == nullptr || res.dataSize <= 0) {
    juce::Logger::writeToLog("SVG resource not found: " + svg);
    return;
  }

  juce::MemoryInputStream svgStream(res.data, static_cast<size_t>(res.dataSize),
                                    false);

  juce::String svgXml = svgStream.readEntireStreamAsString();

  std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(svgXml));

  if (!xml) {
    juce::Logger::writeToLog("Failed to parse SVG: " + svg);
    return;
  }

  std::unique_ptr<juce::Drawable> drawable(juce::Drawable::createFromSVG(*xml));
  if (!drawable) {
    juce::Logger::writeToLog("createFromSVG failed: " + svg);
    return;
  }

  if (drawable != nullptr) {
    button->setImages(drawable.get());
    drawable.release();
  } else {
    // Couldn’t create a Drawable from the parsed XML
    juce::Logger::writeToLog("Error: createFromSVG() returned nullptr for ");
  }
}

void AudioPluginAudioProcessorEditor::setImageButton(
    juce::String image,
    juce::ImageButton* button,
    bool invertColours) {
    
  auto res = getImageResource(image);
  if (res.data == nullptr || res.dataSize <= 0) {
    juce::Logger::writeToLog("Resource not found: " + image);
      return;
  }
  
  juce::Image img = juce::ImageCache::getFromMemory(res.data, res.dataSize);
  
  if (img.isValid()) {
    
    img.duplicateIfShared();
    if (invertColours && img.isValid()) {
      for (int y = 0; y < img.getHeight(); ++y) {
        for (int x = 0; x < img.getWidth(); ++x) {
          auto c = img.getPixelAt(x, y);
          auto inverted =
              juce::Colour::fromRGBA(255 - c.getRed(), 255 - c.getGreen(),
                                     255 - c.getBlue(), c.getAlpha());
          img.setPixelAt(x, y, inverted);
        }
      }
    }
    button->setImages(false, true, true,
                      img, 0.5f,
                      juce::Colours::transparentBlack,
                      img,  // overImage
                      0.8f, juce::Colours::transparentWhite,
                      img,  // downImage
                      1.0f, juce::Colours::transparentWhite,
                      0.0f);
  } else {
    // Couldn’t create a Drawable from the parsed XML
    juce::Logger::writeToLog("Error: loding image returned nullptr");
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
