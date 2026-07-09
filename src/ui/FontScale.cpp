#include "FontScale.h"
#include "lib/bmin/Map.h"
#include <algorithm>
#include <array>

namespace {

constexpr std::array<int, 15> kPresetFontSizes{
    10, 12, 14, 15, 16, 18, 20, 22, 24, 28, 32, 36, 48, 60, 72};

bmin::Map<int, bmin::DynArray<int>>& kOverrideFontSizes() {
  static bmin::Map<int, bmin::DynArray<int>> map = []() {
    bmin::Map<int, bmin::DynArray<int>> result;
    bmin::DynArray<int> sizes;
    for (int size : {10, 12, 14, 15, 16, 18, 20, 22, 24, 28, 32, 36, 48, 60, 72}) {
      sizes.pushBack(size);
    }
    result.insert(10, bmin::move(sizes));
    return result;
  }();
  return map;
}

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

  auto& overrideFontSizes = kOverrideFontSizes();
  auto overrideIt = overrideFontSizes.find(baseSizePx);
  if (overrideIt != overrideFontSizes.end()) {
    const auto& sizeSequence = (*overrideIt).value;
    for (size_t i = 0; i < sizeSequence.size(); ++i) {
      if (sizeSequence[i] == baseSizePx) {
        const int scaledIndex = std::clamp(
            static_cast<int>(i) + fontScale, 0, static_cast<int>(sizeSequence.size()) - 1);
        return static_cast<sdl2w::TextSize>(sizeSequence[static_cast<size_t>(scaledIndex)]);
      }
    }
  }

  int basePresetIndex = static_cast<int>(nearestPresetIndex(baseSizePx));
  int scaledPresetIndex = std::clamp(
      basePresetIndex + fontScale, 0, static_cast<int>(kPresetFontSizes.size()) - 1);
  return static_cast<sdl2w::TextSize>(kPresetFontSizes[scaledPresetIndex]);
}

} // namespace ui
