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
  // OutsetRectangle doesn't need to build children, it renders directly
}

void OutsetRectangle::render(int dt) {
  auto& draw = window->getDraw();

  // if (props.color.a <= 0) {
  //   return;
  // }

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

    // Diagonal corners top left bottom right
    for (int i = 0; i < borderSize; i++) {
      draw.drawLine({scaledX + i, scaledY},
                    {scaledX + borderSize, scaledY + borderSize - i},
                    1,
                    props.colorTopRight);
      draw.drawLine(
          {scaledX + scaledWidth - borderSize + i, scaledY + scaledHeight - borderSize},
          {scaledX + scaledWidth, scaledY + scaledHeight - i},
          1,
          props.colorTopRight);
    }
  }

  // Render children
  UiElement::render(dt);
}

} // namespace ui
