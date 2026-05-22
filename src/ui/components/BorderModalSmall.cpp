#include "BorderModalSmall.h"

namespace ui {

BorderModalSmall::BorderModalSmall(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void BorderModalSmall::setProps(const BorderModalSmallProps& _props) {
  props = _props;
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

const std::pair<int, int> BorderModalSmall::getIconBorderLocation() const {
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  int margin = style.scale * (props.headerHeight - props.iconSize) / 2;
  int iconBorderX = style.x + scaledBorderWidth + margin;
  int iconBorderY = style.y + scaledBorderWidth + margin;
  return {iconBorderX, iconBorderY};
}

const std::pair<int, int> BorderModalSmall::getIconLocationCenter() const {
  auto [iconBorderX, iconBorderY] = getIconBorderLocation();
  return {iconBorderX + props.iconSize / 2, iconBorderY + props.iconSize / 2};
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
  int titleX = style.x + scaledBorderWidth + margin * 2 + props.iconSize;
  int titleY = style.y + scaledBorderWidth + props.headerHeight / 2;
  return {titleX, titleY};
}

const std::pair<int, int> BorderModalSmall::getContentLocation() const {
  int contentX = style.x + props.borderWidth;
  int contentY = style.y + props.headerHeight + props.borderWidth;
  return {contentX, contentY};
}

void BorderModalSmall::build() {}

void BorderModalSmall::renderBgOverlay() {
  int contentX = style.x;
  int contentY = style.y;
  int contentWidthScaled = style.width * style.scale;
  int contentHeightScaled = style.height * style.scale;
  auto& draw = window->getDraw();
  auto& store = window->getStore();
  auto& sprite = store.getSprite("ui_overlay_256");
  auto alpha = draw.getGlobalAlpha();
  draw.setGlobalAlpha(40);

  // Get native sprite size (assuming sprite.w and sprite.h exist)
  int spriteW = sprite.w;
  int spriteH = sprite.h;

  // Tile the sprite across the content area
  for (int y = contentY; y < contentY + contentHeightScaled; y += spriteH) {
    for (int x = contentX; x < contentX + contentWidthScaled; x += spriteW) {
      int clipW = std::min(spriteW, contentX + contentWidthScaled - x);
      int clipH = std::min(spriteH, contentY + contentHeightScaled - y);
      // Only draw portion if at the edge (may be partial)
      draw.drawSprite(sprite,
                      sdl2w::RenderableParamsEx{
                          .scale = {1.0, 1.0},
                          .x = x,
                          .y = y,
                          .w = clipW,
                          .h = clipH,
                          .clipX = 0,
                          .clipY = 0,
                          .clipW = clipW,
                          .clipH = clipH,
                          .centered = false,
                      });
    }
  }

  draw.setGlobalAlpha(alpha);
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
  draw.drawRect(
      iconBorderX, iconBorderY, props.iconSize, props.iconSize, Colors::DarkBlue);
  UiElement::render(dt);
  renderBgOverlay();
}

} // namespace ui
