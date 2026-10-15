#include "KeyboardPressFlash.h"
#include "ui/helpers/uiSounds.h"

namespace ui {

bool KeyboardPressFlash::isConfirmKey(std::string_view key) {
  return key == "Return" || key == "Keypad Enter" || key == "space";
}

std::optional<int> KeyboardPressFlash::choiceIndexFromKey(std::string_view key) {
  if (key.size() != 1 || key[0] < '1' || key[0] > '9') {
    return std::nullopt;
  }
  return static_cast<int>(key[0] - '1');
}

bool KeyboardPressFlash::isBusy() const { return static_cast<bool>(pendingComplete); }

void KeyboardPressFlash::setActive(bool active) {
  if (!activeFlagGetter) {
    return;
  }
  if (auto* flag = activeFlagGetter()) {
    *flag = active;
  }
}

void KeyboardPressFlash::stop() {
  setActive(false);
  remainingMs = 0;
  pendingComplete = {};
  activeFlagGetter = {};
}

void KeyboardPressFlash::begin(std::function<bool*()> getter,
                               std::function<void()> onComplete,
                               sdl2w::Window* window,
                               const char* soundName,
                               bool deferAction) {
  if (isBusy() || !onComplete) {
    return;
  }
  // Drop any leftover visual-only highlight before starting a new flash.
  stop();
  playUiSound(window, soundName != nullptr ? soundName : kButtonSoundName);
  activeFlagGetter = std::move(getter);
  remainingMs = durationMs;
  setActive(true);
  if (deferAction) {
    pendingComplete = std::move(onComplete);
  } else {
    onComplete();
  }
}

void KeyboardPressFlash::update(int deltaTime) {
  if (remainingMs <= 0) {
    return;
  }
  remainingMs -= deltaTime;
  if (remainingMs > 0) {
    return;
  }
  remainingMs = 0;
  setActive(false);
  auto complete = std::move(pendingComplete);
  activeFlagGetter = {};
  pendingComplete = {};
  if (complete) {
    complete();
  }
}

} // namespace ui
