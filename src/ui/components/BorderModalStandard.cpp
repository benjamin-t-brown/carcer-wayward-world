#include "BorderModalStandard.h"
#include "../elements/OutsetRectangle.h"

namespace ui {

BorderModalStandard::BorderModalStandard(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Component doesn't need special initialization
}

const std::pair<int, int> BorderModalStandard::getDims() const {
  return {style.width, style.height};
}

void BorderModalStandard::build() {
  children.clear();

  // Calculate scaled dimensions
  int scaledWidth = static_cast<int>(style.width * style.scale);
  int scaledHeight = static_cast<int>(style.height * style.scale);

  // 1. A 78 x 78 pixel square in the top left
  auto topLeftSquare = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle squareStyle;
  squareStyle.x = style.x;
  squareStyle.y = style.y;
  squareStyle.width = 78;
  squareStyle.height = 78;
  squareStyle.scale = style.scale;
  topLeftSquare->setStyle(squareStyle);
  // Uses default OutsetRectangleProps (BorderModalStandard colors, borderSize = 4)
  topLeftSquare->setProps(OutsetRectangleProps{});
  children.push_back(std::move(topLeftSquare));

  // 2. A {style.width - 78} x 78 rectangle on the top
  auto topRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle topStyle;
  topStyle.x = style.x + 78;
  topStyle.y = style.y;
  topStyle.width = scaledWidth - 78;
  topStyle.height = 78;
  topStyle.scale = 1.0f; // Already scaled in width calculation
  topRectangle->setStyle(topStyle);
  topRectangle->setProps(OutsetRectangleProps{});
  children.push_back(std::move(topRectangle));

  // 3. A 16 x {style.height} rectangle on the left
  auto leftRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle leftStyle;
  leftStyle.x = style.x;
  leftStyle.y = style.y + 78;
  leftStyle.width = 16;
  leftStyle.height = scaledHeight - 78;
  leftStyle.scale = 1.0f; // Already scaled in height calculation
  leftRectangle->setStyle(leftStyle);
  leftRectangle->setProps(OutsetRectangleProps{});
  children.push_back(std::move(leftRectangle));

  // 4. A 4 x {style.height - 78} rectangle on the right with border size set to 0
  auto rightRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle rightStyle;
  rightStyle.x = style.x + scaledWidth - 4;
  rightStyle.y = style.y + 78;
  rightStyle.width = 4;
  rightStyle.height = scaledHeight - 78;
  rightStyle.scale = 1.0f; // Already scaled in calculations
  rightRectangle->setStyle(rightStyle);
  OutsetRectangleProps rightProps;
  rightProps.borderSize = 0;
  rightRectangle->setProps(rightProps);
  children.push_back(std::move(rightRectangle));

  // 5. A {style.height - 16} x 4 rectangle on the bottom with border size set to 0
  auto bottomRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle bottomStyle;
  bottomStyle.x = style.x + 16;
  bottomStyle.y = style.y + scaledHeight - 4;
  bottomStyle.width = scaledWidth - 16;
  bottomStyle.height = 4;
  bottomStyle.scale = 1.0f; // Already scaled in calculations
  bottomRectangle->setStyle(bottomStyle);
  OutsetRectangleProps bottomProps;
  bottomProps.borderSize = 0;
  bottomRectangle->setProps(bottomProps);
  children.push_back(std::move(bottomRectangle));
}

void BorderModalStandard::render() {
  // Render all child OutsetRectangle elements
  UiElement::render();
}

} // namespace ui
