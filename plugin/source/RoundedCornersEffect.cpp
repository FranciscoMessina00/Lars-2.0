#include "SlothPlugin/RoundedCornersEffect.h"

RoundedCornersEffect::RoundedCornersEffect()
    : cornerRadius(5.f),
      topLeftCorner(true),
      topRightCorner(true),
      bottomLeftCorner(true),
      bottomRightCorner(true),
      isClipReductionPathProvided(false) {
  // default no arg ctor
}

RoundedCornersEffect::RoundedCornersEffect(const float& radius,
                                           const bool& roundTopLeftCorner,
                                           const bool& roundTopRightCorner,
                                           const bool& roundBottomLeftCorner,
                                           const bool& roundBottomRightCorner)
    : cornerRadius(radius),
      topLeftCorner(roundTopLeftCorner),
      topRightCorner(roundTopRightCorner),
      bottomLeftCorner(roundBottomLeftCorner),
      bottomRightCorner(roundBottomRightCorner),
      isClipReductionPathProvided(false) {
  // ctor with args
}

RoundedCornersEffect::~RoundedCornersEffect() = default;

void RoundedCornersEffect::setCornerRadius(const float& radius,
                                           const bool& roundTopLeftCorner,
                                           const bool& roundTopRightCorner,
                                           const bool& roundBottomLeftCorner,
                                           const bool& roundBottomRightCorner) {
  cornerRadius = radius;
  topLeftCorner = roundTopLeftCorner;
  topRightCorner = roundTopRightCorner;
  bottomLeftCorner = roundBottomLeftCorner;
  bottomRightCorner = roundBottomRightCorner;
}

void RoundedCornersEffect::setClipReductionPath(const juce::Path& clipPath) {
  clipReductionPath = clipPath;
  scaledClipReductionPath = clipPath;
  isClipReductionPathProvided = true;
}

void RoundedCornersEffect::applyEffect(juce::Image& image,
                                       juce::Graphics& g,
                                       float scaleFactor,
                                       float alpha) {
  if (isClipReductionPathProvided) {
    scaledClipReductionPath.clear();
    scaledClipReductionPath = clipReductionPath;
    scaledClipReductionPath.applyTransform(
        juce::AffineTransform::scale(scaleFactor));

    g.reduceClipRegion(scaledClipReductionPath,
                       juce::AffineTransform::scale(scaleFactor));
  }

  if (!isClipReductionPathProvided && cornerRadius > 0.f) {
    clipReductionPath.clear();
    clipReductionPath.addRoundedRectangle(
        image.getBounds().toFloat().getX(), image.getBounds().toFloat().getY(),
        image.getBounds().toFloat().getWidth(),
        image.getBounds().toFloat().getHeight(), cornerRadius, cornerRadius,
        topLeftCorner, topRightCorner, bottomLeftCorner, bottomRightCorner);

    scaledClipReductionPath = clipReductionPath;

    g.reduceClipRegion(scaledClipReductionPath);
  }

  g.setOpacity(alpha);
  g.drawImageAt(image, 0, 0);
}
