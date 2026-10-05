#pragma once

#include "sdl2w/Window.h"

namespace ui {

inline constexpr const char* kButtonSoundName = "button";
inline constexpr const char* kButton2SoundName = "button2";
inline constexpr const char* kButtonGetItemSoundName = "button_get_item";
inline constexpr const char* kButtonPageTurnSoundName = "button_page_turn";

// Immediate UI clip. Null window is a no-op so headless tests stay quiet.
inline void playUiSound(sdl2w::Window* window, const char* soundName) {
  if (window == nullptr || soundName == nullptr) {
    return;
  }
  window->playSound(soundName);
}

inline void playButtonSound(sdl2w::Window* window) {
  playUiSound(window, kButtonSoundName);
}

inline void playButtonGetItemSound(sdl2w::Window* window) {
  playUiSound(window, kButtonGetItemSoundName);
}

inline void playButtonPageTurnSound(sdl2w::Window* window) {
  playUiSound(window, kButtonPageTurnSoundName);
}

} // namespace ui
