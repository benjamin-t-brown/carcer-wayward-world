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
  ui::BaseStyle modalCloseStyle;
  modalCloseStyle.x = closeLocation.first;
  modalCloseStyle.y = closeLocation.second;
  modalCloseStyle.width = 32;
  modalCloseStyle.height = 32;
  modalClose->setStyle(modalCloseStyle);
  ui::ButtonCloseProps modalCloseProps;
  modalCloseProps.closeType = ui::CloseType::MODAL;
  modalClose->setProps(modalCloseProps);
  // modalClose->addEventObserver(std::make_unique<TestButtonObserver>("modalClose"));
  children.push_back(std::move(modalClose));

  // TODO decoration sprite
}

void ModalStandard::setTitleElement(std::unique_ptr<UiElement> _titleElement) {
  removeChildById("title");
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  // Add new title element
  if (_titleElement && borderElement) {
    BaseStyle& titleStyle = _titleElement->getStyle();
    auto titleLocation = borderElement->getTitleLocation();
    titleStyle.x = titleLocation.first;
    titleStyle.y = titleLocation.second;
    _titleElement->setStyle(titleStyle);
    _titleElement->setId("title");
    children.push_back(std::move(_titleElement));
  }
}

UiElement* ModalStandard::getTitleElement() { return getChildById("title"); }

void ModalStandard::setSubtitleElement(std::unique_ptr<UiElement> _subtitleElement) {
  removeChildById("subtitle");
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  // Add new subtitle element
  if (_subtitleElement && borderElement) {
    BaseStyle& subtitleStyle = _subtitleElement->getStyle();
    auto subtitleLocation = borderElement->getSubTitleLocation();
    subtitleStyle.x = subtitleLocation.first;
    subtitleStyle.y = subtitleLocation.second;
    _subtitleElement->setStyle(subtitleStyle);
    _subtitleElement->setId("subtitle");
    children.push_back(std::move(_subtitleElement));
  }
}

UiElement* ModalStandard::getSubtitleElement() { return getChildById("subtitle"); }

void ModalStandard::setContentElement(std::unique_ptr<UiElement> _contentElement) {
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
    children.push_back(std::move(_contentElement));
  }
}

UiElement* ModalStandard::getContentElement() { return getChildById("content"); }

UiElement* ModalStandard::getCloseButtonElement() { return getChildById("closeButton"); }

void ModalStandard::render() {
  auto& draw = window->getDraw();
  draw.drawRect(style.x, style.y, style.width, style.height, props.backgroundColor);
  UiElement::render();
}

} // namespace ui
