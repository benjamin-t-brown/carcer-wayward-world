#include "KeyboardPressFlash.h"

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

bool KeyboardPressFlash::isBusy() const { return remainingMs > 0; }

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
                               std::function<void()> onComplete) {
  if (isBusy() || !onComplete) {
    return;
  }
  activeFlagGetter = std::move(getter);
  pendingComplete = std::move(onComplete);
  remainingMs = durationMs;
  setActive(true);
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
