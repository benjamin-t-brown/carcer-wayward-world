#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "ui/elements/SectionScrollable.h"
#include <functional>
#include <optional>
#include <string_view>

namespace sdl2w {
class Window;
}

namespace ui {

enum class ScrollDirection { Up, Down };

// Hold-to-repeat keyboard scrolling for any SectionScrollable (or custom action).
// Wire from a Layer/Page: onKeyDown / onKeyUp / update. Timing matches world movement
// (first step immediately, 300ms pause, then every 50ms).
class KeyboardHeldScroll {
public:
  static constexpr int kInitialDelayMs = 300;
  static constexpr int kRepeatIntervalMs = 50;

  void clearBindings();

  // Bind a key to an arbitrary scroll/step action.
  void bindKey(std::string_view key, std::function<void()> action);

  // Bind a key to scrollUp/scrollDown on a section resolved each step (safe across UI rebuilds).
  void bindSectionKey(std::string_view key,
                      std::function<SectionScrollable*()> sectionGetter,
                      ScrollDirection direction);

  // Returns true if the key matched a binding and was handled.
  bool onKeyDown(std::string_view key);
  void onKeyUp(std::string_view key);
  void update(int deltaTime, sdl2w::Window* window);
  void clearHeld();
  bool isHolding() const { return held.has_value(); }

private:
  struct Binding {
    bmin::String key;
    std::function<void()> action;
  };

  struct Held {
    bmin::String key;
    std::function<void()> action;
    int heldMs = 0;
    bool repeating = false;
  };

  const Binding* findBinding(std::string_view key) const;
  void applyHeldAction();

  bmin::DynArray<Binding> bindings;
  std::optional<Held> held;
};

} // namespace ui
