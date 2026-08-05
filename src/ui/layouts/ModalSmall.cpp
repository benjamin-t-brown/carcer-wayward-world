#include "ModalSmall.h"
#include "bmin/StringInterop.h"
#include "sdl2w/Draw.h"
#include "ui/components/borders/BorderModalSmall.h"
#include "ui/elements/Quad.h"
#include "ui/elements/buttons/ButtonClose.h"
#include "ui/helpers/modalLayoutFit.h"
#include <algorithm>

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
  return static_cast<int>(BUTTONS_AREA_HEIGHT * style.scale);
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
    auto [titleX, titleY] = borderElement->getTitleLocation();
    auto [titleWidth, titleHeight] = _titleElement->getDims();
    _titleElement->setPos(titleX, titleY - titleHeight / 2);
    _titleElement->setId("title");
    addChild(_titleElement);
  }
}

UiElement* ModalSmall::getTitleElement() { return getChildById("title"); }

UiElement* ModalSmall::getCloseButtonElement() { return getChildById("closeButton"); }

void ModalSmall::build() {
  removeChildById("border");
  removeChildById("closeButton");
  removeChildById("headerIcon");

  if (props.width > 0 && props.height > 0) {
    if (props.layoutFit == LayoutFit::CappedCentered) {
      const auto rect =
          computeCappedCenteredRect(props.width, props.height, ModalSizeClass::Small);
      style.x = rect.x;
      style.y = rect.y;
      style.width = rect.width;
      style.height = rect.height;
    } else {
      style.width = props.width;
      style.height = props.height;
    }
  }

  constexpr int baseHeaderHeight = 80;
  constexpr int baseIconSize = 64;
  const float iconScale = props.iconScale > 0.f ? props.iconScale : 1.f;
  const int headerHeight = std::max(1, static_cast<int>(baseHeaderHeight * iconScale));
  const int iconWellSize = std::max(1, static_cast<int>(baseIconSize * iconScale));

  int spriteW = 0;
  int spriteH = 0;
  if (!props.iconSprite.empty() && window) {
    const auto& sprite =
        window->getStore().getSprite(bmin::toStringView(props.iconSprite));
    spriteW = std::max(1, sprite.w);
    spriteH = std::max(1, sprite.h);
  }

  // Create border element
  auto border = new BorderModalSmall(window, this);
  border->setId("border");
  border->setPos(style.x, style.y);
  border->setScale(style.scale);
  border->setProps(BorderModalSmallProps{
      .width = style.width,
      .height = style.height,
      .headerHeight = headerHeight,
      .iconSize = iconWellSize,
  });
  addChild(border);

  if (props.enableCloseButton) {
    auto [closeX, closeY] = border->getCloseButtonLocation();

    auto modalClose = new ButtonClose(window, this);
    modalClose->setId("closeButton");
    modalClose->setPos(closeX, closeY);
    modalClose->setScale(style.scale);
    ui::ButtonCloseProps modalCloseProps;
    modalCloseProps.closeType = ui::CloseType::MODAL;
    modalClose->setProps(modalCloseProps);
    addChild(modalClose);
  }

  if (!props.iconSprite.empty() && spriteW > 0 && spriteH > 0) {
    const float drawScale = iconScale * style.scale;
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

void ModalSmall::render(int dt) {
  auto& draw = window->getDraw();
  draw.drawRect(style.x, style.y, style.width, style.height, props.backgroundColor);
  UiElement::render(dt);
}

} // namespace ui
