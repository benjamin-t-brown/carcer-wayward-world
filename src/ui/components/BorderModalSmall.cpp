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
  return {style.width - BorderModalSmall::BORDER_WIDTH,
          style.height - BorderModalSmall::TOP_LEFT_SQUARE_SIZE};
}

const std::pair<int, int> BorderModalSmall::getIconLocation(int iconWidth,
                                                            int iconHeight) const {
  // Icon goes in the top-left square, centered
  int iconX = style.x + (BorderModalSmall::TOP_LEFT_SQUARE_SIZE - iconWidth) / 2;
  int iconY = style.y + (BorderModalSmall::TOP_LEFT_SQUARE_SIZE - iconHeight) / 2;
  return {iconX, iconY};
}

const std::pair<int, int> BorderModalSmall::getCloseButtonLocation() const {
  ButtonClose closeButton = ButtonClose(window);
  auto closeWidth = closeButton.getDims().first;

  // Close button goes in the top-right corner of the top rectangle
  int closeX = style.x + static_cast<int>(style.width * style.scale) - closeWidth -
               BorderModalSmall::CLOSE_BUTTON_PADDING; // Right edge of top rectangle
  int closeY = style.y + BorderModalSmall::CLOSE_BUTTON_PADDING;
  return {closeX, closeY};
}

const std::pair<int, int> BorderModalSmall::getTitleLocation() const {
  // Title goes in the top rectangle, left-aligned with some padding
  int titleX = style.x + BorderModalSmall::TOP_LEFT_SQUARE_SIZE +
               BorderModalSmall::BORDER_WIDTH; // Left edge of top rectangle + padding
  int titleY = style.y + BorderModalSmall::BORDER_WIDTH; // Top edge + padding
  return {titleX, titleY};
}

const std::pair<int, int> BorderModalSmall::getSubTitleLocation() const {
  // Subtitle goes below the title in the top rectangle
  int subTitleX = style.x + BorderModalSmall::TOP_LEFT_SQUARE_SIZE +
                  BorderModalSmall::BORDER_WIDTH;                // Same as title
  int subTitleY = style.y + BorderModalSmall::SUBTITLE_Y_OFFSET; // Below title
  return {subTitleX, subTitleY};
}

const std::pair<int, int> BorderModalSmall::getContentLocation() const {
  // Content area is the main area inside the border, with padding
  int contentX = style.x + BorderModalSmall::BORDER_WIDTH; // Left border + padding
  int contentY =
      style.y + BorderModalSmall::TOP_LEFT_SQUARE_SIZE; // Below top rectangle + padding
  return {contentX, contentY};
}

void BorderModalSmall::build() {
  children.clear();

  // Calculate scaled dimensions
  int scaledWidth = static_cast<int>(style.width * style.scale);
  int scaledHeight = static_cast<int>(style.height * style.scale);

  // 1. A square in the top left
  auto topLeftSquare = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle squareStyle;
  squareStyle.x = style.x;
  squareStyle.y = style.y;
  squareStyle.width = BorderModalSmall::TOP_LEFT_SQUARE_SIZE;
  squareStyle.height = BorderModalSmall::TOP_LEFT_SQUARE_SIZE;
  squareStyle.scale = style.scale;
  topLeftSquare->setStyle(squareStyle);
  // Uses default OutsetRectangleProps (BorderModalStandard colors, borderSize = 4)
  topLeftSquare->setProps(OutsetRectangleProps{});
  children.push_back(std::move(topLeftSquare));

  // 2. A {style.width - TOP_LEFT_SQUARE_SIZE} x TOP_LEFT_SQUARE_SIZE rectangle on the top
  auto topRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle topStyle;
  topStyle.x = style.x + BorderModalSmall::TOP_LEFT_SQUARE_SIZE;
  topStyle.y = style.y;
  topStyle.width = scaledWidth - BorderModalSmall::TOP_LEFT_SQUARE_SIZE;
  topStyle.height = BorderModalSmall::TOP_LEFT_SQUARE_SIZE;
  topStyle.scale = 1.0f; // Already scaled in width calculation
  topRectangle->setStyle(topStyle);
  topRectangle->setProps(OutsetRectangleProps{});
  children.push_back(std::move(topRectangle));

  // 3. A BORDER_WIDTH x {style.height - TOP_LEFT_SQUARE_SIZE} rectangle on the left with
  // no border
  auto leftRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle leftStyle;
  leftStyle.x = style.x;
  leftStyle.y = style.y + BorderModalSmall::TOP_LEFT_SQUARE_SIZE;
  leftStyle.width = BorderModalSmall::BORDER_WIDTH;
  leftStyle.height = scaledHeight - BorderModalSmall::TOP_LEFT_SQUARE_SIZE;
  leftStyle.scale = 1.0f; // Already scaled in height calculation
  leftRectangle->setStyle(leftStyle);
  OutsetRectangleProps leftProps;
  leftProps.borderSize = 0;
  leftRectangle->setProps(leftProps);
  children.push_back(std::move(leftRectangle));

  // 4. A BORDER_WIDTH x {style.height - TOP_LEFT_SQUARE_SIZE} rectangle on the right with
  // no border
  auto rightRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle rightStyle;
  rightStyle.x = style.x + scaledWidth - BorderModalSmall::BORDER_WIDTH;
  rightStyle.y = style.y + BorderModalSmall::TOP_LEFT_SQUARE_SIZE;
  rightStyle.width = BorderModalSmall::BORDER_WIDTH;
  rightStyle.height = scaledHeight - BorderModalSmall::TOP_LEFT_SQUARE_SIZE;
  rightStyle.scale = 1.0f; // Already scaled in calculations
  rightRectangle->setStyle(rightStyle);
  OutsetRectangleProps rightProps;
  rightProps.borderSize = 0;
  rightProps.color = Colors::BorderModalStandardLight;
  rightRectangle->setProps(rightProps);
  children.push_back(std::move(rightRectangle));

  // 5. A {style.width - BOTTOM_BORDER_WIDTH} x BORDER_WIDTH rectangle on the bottom with
  // no border
  auto bottomRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle bottomStyle;
  bottomStyle.x = style.x;
  bottomStyle.y = style.y + scaledHeight - BorderModalSmall::BORDER_WIDTH;
  bottomStyle.width = scaledWidth;
  bottomStyle.height = BorderModalSmall::BORDER_WIDTH;
  bottomStyle.scale = 1.0f; // Already scaled in calculations
  bottomRectangle->setStyle(bottomStyle);
  OutsetRectangleProps bottomProps;
  bottomProps.borderSize = 0;
  bottomRectangle->setProps(bottomProps);
  children.push_back(std::move(bottomRectangle));
}

void BorderModalSmall::render() { UiElement::render(); }

} // namespace ui
