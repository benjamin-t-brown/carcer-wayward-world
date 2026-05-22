#include "BorderInGameNarrow.h"
#include "../elements/OutsetRectangle.h"
#include "BorderInGameWide.h"

namespace ui {

BorderInGameNarrow::BorderInGameNarrow(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void BorderInGameNarrow::setProps(const BorderInGameNarrowProps& _props) {
  props = _props;
  build();
}

BorderInGameNarrowProps& BorderInGameNarrow::getProps() { return props; }

const BorderInGameNarrowProps& BorderInGameNarrow::getProps() const { return props; }

const std::pair<int, int> BorderInGameNarrow::getContentAreaLocation() const {
  int contentX = style.x + props.sideBorderWidth * style.scale;
  int contentY = style.y + props.titleHeight * style.scale +
                 props.partyMemberAreaHeight * style.scale;
  return {contentX, contentY};
}

const std::pair<int, int> BorderInGameNarrow::getContentDims() const {
  return {(style.width - props.sideBorderWidth - props.sideBorderWidth) * style.scale,
          (style.height - props.titleHeight - props.partyMemberAreaHeight -
           ACTION_BUTTON_SIZE * props.actionButtonsScale - props.outsetBorderSize * 2) *
              style.scale};
}

const std::pair<int, int> BorderInGameNarrow::getTitleLocation() const {
  int titleX = style.x + props.outsetBorderSize * style.scale;
  int titleY = style.y + props.outsetBorderSize * style.scale;
  return {titleX, titleY};
}

const std::pair<int, int> BorderInGameNarrow::getTitleDims() const {
  return {style.width * style.scale - props.outsetBorderSize * 2 * style.scale,
          props.titleHeight * style.scale - props.outsetBorderSize * 2 * style.scale};
}

const std::pair<int, int> BorderInGameNarrow::getPartyMemberAreaLocation() const {
  int partyX = style.x + props.sideBorderWidth * style.scale;
  int partyY = style.y + props.titleHeight * style.scale +
               props.partyMemberAreaHeight * style.scale;
  return {partyX, partyY};
}

const std::pair<int, int> BorderInGameNarrow::getActionButtonsAreaLocation() const {
  int actionX = style.x + props.sideBorderWidth * style.scale +
                props.outsetBorderSize * style.scale;
  int actionY = style.y + style.height * style.scale -
                ACTION_BUTTON_SIZE * props.actionButtonsScale * style.scale +
                -props.outsetBorderSize * 2 * style.scale +
                props.outsetBorderSize * style.scale;
  return {actionX, actionY};
}

void BorderInGameNarrow::build() {
  children.clear();

  int scaledWidth = static_cast<int>(style.width * style.scale);
  int scaledHeight = static_cast<int>(style.height * style.scale);
  int sideBorderY = style.y + props.titleHeight * style.scale +
                    props.partyMemberAreaHeight * style.scale;
  int sideBorderHeightScaled = scaledHeight - props.titleHeight * style.scale -
                               props.partyMemberAreaHeight * style.scale;

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

  auto secondaryTopRectangle = new OutsetRectangle(window, this);
  auto& secondaryTopStyle = secondaryTopRectangle->getStyle();
  secondaryTopStyle.x = style.x;
  secondaryTopStyle.y = style.y + props.titleHeight * style.scale;
  secondaryTopStyle.width = scaledWidth / style.scale;
  secondaryTopStyle.height = props.partyMemberAreaHeight;
  secondaryTopStyle.scale = style.scale;
  secondaryTopRectangle->setProps(OutsetRectangleProps{
      .borderSize = props.outsetBorderSize,
  });
  addChild(secondaryTopRectangle);

  auto rightRectangle = new OutsetRectangle(window, this);
  auto& rightStyle = rightRectangle->getStyle();
  rightStyle.x = style.x + scaledWidth - props.sideBorderWidth * style.scale;
  rightStyle.y = sideBorderY;
  rightStyle.width = props.sideBorderWidth;
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
  leftStyle.width = props.sideBorderWidth;
  leftStyle.height = sideBorderHeightScaled / style.scale;
  leftStyle.scale = style.scale;
  leftRectangle->setProps(OutsetRectangleProps{
      .borderSize = props.outsetBorderSize,
  });
  addChild(leftRectangle);

  auto bottomRectangle = new OutsetRectangle(window, this);
  auto& bottomStyle = bottomRectangle->getStyle();
  bottomStyle.x = style.x + props.sideBorderWidth * style.scale;
  bottomStyle.y = style.y + scaledHeight -
                  ACTION_BUTTON_SIZE * props.actionButtonsScale * style.scale -
                  props.outsetBorderSize * 2 * style.scale;
  bottomStyle.width = scaledWidth / style.scale - props.sideBorderWidth * 2;
  bottomStyle.height =
      ACTION_BUTTON_SIZE * props.actionButtonsScale + props.outsetBorderSize * 2;
  bottomStyle.scale = style.scale;
  bottomRectangle->setProps(OutsetRectangleProps{
      .borderSize = props.outsetBorderSize,
  });
  addChild(bottomRectangle);
}

void BorderInGameNarrow::render(int dt) { UiElement::render(dt); }

} // namespace ui
