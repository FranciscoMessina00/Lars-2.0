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
           p.getFileSampleRate(),
           p.transportOriginal.fileName, 
           p.transportOriginal),
      separation(
          p,
            p.transportSeparation.getWaveform(),
            p.transportSeparation.getSampleCount(),
            p.getFileSampleRate(),
            p.transportSeparation.fileName,
            p.transportSeparation),  // Qui andrebbe messo filename2 ma
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
      audioProcessor.process();
      playButton2.setEnabled(true);
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
  second.addAndMakeVisible(separation);
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
  
  //full.setBounds(area.removeFromTop(area.getHeight() / 2));
  //second.setBounds(area); 
  
  //juce::Logger::writeToLog(second.getBounds().toString());
  original.setBounds(full.getLocalBounds());
  separation.setBounds(second.getLocalBounds());
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

void AudioPluginAudioProcessorEditor::updateTransportButtons(int sourceIndex, bool isPlaying) {
  switch (sourceIndex) {
    case 0:  // Original
      //playButton.setToggleState(isPlaying, juce::dontSendNotification);
      
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
