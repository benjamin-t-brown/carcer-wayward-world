#pragma once

#include <functional>
#include <optional>
#include <string_view>

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

  bool isBusy() const;
  void stop();
  void begin(std::function<bool*()> getter, std::function<void()> onComplete);
  void update(int deltaTime);
};

} // namespace ui
