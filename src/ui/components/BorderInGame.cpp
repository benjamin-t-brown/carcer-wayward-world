#include "BorderInGame.h"
#include "../elements/OutsetRectangle.h"

namespace ui {

BorderInGame::BorderInGame(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void BorderInGame::setProps(const BorderInGameProps& _props) {
  props = _props;
  build();
}

BorderInGameProps& BorderInGame::getProps() { return props; }

const BorderInGameProps& BorderInGame::getProps() const { return props; }

const std::pair<int, int> BorderInGame::getDims() const {
  return {style.width, style.height};
}

const std::pair<int, int> BorderInGame::getContentDims() const {
  return {style.width - props.leftBorderWidth - props.partyMemberAreaWidth,
          style.height - props.titleHeight - props.actionButtonsAreaHeight};
}

const std::pair<int, int> BorderInGame::getTitleLocation() const {
  int titleX = style.x + props.outsetBorderSize;
  int titleY = style.y + props.outsetBorderSize;
  return {titleX, titleY};
}

const std::pair<int, int> BorderInGame::getSubTitleLocation() const {
  int subTitleX = style.x + style.width / 2;
  int subTitleY = style.y + props.outsetBorderSize;
  return {subTitleX, subTitleY};
}

const std::pair<int, int> BorderInGame::getPartyMemberAreaLocation() const {
  int partyX =
      style.x + style.width - props.partyMemberAreaWidth - props.outsetBorderSize;
  int partyY = style.y + props.titleHeight + props.outsetBorderSize;
  return {partyX, partyY};
}

const std::pair<int, int> BorderInGame::getActionButtonsAreaLocation() const {
  int actionX = style.x + props.leftBorderWidth + props.outsetBorderSize;
  int actionY =
      style.y + style.height - props.actionButtonsAreaHeight + props.outsetBorderSize;
  return {actionX, actionY};
}

const std::pair<int, int> BorderInGame::getContentAreaLocation() const {
  int contentX = style.x + props.leftBorderWidth;
  int contentY = style.y + props.titleHeight;
  return {contentX, contentY};
}

void BorderInGame::build() {
  children.clear();

  int scaledWidth = static_cast<int>(style.width * style.scale);
  int scaledHeight = static_cast<int>(style.height * style.scale);

  auto topRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle topStyle;
  topStyle.x = style.x;
  topStyle.y = style.y;
  topStyle.width = scaledWidth;
  topStyle.height = props.titleHeight;
  topStyle.scale = 1.0f;
  topRectangle->setStyle(topStyle);
  topRectangle->setProps(OutsetRectangleProps{});
  children.push_back(std::move(topRectangle));

  auto rightRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle rightStyle;
  rightStyle.x = style.x + scaledWidth - props.partyMemberAreaWidth;
  rightStyle.y = style.y + props.titleHeight;
  rightStyle.width = props.partyMemberAreaWidth;
  rightStyle.height = scaledHeight - props.titleHeight;
  rightStyle.scale = 1.0f;
  rightRectangle->setStyle(rightStyle);
  rightRectangle->setProps(OutsetRectangleProps{});
  children.push_back(std::move(rightRectangle));

  auto leftRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle leftStyle;
  leftStyle.x = style.x;
  leftStyle.y = style.y + props.titleHeight;
  leftStyle.width = props.leftBorderWidth;
  leftStyle.height = scaledHeight - props.titleHeight;
  leftStyle.scale = 1.0f;
  leftRectangle->setStyle(leftStyle);
  leftRectangle->setProps(OutsetRectangleProps{});
  children.push_back(std::move(leftRectangle));

  auto bottomRectangle = std::make_unique<OutsetRectangle>(window, this);
  BaseStyle bottomStyle;
  bottomStyle.x = style.x + props.leftBorderWidth;
  bottomStyle.y = style.y + scaledHeight - props.actionButtonsAreaHeight;
  bottomStyle.width = scaledWidth - props.leftBorderWidth - props.partyMemberAreaWidth;
  bottomStyle.height = props.actionButtonsAreaHeight;
  bottomStyle.scale = 1.0f;
  bottomRectangle->setStyle(bottomStyle);
  bottomRectangle->setProps(OutsetRectangleProps{});
  children.push_back(std::move(bottomRectangle));
}

void BorderInGame::render(int dt) {
  UiElement::render(dt);
}

} // namespace ui
