#include "ModalStandard.h"
#include "bmin/StringInterop.h"
#include "ui/components/borders/BorderModalStandard.h"
#include "ui/elements/Quad.h"
#include "ui/elements/buttons/ButtonClose.h"
#include <algorithm>

namespace ui {

ModalStandard::ModalStandard(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void ModalStandard::setProps(const ModalStandardProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
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

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  // Create border element
  auto border = new BorderModalStandard(window, this);
  border->setId("border");
  border->setPos(style.x, style.y);
  border->setScale(style.scale);
  border->setProps(BorderModalSmallProps{
      .width = style.width,
      .height = style.height,
      .headerHeight = 80,
      .iconSize = 64,
      .borderWidth = 2,
  });

  // Insert border at the beginning
  addChild(border);

  // Get close button location before moving border
  auto [closeX, closeY] = border->getCloseButtonLocation();

  auto modalClose = new ButtonClose(window, this);
  modalClose->setId("closeButton");
  modalClose->setPos(closeX, closeY);
  modalClose->setScale(style.scale);
  ui::ButtonCloseProps modalCloseProps;
  modalCloseProps.closeType = ui::CloseType::MODAL;
  modalClose->setProps(modalCloseProps);
  addChild(modalClose);

  if (!props.iconSprite.empty()) {
    auto* border = dynamic_cast<BorderModalStandard*>(getChildById("border"));
    if (border != nullptr) {
      // drawSprite always uses the sprite's native w/h (params.w/h are ignored), so the
      // Quad texture must match the sprite or the image is clipped (e.g. 52px portraits
      // into a 32px texture looked shifted up-left).
      const auto& sprite =
          window->getStore().getSprite(bmin::toStringView(props.iconSprite));
      const int spriteW = std::max(1, sprite.w);
      const int spriteH = std::max(1, sprite.h);
      const int iconSize = border->getProps().iconSize;
      const float fitScale =
          static_cast<float>(iconSize) / static_cast<float>(std::max(spriteW, spriteH));
      const float drawScale = fitScale * style.scale;
      const int screenW = static_cast<int>(spriteW * drawScale);
      const int screenH = static_cast<int>(spriteH * drawScale);

      auto [centerX, centerY] = border->getIconLocationCenter();

      auto icon = new Quad(window, this);
      icon->setId("headerIcon");
      icon->setPos(centerX - screenW / 2, centerY - screenH / 2);
      icon->setScale(drawScale);
      icon->setProps(QuadProps{
          .width = spriteW,
          .height = spriteH,
          .bgSprite = props.iconSprite,
      });
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
    auto [titleX, titleY] = borderElement->getTitleLocation();
    auto [titleWidth, titleHeight] = _titleElement->getDims();
    _titleElement->setPos(titleX, titleY - titleHeight / 2);
    _titleElement->setId("title");
    addChild(_titleElement);
  }
}

UiElement* ModalStandard::getTitleElement() { return getChildById("title"); }

UiElement* ModalStandard::getCloseButtonElement() { return getChildById("closeButton"); }

void ModalStandard::render(int dt) { UiElement::render(dt); }

} // namespace ui
