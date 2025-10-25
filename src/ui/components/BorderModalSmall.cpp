#include "BorderModalSmall.h"
#include "../elements/OutsetRectangle.h"
#include "ui/elements/ButtonClose.h"

namespace ui {

BorderModalSmall::BorderModalSmall(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Component doesn't need special initialization
}

const std::pair<int, int> BorderModalSmall::getDims() const {
  return {style.width, style.height};
}

const std::pair<int, int> BorderModalSmall::getContentDims() const {
  return {style.width - 4, style.height - 78};
}

const std::pair<int, int> BorderModalSmall::getIconLocation(int iconWidth,
                                                            int iconHeight) const {
  // Icon goes in the top-left 78x78 square, centered
  int iconX = style.x + (78 - iconWidth) / 2;
  int iconY = style.y + (78 - iconHeight) / 2;
  return {iconX, iconY};
}

const std::pair<int, int> BorderModalSmall::getCloseButtonLocation() const {
  ButtonClose closeButton = ButtonClose(window);
  auto closeWidth = closeButton.getDims().first;

  // Close button goes in the top-right corner of the top rectangle
  int closeX = style.x + static_cast<int>(style.width * style.scale) - closeWidth -
               6; // Right edge of top rectangle
  int closeY = style.y + 6;
  return {closeX, closeY};
}

const std::pair<int, int> BorderModalSmall::getTitleLocation() const {
  // Title goes in the top rectangle, left-aligned with some padding
  int titleX = style.x + 78 + 10; // Left edge of top rectangle + padding
  int titleY = style.y + 10;      // Top edge + padding
  return {titleX, titleY};
}

const std::pair<int, int> BorderModalSmall::getSubTitleLocation() const {
  // Subtitle goes below the title in the top rectangle
  int subTitleX = style.x + 78 + 10; // Same as title
  int subTitleY = style.y + 40;      // Below title
  return {subTitleX, subTitleY};
}

const std::pair<int, int> BorderModalSmall::getContentLocation() const {
  // Content area is the main area inside the border, with padding
  int contentX = style.x + 4;  // Left border + padding
  int contentY = style.y + 78; // Below top rectangle + padding
  return {contentX, contentY};
}

void BorderModalSmall::build() {
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

  // 3. A 4 x {style.height - 78} rectangle on the left with no border
  auto leftRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle leftStyle;
  leftStyle.x = style.x;
  leftStyle.y = style.y + 78;
  leftStyle.width = 4;
  leftStyle.height = scaledHeight - 78;
  leftStyle.scale = 1.0f; // Already scaled in height calculation
  leftRectangle->setStyle(leftStyle);
  OutsetRectangleProps leftProps;
  leftProps.borderSize = 0;
  leftRectangle->setProps(leftProps);
  children.push_back(std::move(leftRectangle));

  // 4. A 4 x {style.height - 78} rectangle on the right with no border
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

  // 5. A {style.width - 8} x 4 rectangle on the bottom with no border
  auto bottomRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle bottomStyle;
  bottomStyle.x = style.x + 4;
  bottomStyle.y = style.y + scaledHeight - 4;
  bottomStyle.width = scaledWidth - 8;
  bottomStyle.height = 4;
  bottomStyle.scale = 1.0f; // Already scaled in calculations
  bottomRectangle->setStyle(bottomStyle);
  OutsetRectangleProps bottomProps;
  bottomProps.borderSize = 0;
  bottomRectangle->setProps(bottomProps);
  children.push_back(std::move(bottomRectangle));
}

void BorderModalSmall::render() {
  UiElement::render();
}

} // namespace ui
