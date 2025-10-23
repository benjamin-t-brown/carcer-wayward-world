#pragma once

#include <SDL2/SDL_pixels.h>
namespace ui {
struct Colors {
  static constexpr SDL_Color Transparent{0, 0, 0, 0};
  static constexpr SDL_Color Black{0, 0, 0, 255};
  static constexpr SDL_Color White{255, 255, 255, 255};
  static constexpr SDL_Color Red{255, 0, 0, 255};
  static constexpr SDL_Color Blue{0, 0, 255, 255};
  static constexpr SDL_Color ButtonModalGrey1{75, 75, 75, 255};
  static constexpr SDL_Color ButtonModalGrey2{100, 100, 100, 255};
  static constexpr SDL_Color ButtonModalGrey3{50, 50, 50, 255};
  static constexpr SDL_Color ButtonModalSelected{66, 202, 253, 255};
  static constexpr SDL_Color ButtonCloseRed{169, 59, 59, 255};
  static constexpr SDL_Color ButtonCloseRedHover{200, 70, 70, 255};
  static constexpr SDL_Color ButtonCloseRedActive{255, 0, 0, 255};
  static constexpr SDL_Color ButtonCloseTextWhite{255, 255, 255, 255};
  static constexpr SDL_Color ButtonCloseTextGrey{100, 100, 100, 255};
};
} // namespace ui