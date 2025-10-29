#include "VerticalList.h"
#include "ui/colors.h"

namespace ui {

VerticalList::VerticalList(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  build();
}

ui::Quad* VerticalList::getQuad() {
  if (children.empty()) {
    return nullptr;
  }

  return dynamic_cast<ui::Quad*>(children[0].get());
}

const ui::Quad* VerticalList::getQuad() const {
  return dynamic_cast<const ui::Quad*>(children[0].get());
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

void VerticalList::addListItem(std::unique_ptr<UiElement> item) {
  Quad* quad = getQuad();
  if (quad == nullptr) {
    return;
  }

  quad->getChildren().push_back(std::move(item));
  auto child = quad->getChildren().back().get();
  auto style = child->getStyle();
  style.y = style.y + props.lineHeight + props.lineGap;
  style.x = style.x;
  style.width = style.width;
  style.height = props.lineHeight;
  child->setStyle(style);
  build();
}

void VerticalList::addListItems(const std::vector<UiElement*>& items) {
  Quad* quad = getQuad();
  if (quad == nullptr) {
    return;
  }

  for (int i = 0; i < static_cast<int>(items.size()); i++) {
    UiElement* item = items[i];
    auto listItemStyle = item->getStyle();
    listItemStyle.y = 0 + i * (props.lineHeight + props.lineGap);
    listItemStyle.x = 0;
    listItemStyle.width = style.width;
    listItemStyle.height = props.lineHeight;
    item->setStyle(listItemStyle);
    quad->getChildren().push_back(std::unique_ptr<UiElement>(item));
  }
  build();
}

void VerticalList::removeListItemAtIndex(size_t index) {
  if (index < children.size()) {
    children.erase(children.begin() + index);
    build();
  }
}

const std::pair<int, int> VerticalList::getDims() const {
  const Quad* quad = getQuad();
  if (quad == nullptr) {
    return {style.width, 0};
  }

  return {style.width, (props.lineHeight + props.lineGap) * quad->getChildren().size()};
}

void VerticalList::build() {
  Quad* quad = getQuad();
  if (quad == nullptr) {
    children.push_back(std::make_unique<Quad>(window));
    quad = getQuad();
  }

  ui::QuadProps quadProps = quad->getProps();
  quadProps.bgColor = props.bgColor;
  quadProps.borderColor = Colors::Transparent;
  quadProps.borderSize = 0;
  quad->setProps(quadProps);
  ui::BaseStyle quadStyle = quad->getStyle();
  quadStyle.x = style.x;
  quadStyle.y = style.y;
  quadStyle.width = style.width;
  quadStyle.height = (props.lineHeight + props.lineGap) * quad->getChildren().size();
  quadStyle.scale = 1.f;
  quad->setStyle(quadStyle);
}

void VerticalList::render(int dt) {
  // auto& draw = window->getDraw();

  // Render children
  UiElement::render(dt);
}

} // namespace ui
