module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <string_view>
#include <string>
#include <functional>

export module carcer.ui.KeyboardHeldScroll;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.elements;
import sdl2w;
#include "macros.h"

export {

// --- from ui/KeyboardHeldScroll.h ---
namespace ui {

enum class HeldScrollDirection { Up, Down };

// Hold-to-repeat keyboard scrolling for any SectionScrollable (or custom action).
// Wire from a Layer/Page: onKeyDown / onKeyUp / update. Timing matches world movement
// (first step immediately, 300ms pause, then every 50ms).
class KeyboardHeldScroll {
private:
  struct Binding {
    bmin::String key;
    std::function<void()> action;
  };

  struct Held {
    bool isActive = false;
    std::function<void()> action;
    bmin::String key;
    int dx = 0;
    int dy = 0;
    model::TimerStruct initialDelay = model::TimerStruct(300);
    model::TimerStruct moveDelay = model::TimerStruct(50);
  };

  const Binding* findBinding(std::string_view key) const;
  void applyHeldAction();

  bmin::DynArray<Binding> bindings;
  Held held;

public:
  void clearBindings();
  void stopScroll();

  // Bind a key to an arbitrary scroll/step action.
  void bindKey(std::string_view key, std::function<void()> action);

  // Bind a key to scrollUp/scrollDown on a section resolved each step (safe across UI
  // rebuilds).
  void bindSectionKey(std::string_view key,
                      std::function<SectionScrollable*()> sectionGetter,
                      HeldScrollDirection direction);

  // Returns true if the key matched a binding and was handled.
  bool onKeyDown(std::string_view key);
  void onKeyUp(std::string_view key);
  void update(int deltaTime, sdl2w::Window* window);
  bool isHolding() const { return held.isActive; }
};

} // namespace ui

} // export
