#include "BorderInGame.h"
#include "../elements/OutsetRectangle.h"

namespace ui {

BorderInGame::BorderInGame(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  // Component doesn't need special initialization
}

const std::pair<int, int> BorderInGame::getDims() const {
  return {style.width, style.height};
}

const std::pair<int, int> BorderInGame::getContentDims() const {
  return {style.width - LEFT_BORDER_WIDTH - PARTY_MEMBER_AREA_WIDTH, style.height - TITLE_HEIGHT - ACTION_BUTTONS_AREA_HEIGHT};
}

const std::pair<int, int> BorderInGame::getTitleLocation() const {
  // Title goes in the top rectangle, left-aligned with some padding
  int titleX = style.x + OUTSET_BORDER_SIZE; // Left edge + padding
  int titleY = style.y + OUTSET_BORDER_SIZE; // Top edge + padding
  return {titleX, titleY};
}

const std::pair<int, int> BorderInGame::getSubTitleLocation() const {
  // Subtitle goes below the title in the top rectangle
  int subTitleX = style.x + style.width / 2; // Same as title
  int subTitleY = style.y + OUTSET_BORDER_SIZE; // Below title
  return {subTitleX, subTitleY};
}

const std::pair<int, int> BorderInGame::getPartyMemberAreaLocation() const {
  // Party member area is above the right border
  int partyX = style.x + style.width - PARTY_MEMBER_AREA_WIDTH - OUTSET_BORDER_SIZE; // Right edge of content area - padding
  int partyY = style.y + TITLE_HEIGHT + OUTSET_BORDER_SIZE; // Below top rectangle + padding
  return {partyX, partyY};
}

const std::pair<int, int> BorderInGame::getActionButtonsAreaLocation() const {
  // Action buttons area is at the bottom border
  int actionX = style.x + LEFT_BORDER_WIDTH + OUTSET_BORDER_SIZE; // Left edge of bottom rectangle + padding
  int actionY = style.y + style.height - ACTION_BUTTONS_AREA_HEIGHT + OUTSET_BORDER_SIZE; // Top edge of bottom rectangle + padding
  return {actionX, actionY};
}

const std::pair<int, int> BorderInGame::getContentAreaLocation() const {
  // Content area is the main area inside the border, with padding
  int contentX = style.x + LEFT_BORDER_WIDTH; // Left border + padding
  int contentY = style.y + TITLE_HEIGHT; // Below top rectangle + padding
  return {contentX, contentY};
}

void BorderInGame::build() {
  children.clear();

  // Calculate scaled dimensions
  int scaledWidth = static_cast<int>(style.width * style.scale);
  int scaledHeight = static_cast<int>(style.height * style.scale);

  // 1. Top rectangle: {style.width} by 44px
  auto topRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle topStyle;
  topStyle.x = style.x;
  topStyle.y = style.y;
  topStyle.width = scaledWidth;
  topStyle.height = TITLE_HEIGHT;
  topStyle.scale = 1.0f; // Already scaled in width calculation
  topRectangle->setStyle(topStyle);
  topRectangle->setProps(OutsetRectangleProps{});
  children.push_back(std::move(topRectangle));

  // 2. Right rectangle: 84px by {style.height - 44px}
  auto rightRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle rightStyle;
  rightStyle.x = style.x + scaledWidth - PARTY_MEMBER_AREA_WIDTH;
  rightStyle.y = style.y + TITLE_HEIGHT;
  rightStyle.width = PARTY_MEMBER_AREA_WIDTH;
  rightStyle.height = scaledHeight - TITLE_HEIGHT;
  rightStyle.scale = 1.0f; // Already scaled in height calculation
  rightRectangle->setStyle(rightStyle);
  rightRectangle->setProps(OutsetRectangleProps{});
  children.push_back(std::move(rightRectangle));

  // 3. Left rectangle: 16px by {style.height - 44px}
  auto leftRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle leftStyle;
  leftStyle.x = style.x;
  leftStyle.y = style.y + TITLE_HEIGHT;
  leftStyle.width = LEFT_BORDER_WIDTH;
  leftStyle.height = scaledHeight - TITLE_HEIGHT;
  leftStyle.scale = 1.0f; // Already scaled in height calculation
  leftRectangle->setStyle(leftStyle);
  leftRectangle->setProps(OutsetRectangleProps{});
  children.push_back(std::move(leftRectangle));

  // 4. Bottom rectangle: {style.width - 16px} by 58px
  auto bottomRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle bottomStyle;
  bottomStyle.x = style.x + LEFT_BORDER_WIDTH;
  bottomStyle.y = style.y + scaledHeight - ACTION_BUTTONS_AREA_HEIGHT;
  bottomStyle.width = scaledWidth - LEFT_BORDER_WIDTH - PARTY_MEMBER_AREA_WIDTH;
  bottomStyle.height = ACTION_BUTTONS_AREA_HEIGHT;
  bottomStyle.scale = 1.0f; // Already scaled in calculations
  bottomRectangle->setStyle(bottomStyle);
  bottomRectangle->setProps(OutsetRectangleProps{});
  children.push_back(std::move(bottomRectangle));
}

void BorderInGame::render(int dt) {
  UiElement::render(dt);
}

} // namespace ui
