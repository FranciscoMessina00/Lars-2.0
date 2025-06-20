#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>

class RoundedCornersEffect : public juce::ImageEffectFilter {
public:
  RoundedCornersEffect();

  explicit RoundedCornersEffect(const float& radius,
                                const bool& roundTopLeftCorner = true,
                                const bool& roundTopRightCorner = true,
                                const bool& roundBottomLeftCorner = true,
                                const bool& roundBottomRightCorner = true);

  ~RoundedCornersEffect() override;

  void setCornerRadius(const float& radius,
                       const bool& roundTopLeftCorner = true,
                       const bool& roundTopRightCorner = true,
                       const bool& roundBottomLeftCorner = true,
                       const bool& roundBottomRightCorner = true);

  void setClipReductionPath(const juce::Path& clipPath);

  void applyEffect(juce::Image& sourceImage,
                   juce::Graphics& destContext,
                   float scaleFactor,
                   float alpha) override;

private:
  float cornerRadius;

  bool topLeftCorner;
  bool topRightCorner;
  bool bottomLeftCorner;
  bool bottomRightCorner;

  juce::Path clipReductionPath;
  juce::Path scaledClipReductionPath;

  bool isClipReductionPathProvided;

  //==============================================================================

  JUCE_LEAK_DETECTOR(RoundedCornersEffect)
};