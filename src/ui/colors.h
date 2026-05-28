#pragma once

#include "SdlPixels.h" // IWYU pragma: keep

namespace ui {
struct Colors {
  static constexpr SDL_Color Transparent{0, 0, 0, 0};
  static constexpr SDL_Color Black{0, 0, 0, 255};
  static constexpr SDL_Color White{255, 255, 255, 255};
  static constexpr SDL_Color OffWhite{248, 248, 248, 255};
  static constexpr SDL_Color LightGrey{188, 183, 197, 255};
  static constexpr SDL_Color DarkGrey{75, 75, 75, 255};
  static constexpr SDL_Color Grey{128, 128, 128, 255};
  static constexpr SDL_Color Red{225, 83, 74, 255};
  static constexpr SDL_Color Blue{57, 120, 168, 255};
  static constexpr SDL_Color LightBlue{66, 202, 253, 255};
  static constexpr SDL_Color DarkBlue{36, 63, 114, 255};
  static constexpr SDL_Color Green{57, 120, 68, 255};
  static constexpr SDL_Color ButtonModalGrey1{75, 75, 75, 255};
  static constexpr SDL_Color ButtonModalGrey2{100, 100, 100, 255};
  static constexpr SDL_Color ButtonModalGrey3{50, 50, 50, 255};
  static constexpr SDL_Color ButtonModalSelected{66, 202, 253, 255};
  static constexpr SDL_Color ButtonCloseRed{169, 59, 59, 255};
  static constexpr SDL_Color ButtonCloseRedBorder1{255, 139, 156, 255};
  static constexpr SDL_Color ButtonCloseRedBorder2{94, 54, 67, 255};
  static constexpr SDL_Color ButtonCloseRedHover{200, 70, 70, 255};
  static constexpr SDL_Color ButtonCloseRedActive{255, 0, 0, 255};
  static constexpr SDL_Color ButtonCloseTextWhite{255, 255, 255, 255};
  static constexpr SDL_Color ButtonCloseTextGrey{60, 60, 60, 255};
  static constexpr SDL_Color BorderModalStandard{45, 55, 64, 255};
  static constexpr SDL_Color BorderModalStandardLight{91, 109, 127, 255};
  static constexpr SDL_Color BorderModalStandardDark{23, 28, 33, 255};
  static constexpr SDL_Color ModalStandardBackground{248, 248, 248, 255};
  static constexpr SDL_Color ModalHeaderBackground{188, 183, 197, 255};
  static constexpr SDL_Color ButtonWorldActionDefault{100, 120, 140, 255};
  static constexpr SDL_Color ButtonWorldActionHover{120, 140, 160, 255};
  static constexpr SDL_Color ButtonWorldActionActive{80, 100, 120, 255};
  static constexpr SDL_Color ButtonWorldActionText{255, 255, 255, 255};
};
} // namespace ui