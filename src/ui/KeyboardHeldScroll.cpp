#include "KeyboardHeldScroll.h"
#include "sdl2w/Window.h"

namespace ui {

void KeyboardHeldScroll::clearBindings() {
  clearHeld();
  bindings.clear();
}

void KeyboardHeldScroll::bindKey(std::string_view key, std::function<void()> action) {
  if (key.empty() || !action) {
    return;
  }
  bindings.pushBack(Binding{
      .key = bmin::String(key.data(), key.size()),
      .action = std::move(action),
  });
}

void KeyboardHeldScroll::bindSectionKey(std::string_view key,
                                        std::function<SectionScrollable*()> sectionGetter,
                                        ScrollDirection direction) {
  if (!sectionGetter) {
    return;
  }
  bindKey(key, [sectionGetter = std::move(sectionGetter), direction]() {
    auto* section = sectionGetter();
    if (!section) {
      return;
    }
    if (direction == ScrollDirection::Up) {
      section->scrollUp();
    } else {
      section->scrollDown();
    }
  });
}

const KeyboardHeldScroll::Binding*
KeyboardHeldScroll::findBinding(std::string_view key) const {
  for (const auto& binding : bindings) {
    if (std::string_view(binding.key.cStr(), binding.key.size()) == key) {
      return &binding;
    }
  }
  return nullptr;
}

void KeyboardHeldScroll::applyHeldAction() {
  if (held && held->action) {
    held->action();
  }
}

bool KeyboardHeldScroll::onKeyDown(std::string_view key) {
  const Binding* binding = findBinding(key);
  if (!binding) {
    return false;
  }

  // Ignore OS/SDL key-repeat; we time repeats ourselves.
  if (held && std::string_view(held->key.cStr(), held->key.size()) == key) {
    return true;
  }

  held = Held{
      .key = binding->key,
      .action = binding->action,
      .heldMs = 0,
      .repeating = false,
  };
  applyHeldAction();
  return true;
}

void KeyboardHeldScroll::onKeyUp(std::string_view key) {
  if (held && std::string_view(held->key.cStr(), held->key.size()) == key) {
    clearHeld();
  }
}

void KeyboardHeldScroll::clearHeld() {
  held.reset();
}

void KeyboardHeldScroll::update(int deltaTime, sdl2w::Window* window) {
  if (!held || deltaTime <= 0) {
    return;
  }

  if (window &&
      !window->getEvents().isKeyPressed(
          std::string_view(held->key.cStr(), held->key.size()))) {
    clearHeld();
    return;
  }

  held->heldMs += deltaTime;

  if (!held->repeating) {
    if (held->heldMs < kInitialDelayMs) {
      return;
    }
    held->repeating = true;
    held->heldMs -= kInitialDelayMs;
    applyHeldAction();
  }

  // At most one step per frame so a hitch doesn't dump many scrolls.
  if (held->repeating && held->heldMs >= kRepeatIntervalMs) {
    held->heldMs -= kRepeatIntervalMs;
    applyHeldAction();
  }
}

} // namespace ui
