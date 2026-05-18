#include "ModalStandard.h"
#include "lib/sdl2w/Draw.h"
#include "ui/components/BorderModalStandard.h"
#include "ui/elements/ButtonClose.h"

namespace ui {

ModalStandard::ModalStandard(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Layout doesn't need special initialization
}

void ModalStandard::setProps(const ModalStandardProps& _props) { props = _props; }

ModalStandardProps& ModalStandard::getProps() { return props; }

const ModalStandardProps& ModalStandard::getProps() const { return props; }

UiElement* ModalStandard::getChildById(const std::string& id) {
  for (auto& child : children) {
    if (child->getId() == id) {
      return child.get();
    }
  }
  return nullptr;
}

void ModalStandard::removeChildById(const std::string& id) {
  for (size_t i = 0; i < children.size(); i++) {
    auto& child = children[i];
    if (child->getId() == id) {
      children.erase(children.begin() + i);
      return;
    }
  }
}

const std::pair<int, int> ModalStandard::getDims() const {
  return {style.width, style.height};
}

const std::pair<int, int> ModalStandard::getContentDims() {
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getContentDims();
}

const std::pair<int, int> ModalStandard::getContentLoc() {
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getContentLoc();
}

void ModalStandard::build() {
  removeChildById("border");
  removeChildById("closeButton");

  // Create border element
  auto border = std::make_unique<BorderModalStandard>(window, this);
  border->setId("border");
  BaseStyle borderStyle;
  borderStyle.x = style.x;
  borderStyle.y = style.y;
  borderStyle.width = style.width;
  borderStyle.height = style.height;
  border->setStyle(borderStyle);

  // Get close button location before moving border
  auto closeLocation = border->getCloseButtonLocation();

  // Insert border at the beginning
  children.insert(children.begin(), std::move(border));

  auto modalClose = std::make_unique<ButtonClose>(window, this);
  modalClose->setId("closeButton");
  auto& modalCloseStyle = modalClose->getStyle();
  modalCloseStyle.x = closeLocation.first;
  modalCloseStyle.y = closeLocation.second;
  ui::ButtonCloseProps modalCloseProps;
  modalCloseProps.closeType = ui::CloseType::MODAL;
  modalClose->setProps(modalCloseProps);
  // modalClose->addEventObserver(new TestButtonObserver("modalClose"));
  children.push_back(std::move(modalClose));

  // TODO decoration sprite
}

void ModalStandard::setTitleElement(UiElement* _titleElement) {
  removeChildById("title");
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  // Add new title element
  if (_titleElement && borderElement) {
    BaseStyle& titleStyle = _titleElement->getStyle();
    auto [titleX, titleY] = borderElement->getTitleLocation();
    auto [titleWidth, titleHeight] = _titleElement->getDims();
    titleStyle.x = titleX;
    titleStyle.y = titleY - titleHeight / 2;
    _titleElement->setId("title");
    _titleElement->build();
    children.push_back(std::unique_ptr<UiElement>(_titleElement));
  }
}

UiElement* ModalStandard::getTitleElement() { return getChildById("title"); }

void ModalStandard::setContentElement(UiElement* _contentElement) {
  removeChildById("content");
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  // Add new content element
  if (_contentElement) {
    BaseStyle& contentStyle = _contentElement->getStyle();
    auto contentLocation = borderElement->getContentLocation();
    contentStyle.x = contentLocation.first;
    contentStyle.y = contentLocation.second;
    _contentElement->setStyle(contentStyle);
    _contentElement->setId("content");
    children.push_back(std::unique_ptr<UiElement>(_contentElement));
  }
}

UiElement* ModalStandard::getContentElement() { return getChildById("content"); }

UiElement* ModalStandard::getCloseButtonElement() { return getChildById("closeButton"); }

void ModalStandard::render(int dt) {
  // auto& draw = window->getDraw();
  // draw.drawRect(style.x, style.y, style.width, style.height,
  // props.contentBackgroundColor);
  UiElement::render(dt);
}

} // namespace ui
