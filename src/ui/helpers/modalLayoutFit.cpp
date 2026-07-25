#include "modalLayoutFit.h"
#include "ui/UiElement.h"
#include <algorithm>

namespace ui {

static constexpr int portraitMarginPx = 12;
static constexpr float portraitMinAspect = 1.25f;
static constexpr int landscapeVerticalInset = 50;

static constexpr int standardLandscapeWidthCap = 500;
static constexpr int standardLandscapeHeightCap = 500;
static constexpr int smallLandscapeWidthCap = 400;
static constexpr int smallLandscapeHeightCap = 420;

LayoutRect computeCappedCenteredRect(int windowW,
                                     int windowH,
                                     ModalSizeClass sizeClass) {
  if (windowW <= 0 || windowH <= 0) {
    return {};
  }

  // Tall screens (phones / portrait tablets): nearly fill with a small inset.
  const auto aspect = static_cast<float>(windowH) / static_cast<float>(windowW);
  if (aspect >= portraitMinAspect) {
    const auto width = std::max(1, windowW - 2 * portraitMarginPx);
    const auto height = std::max(1, windowH - 2 * portraitMarginPx);
    return LayoutRect{
        .x = (windowW - width) / 2,
        .y = (windowH - height) / 2,
        .width = width,
        .height = height,
    };
  }

  const int widthCap = sizeClass == ModalSizeClass::Small ? smallLandscapeWidthCap
                                                          : standardLandscapeWidthCap;
  const int heightCap = sizeClass == ModalSizeClass::Small ? smallLandscapeHeightCap
                                                           : standardLandscapeHeightCap;

  // Landscape (incl. 4:3): capped box, centered.
  const auto width = std::min(widthCap, windowW);
  const auto height = std::min(heightCap, windowH - landscapeVerticalInset);
  return LayoutRect{
      .x = (windowW - width) / 2,
      .y = (windowH - height) / 2,
      .width = width,
      .height = std::max(1, height),
  };
}

void syncHostStyleToCappedCentered(BaseStyle& style, ModalSizeClass sizeClass) {
  const auto rect = computeCappedCenteredRect(style.width, style.height, sizeClass);
  style.x = rect.x;
  style.y = rect.y;
  style.width = rect.width;
  style.height = rect.height;
}

} // namespace ui
