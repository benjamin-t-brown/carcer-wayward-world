#pragma once

#include <functional>
#include <optional>
#include <string_view>

namespace sdl2w {
class Window;
}

namespace ui {

// Timed keyboard press highlight. After durationMs, invokes onComplete so the
// owner can enqueue the same continue/choice action as a mouse click.
class KeyboardPressFlash {
private:
  int remainingMs = 0;
  std::function<bool*()> activeFlagGetter;
  std::function<void()> pendingComplete;

  void setActive(bool active);

public:
  static constexpr int durationMs = 120;

  static bool isConfirmKey(std::string_view key);
  static std::optional<int> choiceIndexFromKey(std::string_view key);

  // True while a deferred onComplete is waiting (blocks further shortcuts).
  // Visual-only flashes (deferAction=false) do not count as busy.
  bool isBusy() const;
  void stop();
  // soundName null plays the regular button clip.
  // deferAction=false runs onComplete immediately (mode/UI activates now) while
  // the pressed highlight still lasts durationMs.
  void begin(std::function<bool*()> getter,
             std::function<void()> onComplete,
             sdl2w::Window* window = nullptr,
             const char* soundName = nullptr,
             bool deferAction = true);
  void update(int deltaTime);
};

} // namespace ui
