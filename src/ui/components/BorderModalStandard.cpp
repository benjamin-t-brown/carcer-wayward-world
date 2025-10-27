#include "BorderModalStandard.h"
#include "ui/elements/ButtonClose.h"
#include "ui/elements/OutsetRectangle.h"

namespace ui {

BorderModalStandard::BorderModalStandard(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Component doesn't need special initialization
}

const std::pair<int, int> BorderModalStandard::getDims() const {
  return {style.width, style.height};
}

const std::pair<int, int> BorderModalStandard::getContentDims() const {
  return {style.width - BorderModalStandard::LEFT_BORDER_WIDTH -
              BorderModalStandard::BORDER_SIZE,
          style.height - BorderModalStandard::TOP_LEFT_SQUARE_SIZE -
              BorderModalStandard::BORDER_SIZE};
}

const std::pair<int, int> BorderModalStandard::getIconLocation(int iconWidth,
                                                               int iconHeight) const {
  // Icon goes in the top-left square, centered
  int iconX = style.x + (BorderModalStandard::TOP_LEFT_SQUARE_SIZE - iconWidth) / 2;
  int iconY = style.y + (BorderModalStandard::TOP_LEFT_SQUARE_SIZE - iconHeight) / 2;
  return {iconX, iconY};
}

const std::pair<int, int> BorderModalStandard::getCloseButtonLocation() const {
  ButtonClose closeButton = ButtonClose(window);
  auto closeWidth = closeButton.getDims().first;

  // Close button goes in the top-right corner of the top rectangle
  int closeX = style.x + static_cast<int>(style.width * style.scale) - closeWidth -
               BorderModalStandard::CLOSE_BUTTON_PADDING; // Right edge of top rectangle
  int closeY = style.y + BorderModalStandard::CLOSE_BUTTON_PADDING;
  return {closeX, closeY};
}

const std::pair<int, int> BorderModalStandard::getTitleLocation() const {
  // Title goes in the top rectangle, left-aligned with some padding
  int titleX = style.x + BorderModalStandard::TOP_LEFT_SQUARE_SIZE +
               BorderModalStandard::BORDER_SIZE; // Left edge of top rectangle + padding
  int titleY = style.y + BorderModalStandard::BORDER_SIZE;
  return {titleX, titleY};
}

const std::pair<int, int> BorderModalStandard::getSubTitleLocation() const {
  // Subtitle goes below the title in the top rectangle
  int subTitleX = style.x + BorderModalStandard::TOP_LEFT_SQUARE_SIZE +
                  BorderModalStandard::BORDER_SIZE;                 // Same as title
  int subTitleY = style.y + BorderModalStandard::SUBTITLE_Y_OFFSET; // Below title
  return {subTitleX, subTitleY};
}

const std::pair<int, int> BorderModalStandard::getContentLocation() const {
  int contentX = style.x + BorderModalStandard::LEFT_BORDER_WIDTH;
  int contentY = style.y + BorderModalStandard::TOP_LEFT_SQUARE_SIZE;
  return {contentX, contentY};
}

void BorderModalStandard::build() {
  children.clear();

  // Calculate scaled dimensions
  int scaledWidth = static_cast<int>(style.width * style.scale);
  int scaledHeight = static_cast<int>(style.height * style.scale);

  // 1. A square in the top left
  auto topLeftSquare = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle squareStyle;
  squareStyle.x = style.x;
  squareStyle.y = style.y;
  squareStyle.width = BorderModalStandard::TOP_LEFT_SQUARE_SIZE;
  squareStyle.height = BorderModalStandard::TOP_LEFT_SQUARE_SIZE;
  squareStyle.scale = style.scale;
  topLeftSquare->setStyle(squareStyle);
  // Uses default OutsetRectangleProps (BorderModalStandard colors, borderSize = 4)
  topLeftSquare->setProps(OutsetRectangleProps{});
  children.push_back(std::move(topLeftSquare));

  // 2. A {style.width - TOP_LEFT_SQUARE_SIZE} x TOP_LEFT_SQUARE_SIZE rectangle on the top
  auto topRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle topStyle;
  topStyle.x = style.x + BorderModalStandard::TOP_LEFT_SQUARE_SIZE;
  topStyle.y = style.y;
  topStyle.width = scaledWidth - BorderModalStandard::TOP_LEFT_SQUARE_SIZE;
  topStyle.height = BorderModalStandard::TOP_LEFT_SQUARE_SIZE;
  topStyle.scale = 1.0f; // Already scaled in width calculation
  topRectangle->setStyle(topStyle);
  topRectangle->setProps(OutsetRectangleProps{});
  children.push_back(std::move(topRectangle));

  // 3. A LEFT_BORDER_WIDTH x {style.height - TOP_LEFT_SQUARE_SIZE} rectangle on the left
  auto leftRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle leftStyle;
  leftStyle.x = style.x;
  leftStyle.y = style.y + BorderModalStandard::TOP_LEFT_SQUARE_SIZE;
  leftStyle.width = BorderModalStandard::LEFT_BORDER_WIDTH;
  leftStyle.height = scaledHeight - BorderModalStandard::TOP_LEFT_SQUARE_SIZE;
  leftStyle.scale = 1.0f; // Already scaled in height calculation
  leftRectangle->setStyle(leftStyle);
  leftRectangle->setProps(OutsetRectangleProps{});
  children.push_back(std::move(leftRectangle));

  // 4. A RIGHT_BORDER_WIDTH x {style.height - TOP_LEFT_SQUARE_SIZE} rectangle on the
  // right with border size set to 0
  auto rightRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle rightStyle;
  rightStyle.x = style.x + scaledWidth - BorderModalStandard::BORDER_SIZE;
  rightStyle.y = style.y + BorderModalStandard::TOP_LEFT_SQUARE_SIZE;
  rightStyle.width = BorderModalStandard::BORDER_SIZE;
  rightStyle.height = scaledHeight - BorderModalStandard::TOP_LEFT_SQUARE_SIZE;
  rightStyle.scale = 1.0f; // Already scaled in calculations
  rightRectangle->setStyle(rightStyle);
  OutsetRectangleProps rightProps;
  rightProps.borderSize = 0;
  rightRectangle->setProps(rightProps);
  children.push_back(std::move(rightRectangle));

  // 5. A {style.width - LEFT_BORDER_WIDTH} x BOTTOM_BORDER_WIDTH rectangle on the bottom
  // with border size set to 0
  auto bottomRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle bottomStyle;
  bottomStyle.x = style.x + BorderModalStandard::LEFT_BORDER_WIDTH;
  bottomStyle.y = style.y + scaledHeight - BorderModalStandard::BORDER_SIZE;
  bottomStyle.width = scaledWidth - BorderModalStandard::LEFT_BORDER_WIDTH;
  bottomStyle.height = BorderModalStandard::BORDER_SIZE;
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
