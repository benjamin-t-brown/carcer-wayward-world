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

export module carcer.ui.elements:HorizontalList;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.core;
import sdl2w;
#include "macros.h"

export {

// --- from ui/elements/HorizontalList.h ---
// IWYU pragma: keep

namespace ui {

struct HorizontalListProps {
  int height = 0;
  int lineWidth = 20;
  int lineGap = 2;
  SDL_Color bgColor = SDL_Color{255, 255, 255, 0};
};

// HorizontalList element - lays out child UiElements in a row.
class HorizontalList : public UiElement {
private:
  HorizontalListProps props;
  int selectedIndex = -1;

public:
  HorizontalList(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~HorizontalList() override = default;

  void setProps(const HorizontalListProps& _props);
  HorizontalListProps& getProps();
  const HorizontalListProps& getProps() const;

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

HorizontalList::HorizontalList(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  build();
}

void HorizontalList::setProps(const HorizontalListProps& _props) {
  props = _props;
  build();
}

HorizontalListProps& HorizontalList::getProps() { return props; }

const HorizontalListProps& HorizontalList::getProps() const { return props; }

void HorizontalList::setSelectedIndex(int index) {
  if (index >= -1 && index < static_cast<int>(children.size())) {
    selectedIndex = index;
  }
}

int HorizontalList::getSelectedIndex() const { return selectedIndex; }

void HorizontalList::clearSelection() { selectedIndex = -1; }

void HorizontalList::addListItem(UiElement* item) { addChild(item); }

void HorizontalList::addListItems(const bmin::DynArray<UiElement*>& items) {
  for (auto* item : items) {
    addListItem(item);
  }
}

void HorizontalList::removeListItemAtIndex(size_t index) {
  if (index < children.size()) {
    children.erase(children.begin() + index);
  }
}

const std::pair<int, int> HorizontalList::getDims() const {
  auto w = (props.lineWidth + props.lineGap) * static_cast<int>(children.size());
  auto h = props.height > 0 ? props.height : style.height;
  return {static_cast<int>(w * style.scale), static_cast<int>(h * style.scale)};
}

void HorizontalList::build() {
  if (props.height > 0) {
    style.height = props.height;
  }
  for (size_t i = 0; i < children.size(); i++) {
    auto& child = children[i];
    child->setPos(style.x + (props.lineWidth + props.lineGap) * static_cast<int>(i),
                  style.y);
    child->build();
  }
}

void HorizontalList::render(int dt) { UiElement::render(dt); }

} // namespace ui
