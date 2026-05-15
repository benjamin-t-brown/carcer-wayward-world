#include "ModalSmall.h"
#include "lib/sdl2w/Draw.h"
#include "ui/components/BorderModalSmall.h"
#include "ui/elements/ButtonClose.h"

namespace ui {

ModalSmall::ModalSmall(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Layout doesn't need special initialization
}

void ModalSmall::setProps(const ModalSmallProps& _props) { props = _props; }

ModalSmallProps& ModalSmall::getProps() { return props; }

const ModalSmallProps& ModalSmall::getProps() const { return props; }

UiElement* ModalSmall::getChildById(const std::string& id) {
  for (auto& child : children) {
    if (child->getId() == id) {
      return child.get();
    }
  }
  return nullptr;
}

void ModalSmall::removeChildById(const std::string& id) {
  for (size_t i = 0; i < children.size(); i++) {
    auto& child = children[i];
    if (child->getId() == id) {
      children.erase(children.begin() + i);
      return;
    }
  }
}

const std::pair<int, int> ModalSmall::getDims() const {
  return {style.width, style.height};
}

void ModalSmall::build() {
  removeChildById("border");
  removeChildById("closeButton");

  // Create border element
  auto border = std::make_unique<BorderModalSmall>(window, this);
  border->setId("border");
  auto& borderStyle = border->getStyle();
  borderStyle.x = style.x;
  borderStyle.y = style.y;
  borderStyle.width = style.width;
  borderStyle.height = style.height;

  // Get close button location before moving border
  auto closeLocation = border->getCloseButtonLocation();

  // Insert border at the beginning
  children.insert(children.begin(), std::move(border));

  auto modalClose = std::make_unique<ButtonClose>(window, this);
  modalClose->setId("closeButton");
  ui::BaseStyle modalCloseStyle;
  modalCloseStyle.x = closeLocation.first;
  modalCloseStyle.y = closeLocation.second;
  modalCloseStyle.width = 32;
  modalCloseStyle.height = 32;
  modalClose->setStyle(modalCloseStyle);
  ui::ButtonCloseProps modalCloseProps;
  modalCloseProps.closeType = ui::CloseType::MODAL;
  modalClose->setProps(modalCloseProps);
  children.push_back(std::move(modalClose));

  // TODO decoration sprite
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
    children.push_back(std::unique_ptr<UiElement>(_titleElement));
  }
}

UiElement* ModalSmall::getTitleElement() { return getChildById("title"); }

void ModalSmall::setContentElement(UiElement* _contentElement) {
  removeChildById("content");
  auto borderElement = dynamic_cast<BorderModalSmall*>(getChildById("border"));
  // Add new content element
  if (_contentElement && borderElement) {
    BaseStyle& contentStyle = _contentElement->getStyle();
    auto contentLocation = borderElement->getContentLocation();
    contentStyle.x = contentLocation.first;
    contentStyle.y = contentLocation.second;
    _contentElement->setStyle(contentStyle);
    _contentElement->setId("content");
    children.push_back(std::unique_ptr<UiElement>(_contentElement));
  }
}

UiElement* ModalSmall::getContentElement() { return getChildById("content"); }

UiElement* ModalSmall::getCloseButtonElement() { return getChildById("closeButton"); }

void ModalSmall::render(int dt) {
  auto& draw = window->getDraw();
  draw.drawRect(style.x, style.y, style.width, style.height, props.backgroundColor);
  UiElement::render(dt);
}

} // namespace ui
