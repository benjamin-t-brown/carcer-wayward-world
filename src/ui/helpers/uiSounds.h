#pragma once

#include "sdl2w/Window.h"

namespace ui {

inline constexpr const char* kButtonSoundName = "button";

// Immediate UI chrome click. Null window is a no-op so headless tests stay quiet.
inline void playButtonSound(sdl2w::Window* window) {
  if (window == nullptr) {
    return;
  }
  window->playSound(kButtonSoundName);
}

} // namespace ui
