#include "ModalSmall.h"
#include "lib/sdl2w/Draw.h"
#include "ui/components/borders/BorderModalSmall.h"
#include "ui/elements/buttons/ButtonClose.h"

namespace ui {

ModalSmall::ModalSmall(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Layout doesn't need special initialization
}

void ModalSmall::setProps(const ModalSmallProps& _props) {
  props = _props;
  build();
}

ModalSmallProps& ModalSmall::getProps() { return props; }

const ModalSmallProps& ModalSmall::getProps() const { return props; }

int ModalSmall::getScaledButtonsAreaHeight() {
  auto borderElement = dynamic_cast<BorderModalSmall*>(getChildById("border"));
  if (borderElement == nullptr) {
    return 0;
  }
  return static_cast<int>(BUTTONS_AREA_HEIGHT * borderElement->getStyle().scale);
}

const std::pair<int, int> ModalSmall::getButtonsDims() {
  auto borderElement = dynamic_cast<BorderModalSmall*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  auto [contentW, _] = borderElement->getContentDims();
  return {contentW, getScaledButtonsAreaHeight()};
}

const std::pair<int, int> ModalSmall::getButtonsLocation() {
  auto borderElement = dynamic_cast<BorderModalSmall*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  auto [contentX, contentY] = borderElement->getContentLocation();
  auto [_, contentH] = borderElement->getContentDims();
  int buttonsH = getScaledButtonsAreaHeight();
  return {contentX, contentY + contentH - buttonsH};
}

const std::pair<int, int> ModalSmall::getContentDims() {
  auto borderElement = dynamic_cast<BorderModalSmall*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  auto [contentW, contentH] = borderElement->getContentDims();
  return {contentW, contentH - getScaledButtonsAreaHeight()};
}

const std::pair<int, int> ModalSmall::getContentLocation() {
  auto borderElement = dynamic_cast<BorderModalSmall*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getContentLocation();
}

void ModalSmall::setTitleElement(UiElement* _titleElement) {
  removeChildById("title");
  auto borderElement = dynamic_cast<BorderModalSmall*>(getChildById("border"));
  // Add new title element
  if (_titleElement && borderElement) {
    auto& titleStyle = _titleElement->getStyle();
    auto [titleX, titleY] = borderElement->getTitleLocation();
    auto [titleWidth, titleHeight] = _titleElement->getDims();
    titleStyle.x = titleX;
    titleStyle.y = titleY - titleHeight / 2;
    _titleElement->setId("title");
    _titleElement->build();
    addChild(_titleElement);
  }
}

UiElement* ModalSmall::getTitleElement() { return getChildById("title"); }

UiElement* ModalSmall::getCloseButtonElement() { return getChildById("closeButton"); }

void ModalSmall::build() {
  removeChildById("border");
  removeChildById("closeButton");

  // Create border element
  auto border = new BorderModalSmall(window, this);
  border->setId("border");
  auto& borderStyle = border->getStyle();
  borderStyle.x = style.x;
  borderStyle.y = style.y;
  borderStyle.width = style.width;
  borderStyle.height = style.height;
  borderStyle.scale = style.scale;
  addChild(border);

  if (props.enableCloseButton) {
    auto [closeX, closeY] = border->getCloseButtonLocation();

    auto modalClose = new ButtonClose(window, this);
    modalClose->setId("closeButton");
    ui::BaseStyle modalCloseStyle;
    modalCloseStyle.x = closeX;
    modalCloseStyle.y = closeY;
    modalCloseStyle.width = 32;
    modalCloseStyle.height = 32;
    modalCloseStyle.scale = style.scale;
    modalClose->setStyle(modalCloseStyle);
    ui::ButtonCloseProps modalCloseProps;
    modalCloseProps.closeType = ui::CloseType::MODAL;
    modalClose->setProps(modalCloseProps);
    addChild(modalClose);
  }

  // TODO decoration sprite
}

void ModalSmall::render(int dt) {
  auto& draw = window->getDraw();
  draw.drawRect(style.x, style.y, style.width, style.height, props.backgroundColor);
  UiElement::render(dt);
}

} // namespace ui
