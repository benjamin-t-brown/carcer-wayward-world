module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.ui.BorderModalSmall;
export import carcer.ui.UiElement;
import sdl2w;
import carcer.ui.colors;
import carcer.ui.components.TiledOverlay;
import bmin.string_interop;
#include "macros.h"

export {

// --- from ui/components/borders/BorderModalSmall.h ---
namespace ui {

struct BorderModalSmallProps {
  int width = 0;
  int height = 0;
  int headerHeight = 80;
  int iconSize = 64;
  int borderWidth = 2;
};

class BorderModalSmall : public UiElement {
protected:
  BorderModalSmallProps props;

public:
  BorderModalSmall(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderModalSmall() override = default;

  void setProps(const BorderModalSmallProps& _props);
  BorderModalSmallProps& getProps();
  const BorderModalSmallProps& getProps() const;

  const std::pair<int, int> getDims() const override;
  const std::pair<int, int> getContentDims() const;
  const std::pair<int, int> getIconBorderLocation() const;
  const std::pair<int, int> getIconLocationCenter() const;
  const std::pair<int, int> getCloseButtonLocation() const;
  const std::pair<int, int> getTitleLocation() const;
  const std::pair<int, int> getContentLocation() const;

  void buildTiledOverlay();
  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

BorderModalSmall::BorderModalSmall(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void BorderModalSmall::setProps(const BorderModalSmallProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

BorderModalSmallProps& BorderModalSmall::getProps() { return props; }

const BorderModalSmallProps& BorderModalSmall::getProps() const { return props; }

const std::pair<int, int> BorderModalSmall::getDims() const {
  return {style.width * style.scale, style.height * style.scale};
}

const std::pair<int, int> BorderModalSmall::getContentDims() const {
  auto [scaledWidth, scaledHeight] = getDims();
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  int scaledHeaderHeight = static_cast<int>(props.headerHeight * style.scale);
  return {scaledWidth - scaledBorderWidth * 2,
          scaledHeight - scaledBorderWidth * 2 - scaledHeaderHeight};
}

const std::pair<int, int> BorderModalSmall::getContentLocation() const {
  int contentX = style.x + props.borderWidth * style.scale;
  int contentY =
      style.y + props.headerHeight * style.scale + props.borderWidth * style.scale;
  return {contentX, contentY};
}

const std::pair<int, int> BorderModalSmall::getIconBorderLocation() const {
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  int margin = style.scale * (props.headerHeight - props.iconSize) / 2;
  int iconBorderX = style.x + scaledBorderWidth + margin;
  int iconBorderY = style.y + scaledBorderWidth + margin;
  return {iconBorderX, iconBorderY};
}

const std::pair<int, int> BorderModalSmall::getIconLocationCenter() const {
  auto [iconBorderX, iconBorderY] = getIconBorderLocation();
  return {iconBorderX + props.iconSize * style.scale / 2,
          iconBorderY + props.iconSize * style.scale / 2};
}

const std::pair<int, int> BorderModalSmall::getCloseButtonLocation() const {
  auto [scaledWidth, scaledHeight] = getDims();
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  return {style.x + scaledWidth - scaledBorderWidth - 32 * style.scale,
          style.y + scaledBorderWidth};
}

const std::pair<int, int> BorderModalSmall::getTitleLocation() const {
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  int margin = style.scale * (props.headerHeight - props.iconSize) / 2;
  int titleX = style.x + scaledBorderWidth + margin * 2 * style.scale +
               props.iconSize * style.scale;
  int titleY = style.y + scaledBorderWidth + props.headerHeight * style.scale / 2;
  return {titleX, titleY};
}

void BorderModalSmall::buildTiledOverlay() {
  removeChildById("tiledOverlay");
  if (style.width <= 0 || style.height <= 0) {
    return;
  }

  auto* overlay = new TiledOverlay(window, this);
  overlay->setId("tiledOverlay");
  overlay->setPos(style.x, style.y);
  overlay->setScale(style.scale);
  overlay->setProps(TiledOverlayProps{
      .width = style.width,
      .height = style.height,
      .spriteName = "ui_overlay_256",
      .alpha = 40,
  });
  addChild(overlay);
}

void BorderModalSmall::build() {
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  buildTiledOverlay();
}

void BorderModalSmall::render(int dt) {
  auto [scaledWidth, scaledHeight] = getDims();
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  auto& draw = window->getDraw();
  // border
  draw.drawRect(
      style.x, style.y, scaledWidth, scaledHeight, Colors::BorderModalStandardDark);
  // background
  draw.drawRect(style.x + scaledBorderWidth,
                style.y + scaledBorderWidth,
                scaledWidth - scaledBorderWidth * 2,
                scaledHeight - scaledBorderWidth * 2,
                Colors::ModalStandardBackground);
  // title background
  draw.drawRect(style.x + scaledBorderWidth,
                style.y + scaledBorderWidth,
                scaledWidth - scaledBorderWidth * 2,
                props.headerHeight * style.scale,
                Colors::ModalHeaderBackground);
  // icon background
  auto [iconBorderX, iconBorderY] = getIconBorderLocation();
  draw.drawRect(iconBorderX,
                iconBorderY,
                props.iconSize * style.scale,
                props.iconSize * style.scale,
                Colors::DarkBlue);
  UiElement::render(dt);
}

} // namespace ui
