#include "VerticalList.h"

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

void VerticalList::addListItems(const std::vector<UiElement*>& items) {
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
  auto w = style.width;
  auto h = (props.lineHeight + props.lineGap) * static_cast<int>(children.size());
  return {static_cast<int>(w * style.scale), static_cast<int>(h * style.scale)};
}

void VerticalList::build() {
  for (size_t i = 0; i < children.size(); i++) {
    auto& child = children[i];
    auto& s = child->getStyle();
    s.x = style.x;
    s.y = style.y + (props.lineHeight + props.lineGap) * i;
    child->build();
  }
}

void VerticalList::render(int dt) {
  // auto& draw = window->getDraw();
  // auto [scaledWidth, scaledHeight] = getDims();
  // draw.drawRect(style.x, style.y, scaledWidth, scaledHeight, props.bgColor);

  // Render children
  UiElement::render(dt);
}

} // namespace ui
