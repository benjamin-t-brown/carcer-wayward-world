#include "BorderInGameWide.h"
#include "../../elements/OutsetRectangle.h"

namespace ui {

BorderInGameWide::BorderInGameWide(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void BorderInGameWide::setProps(const BorderInGameWideProps& _props) {
  props = _props;
  build();
}

BorderInGameWideProps& BorderInGameWide::getProps() { return props; }

const BorderInGameWideProps& BorderInGameWide::getProps() const { return props; }

const std::pair<int, int> BorderInGameWide::getContentAreaLocation() const {
  int contentX = style.x + props.leftBorderWidth * style.scale;
  int contentY = style.y + props.titleHeight * style.scale;
  return {contentX, contentY};
}

const std::pair<int, int> BorderInGameWide::getContentDims() const {
  return {(style.width - props.leftBorderWidth - props.partyMemberAreaWidth) *
              style.scale,
          (style.height - props.titleHeight -
           ACTION_BUTTON_SIZE * props.actionButtonsScale - props.outsetBorderSize * 2) *
              style.scale};
}

const std::pair<int, int> BorderInGameWide::getTitleLocation() const {
  int titleX = style.x + props.outsetBorderSize * style.scale;
  int titleY = style.y + props.outsetBorderSize * style.scale;
  return {titleX, titleY};
}

const std::pair<int, int> BorderInGameWide::getTitleDims() const {
  return {style.width * style.scale - props.outsetBorderSize * 2 * style.scale,
          props.titleHeight * style.scale - props.outsetBorderSize * 2 * style.scale};
}

const std::pair<int, int> BorderInGameWide::getPartyMemberAreaLocation() const {
  int partyX = style.x + style.width * style.scale -
               props.partyMemberAreaWidth * style.scale -
               props.outsetBorderSize * style.scale;
  int partyY =
      style.y + props.titleHeight * style.scale + props.outsetBorderSize * style.scale;
  return {partyX, partyY};
}

const std::pair<int, int> BorderInGameWide::getActionButtonsAreaLocation() const {
  int actionX = style.x + props.leftBorderWidth * style.scale +
                props.outsetBorderSize * style.scale;
  int actionY = style.y + style.height * style.scale -
                ACTION_BUTTON_SIZE * props.actionButtonsScale * style.scale -
                props.outsetBorderSize * 2 * style.scale +
                props.outsetBorderSize * style.scale;
  return {actionX, actionY};
}

void BorderInGameWide::build() {
  children.clear();

  int scaledWidth = static_cast<int>(style.width * style.scale);
  int scaledHeight = static_cast<int>(style.height * style.scale);
  int sideBorderY = style.y + props.titleHeight * style.scale;
  int sideBorderHeightScaled = scaledHeight - props.titleHeight * style.scale;

  auto topRectangle = new OutsetRectangle(window, this);
  auto& topStyle = topRectangle->getStyle();
  topStyle.x = style.x;
  topStyle.y = style.y;
  topStyle.width = scaledWidth / style.scale;
  topStyle.height = props.titleHeight;
  topStyle.scale = style.scale;
  topRectangle->setProps(OutsetRectangleProps{
      .borderSize = props.outsetBorderSize,
  });
  addChild(topRectangle);

  auto rightRectangle = new OutsetRectangle(window, this);
  auto& rightStyle = rightRectangle->getStyle();
  rightStyle.x = style.x + scaledWidth - props.partyMemberAreaWidth * style.scale;
  rightStyle.y = sideBorderY;
  rightStyle.width = props.partyMemberAreaWidth;
  rightStyle.height = sideBorderHeightScaled / style.scale;
  rightStyle.scale = style.scale;
  rightRectangle->setProps(OutsetRectangleProps{
      .borderSize = props.outsetBorderSize,
  });
  addChild(rightRectangle);

  auto leftRectangle = new OutsetRectangle(window, this);
  auto& leftStyle = leftRectangle->getStyle();
  leftStyle.x = style.x;
  leftStyle.y = sideBorderY;
  leftStyle.width = props.leftBorderWidth;
  leftStyle.height = sideBorderHeightScaled / style.scale;
  leftStyle.scale = style.scale;
  leftRectangle->setProps(OutsetRectangleProps{
      .borderSize = props.outsetBorderSize,
  });
  addChild(leftRectangle);

  auto bottomRectangle = new OutsetRectangle(window, this);
  auto& bottomStyle = bottomRectangle->getStyle();
  bottomStyle.x = style.x + props.leftBorderWidth * style.scale;
  bottomStyle.y = style.y + scaledHeight -
                  ACTION_BUTTON_SIZE * props.actionButtonsScale * style.scale -
                  props.outsetBorderSize * 2 * style.scale;
  bottomStyle.width =
      scaledWidth / style.scale - props.leftBorderWidth - props.partyMemberAreaWidth;
  bottomStyle.height =
      ACTION_BUTTON_SIZE * props.actionButtonsScale + props.outsetBorderSize * 2;
  bottomStyle.scale = style.scale;
  bottomRectangle->setProps(OutsetRectangleProps{
      .borderSize = props.outsetBorderSize,
  });
  addChild(bottomRectangle);
}

void BorderInGameWide::render(int dt) { UiElement::render(dt); }

} // namespace ui
