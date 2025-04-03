#include "SlothPlugin/PluginEditor.h"
#include "SlothPlugin/PluginProcessor.h"

namespace audio_plugin {
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p) {
  juce::ignoreUnused(processorRef);
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(400, 300);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  g.setColour(juce::Colours::white);
  g.setFont(15.0f);
  torch::jit::script::Module module;
  try {
    // Deserialize the ScriptModule from a file using torch::jit::load().
    module = torch::jit::load("C:/Users/franc/Documents/Polimi/Capstone/MaE-Capstone/codebase_L13_CAPSTONE2025/codebase_L13_CAPSTONE2025/mdx23c.pt");
    g.drawFittedText("Model loaded!", getLocalBounds(),
                     juce::Justification::centred, 1);
  } catch (const c10::Error& e) {
    g.drawFittedText("Model not loaded", getLocalBounds(),
                     juce::Justification::centred, 1);
  }
}

void AudioPluginAudioProcessorEditor::resized() {
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
}
}  // namespace audio_plugin
