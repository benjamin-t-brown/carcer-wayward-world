#include "OutsetRectangle.h"

namespace ui {

OutsetRectangle::OutsetRectangle(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Default props
}

void OutsetRectangle::setProps(const OutsetRectangleProps& _props) {
  props = _props;
  build();
}

OutsetRectangleProps& OutsetRectangle::getProps() { return props; }

const OutsetRectangleProps& OutsetRectangle::getProps() const { return props; }

const std::pair<int, int> OutsetRectangle::getDims() const {
  return {style.width, style.height};
}

void OutsetRectangle::build() {
  style.width = props.width;
  style.height = props.height;
}

void OutsetRectangle::render(int dt) {
  auto& draw = window->getDraw();

  // Calculate scaled dimensions
  int scaledX = static_cast<int>(style.x);
  int scaledY = static_cast<int>(style.y);
  int scaledWidth = static_cast<int>(style.width * style.scale);
  int scaledHeight = static_cast<int>(style.height * style.scale);
  int borderSize = static_cast<int>(props.borderSize * style.scale);

  // Draw the main rectangle
  draw.drawRect(scaledX + borderSize,
                scaledY + borderSize,
                scaledWidth - borderSize * 2,
                scaledHeight - borderSize * 2,
                props.color);

  // Draw outset border effect
  if (props.borderSize > 0) {
    // Top and Right borders
    draw.drawRect(scaledX, scaledY, scaledWidth, borderSize, props.colorTopRight);
    draw.drawRect(scaledX + scaledWidth - borderSize,
                  scaledY,
                  borderSize,
                  scaledHeight,
                  props.colorTopRight);
    // Bottom and Left borders
    draw.drawRect(scaledX,
                  scaledY + scaledHeight - borderSize,
                  scaledWidth,
                  borderSize,
                  props.colorBottomLeft);
    draw.drawRect(scaledX, scaledY, borderSize, scaledHeight, props.colorBottomLeft);

    if (borderSize > 1) {
      // Diagonal corners top left bottom right
      for (int i = 0; i < borderSize; i++) {
        auto topLeftX = scaledX;
        auto topLeftY = scaledY;
        draw.drawLine({topLeftX + i, topLeftY},
                      {topLeftX + i, topLeftY + i},
                      1,
                      props.colorTopRight);

        auto bottomRightX = scaledX + scaledWidth;
        auto bottomRightY = scaledY + scaledHeight;
        draw.drawLine({bottomRightX - borderSize + i, bottomRightY - borderSize},
                      {bottomRightX - borderSize + i, bottomRightY - borderSize + i},
                      1,
                      props.colorTopRight);
      }
    }
  }

  // Render children
  UiElement::render(dt);
}

} // namespace ui
