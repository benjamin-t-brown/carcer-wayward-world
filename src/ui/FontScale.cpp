#include "FontScale.h"
#include <algorithm>
#include <array>
#include <unordered_map>
#include <vector>

namespace {

constexpr std::array<int, 15> kPresetFontSizes{
    10, 12, 14, 15, 16, 18, 20, 22, 24, 28, 32, 36, 48, 60, 72};

const std::unordered_map<int, std::vector<int>> kOverrideFontSizes{
    // Keep overrides constrained to loaded/preset sizes that sdl2w can resolve.
    {10, {10, 12, 14, 15, 16, 18, 20, 22, 24, 28, 32, 36, 48, 60, 72}},
};

int clampToKnownBounds(int value) {
  return std::clamp(value, kPresetFontSizes.front(), kPresetFontSizes.back());
}

size_t nearestPresetIndex(int sizePx) {
  int clampedSizePx = clampToKnownBounds(sizePx);
  size_t nearestIdx = 0;
  int nearestDistance = std::abs(kPresetFontSizes[0] - clampedSizePx);
  for (size_t i = 1; i < kPresetFontSizes.size(); i++) {
    int distance = std::abs(kPresetFontSizes[i] - clampedSizePx);
    if (distance < nearestDistance) {
      nearestIdx = i;
      nearestDistance = distance;
    }
  }
  return nearestIdx;
}

} // namespace

namespace ui {

int mapFontSizeToPixels(sdl2w::TextSize size) { return static_cast<int>(size); }

sdl2w::TextSize mapPixelsToFontSize(int sizePx) {
  auto nearest = nearestPresetIndex(sizePx);
  return static_cast<sdl2w::TextSize>(kPresetFontSizes[nearest]);
}

sdl2w::TextSize applyFontScale(sdl2w::TextSize baseSize, int fontScale) {
  int baseSizePx = mapFontSizeToPixels(baseSize);

  auto overrideIt = kOverrideFontSizes.find(baseSizePx);
  if (overrideIt != kOverrideFontSizes.end()) {
    const auto& sizeSequence = overrideIt->second;
    auto baseIt = std::find(sizeSequence.begin(), sizeSequence.end(), baseSizePx);
    if (baseIt != sizeSequence.end()) {
      int baseIndex = static_cast<int>(baseIt - sizeSequence.begin());
      int scaledIndex = std::clamp(
          baseIndex + fontScale, 0, static_cast<int>(sizeSequence.size()) - 1);
      return static_cast<sdl2w::TextSize>(sizeSequence[scaledIndex]);
    }
  }

  int basePresetIndex = static_cast<int>(nearestPresetIndex(baseSizePx));
  int scaledPresetIndex = std::clamp(
      basePresetIndex + fontScale, 0, static_cast<int>(kPresetFontSizes.size()) - 1);
  return static_cast<sdl2w::TextSize>(kPresetFontSizes[scaledPresetIndex]);
}

} // namespace ui
