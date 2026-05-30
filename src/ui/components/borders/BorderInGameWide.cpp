#include "BorderInGameWide.h"

namespace ui {

BorderInGameWide::BorderInGameWide(sdl2w::Window* _window, UiElement* _parent)
    : BorderInGame(_window, _parent) {}

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

const std::pair<int, int> BorderInGameWide::getPartyMemberAreaLocation() const {
  int partyX = style.x + style.width * style.scale -
               props.partyMemberAreaWidth * style.scale +
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

  int width = scaledWidth();
  int height = scaledHeight();
  int sideBorderY = style.y + props.titleHeight * style.scale;
  int sideBorderHeightScaled = height - props.titleHeight * style.scale;

  addOutsetRect(style.x, style.y, width / style.scale, props.titleHeight);

  addOutsetRect(style.x + width - props.partyMemberAreaWidth * style.scale,
                sideBorderY,
                props.partyMemberAreaWidth,
                sideBorderHeightScaled / style.scale);

  addOutsetRect(style.x,
                sideBorderY,
                props.leftBorderWidth,
                sideBorderHeightScaled / style.scale);

  addOutsetRect(style.x + props.leftBorderWidth * style.scale,
                style.y + height -
                    ACTION_BUTTON_SIZE * props.actionButtonsScale * style.scale -
                    props.outsetBorderSize * 2 * style.scale,
                width / style.scale - props.leftBorderWidth - props.partyMemberAreaWidth,
                ACTION_BUTTON_SIZE * props.actionButtonsScale + props.outsetBorderSize * 2);
}

} // namespace ui
