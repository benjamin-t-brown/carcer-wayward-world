module;
#include <cstddef>
#include <cstdint>
#include <utility>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_pixels.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#endif

export module carcer.ui.elements:VerticalList;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.SdlPixels;
export import carcer.ui.UiElement;
import sdl2w;
#include "macros.h"

export {

// --- from ui/elements/VerticalList.h ---
// IWYU pragma: keep

namespace ui {

struct VerticalListProps {
  int width = 0;
  int lineHeight = 20;
  int lineGap = 2;
  SDL_Color bgColor = SDL_Color{255, 255, 255, 0};
};

// VerticalList element - renders an opinionated list of UiElements
class VerticalList : public UiElement {
private:
  VerticalListProps props;
  int selectedIndex = -1;

public:
  VerticalList(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~VerticalList() override = default;

  void setProps(const VerticalListProps& _props);
  VerticalListProps& getProps();
  const VerticalListProps& getProps() const;

  void setSelectedIndex(int index);
  int getSelectedIndex() const;
  void clearSelection();

  void addListItem(UiElement* item);
  void addListItems(const bmin::DynArray<UiElement*>& items);
  void removeListItemAtIndex(size_t index);

  const std::pair<int, int> getDims() const override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

VerticalList::VerticalList(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  build();
}

void VerticalList::setProps(const VerticalListProps& _props) {
  props = _props;
  build();
}

VerticalListProps& VerticalList::getProps() { return props; }

const VerticalListProps& VerticalList::getProps() const { return props; }

void VerticalList::setSelectedIndex(int index) {
  if (index >= -1 && index < static_cast<int>(children.size())) {
    selectedIndex = index;
  }
}

int VerticalList::getSelectedIndex() const { return selectedIndex; }

void VerticalList::clearSelection() { selectedIndex = -1; }

void VerticalList::addListItem(UiElement* item) { addChild(item); }

void VerticalList::addListItems(const bmin::DynArray<UiElement*>& items) {
  for (int i = 0; i < static_cast<int>(items.size()); i++) {
    addListItem(items[i]);
  }
}

void VerticalList::removeListItemAtIndex(size_t index) {
  if (index < children.size()) {
    children.erase(children.begin() + index);
  }
}

const std::pair<int, int> VerticalList::getDims() const {
  auto w = props.width > 0 ? props.width : style.width;
  auto h = (props.lineHeight + props.lineGap) * static_cast<int>(children.size());
  return {static_cast<int>(w * style.scale), static_cast<int>(h * style.scale)};
}

void VerticalList::build() {
  if (props.width > 0) {
    style.width = props.width;
  }
  for (size_t i = 0; i < children.size(); i++) {
    auto& child = children[i];
    child->setPos(style.x, style.y + (props.lineHeight + props.lineGap) * static_cast<int>(i));
    child->build();
  }
}

void VerticalList::render(int dt) { UiElement::render(dt); }

} // namespace ui
