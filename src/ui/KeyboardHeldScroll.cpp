#include "KeyboardHeldScroll.h"
#include "model/templates/UtilityTypes.h"
#include "sdl2w/Window.h"

namespace ui {

void KeyboardHeldScroll::clearBindings() {
  held.isActive = false;
  bindings.clear();
}

void KeyboardHeldScroll::stopScroll() { held.isActive = false; }

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
  if (held.isActive && held.action) {
    held.action();
  }
}

bool KeyboardHeldScroll::onKeyDown(std::string_view key) {
  const Binding* binding = findBinding(key);
  if (!binding) {
    return false;
  }

  // Ignore OS/SDL key-repeat; we time repeats ourselves.
  if (held.isActive && held.key.sliceView() == key) {
    return true;
  }

  held.isActive = true;
  held.key = binding->key;
  held.action = binding->action;
  held.action();
  model::timerStructStart(held.initialDelay);
  model::timerStructStart(held.moveDelay);
  return true;
}

void KeyboardHeldScroll::onKeyUp(std::string_view key) {
  if (held.isActive && held.key.sliceView() == key) {
    held.isActive = false;
  }
}

void KeyboardHeldScroll::update(int deltaTime, sdl2w::Window* window) {
  if (!held.isActive || deltaTime <= 0) {
    return;
  }

  if (window && !window->getEvents().isKeyPressed(held.key.sliceView())) {
    held.isActive = false;
    return;
  }

  model::timerStructUpdate(held.initialDelay, deltaTime);
  model::timerStructUpdate(held.moveDelay, deltaTime);

  if (held.isActive && model::timerStructIsComplete(held.initialDelay)) {
    if (model::timerStructIsComplete(held.moveDelay)) {
      applyHeldAction();
      model::timerStructStart(held.moveDelay);
    }
  }
}

} // namespace ui
