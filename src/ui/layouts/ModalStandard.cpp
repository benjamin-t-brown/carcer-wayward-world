#include "ModalStandard.h"
#include "ui/components/borders/BorderModalStandard.h"
#include "ui/elements/Quad.h"
#include "ui/elements/buttons/ButtonClose.h"

namespace ui {

ModalStandard::ModalStandard(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void ModalStandard::setProps(const ModalStandardProps& _props) {
  props = _props;
  build();
}

ModalStandardProps& ModalStandard::getProps() { return props; }

const ModalStandardProps& ModalStandard::getProps() const { return props; }

const std::pair<int, int> ModalStandard::getSubTitleDims() {
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getSubTitleDims();
}

const std::pair<int, int> ModalStandard::getSubTitleLocation() {
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getSubTitleLocation();
}

const std::pair<int, int> ModalStandard::getContentDims() {
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getContentDims();
}

const std::pair<int, int> ModalStandard::getContentLocation() {
  auto borderElement = dynamic_cast<BorderModalStandard*>(getChildById("border"));
  if (borderElement == nullptr) {
    return {0, 0};
  }
  return borderElement->getContentLocation();
}

void ModalStandard::build() {
  removeChildById("border");
  removeChildById("closeButton");
  removeChildById("headerIcon");

  // Create border element
  auto border = new BorderModalStandard(window, this);
  border->setId("border");
  auto& borderStyle = border->getStyle();
  borderStyle.x = style.x;
  borderStyle.y = style.y;
  borderStyle.width = style.width;
  borderStyle.height = style.height;
  borderStyle.scale = style.scale;
  border->setProps(BorderModalSmallProps{});

  // Insert border at the beginning
  addChild(border);

  // Get close button location before moving border
  auto [closeX, closeY] = border->getCloseButtonLocation();

  auto modalClose = new ButtonClose(window, this);
  modalClose->setId("closeButton");
  auto& modalCloseStyle = modalClose->getStyle();
  modalCloseStyle.x = closeX;
  modalCloseStyle.y = closeY;
  modalCloseStyle.scale = style.scale;
  ui::ButtonCloseProps modalCloseProps;
  modalCloseProps.closeType = ui::CloseType::MODAL;
  modalClose->setProps(modalCloseProps);
  addChild(modalClose);

  if (!props.iconSprite.empty()) {
    auto* border = dynamic_cast<BorderModalStandard*>(getChildById("border"));
    if (border != nullptr) {
      constexpr int kIconTextureSize = 32;
      constexpr float kIconScale = 2.f;
      constexpr int kIconDisplaySize = 64;
      static_assert(kIconTextureSize * static_cast<int>(kIconScale) == kIconDisplaySize);

      auto [centerX, centerY] = border->getIconSectionCenter();
      const int screenIconSize = static_cast<int>(kIconDisplaySize * style.scale);

      auto icon = new Quad(window, this);
      icon->setId("headerIcon");
      auto& iconStyle = icon->getStyle();
      iconStyle.x = centerX - screenIconSize / 2;
      iconStyle.y = centerY - screenIconSize / 2;
      iconStyle.width = kIconTextureSize;
      iconStyle.height = kIconTextureSize;
      iconStyle.scale = kIconScale * style.scale;
      icon->setProps(QuadProps{.bgSprite = props.iconSprite});
      addChild(icon);
    }
  }

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
    addChild(_titleElement);
  }
}

UiElement* ModalStandard::getTitleElement() { return getChildById("title"); }

UiElement* ModalStandard::getCloseButtonElement() { return getChildById("closeButton"); }

void ModalStandard::render(int dt) { UiElement::render(dt); }

} // namespace ui
