module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.ui.BorderInGameNarrow;
export import carcer.ui.BorderInGame;
import sdl2w;
import carcer.ui.core;
import bmin.string_interop;
#include "macros.h"

export {

// --- from ui/components/borders/BorderInGameNarrow.h ---
namespace ui {

struct BorderInGameNarrowProps : BorderInGameProps {
  int width = 0;
  int height = 0;
  int partyMemberAreaHeight = 72;
  int sideBorderWidth = 16;
};

// BorderInGameNarrow component - renders a narrow in-game border layout
class BorderInGameNarrow : public BorderInGame {
private:
  BorderInGameNarrowProps props;

protected:
  const BorderInGameProps& inGameProps() const override { return props; }

public:
  BorderInGameNarrow(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderInGameNarrow() override = default;

  void setProps(const BorderInGameNarrowProps& _props);
  BorderInGameNarrowProps& getProps();
  const BorderInGameNarrowProps& getProps() const;

  const std::pair<int, int> getContentAreaLocation() const override;
  const std::pair<int, int> getContentDims() const override;
  const std::pair<int, int> getPartyMemberAreaLocation() const override;
  const std::pair<int, int> getActionButtonsAreaLocation() const override;

  void build() override;
};

} // namespace ui

} // export

namespace ui {

BorderInGameNarrow::BorderInGameNarrow(sdl2w::Window* _window, UiElement* _parent)
    : BorderInGame(_window, _parent) {}

void BorderInGameNarrow::setProps(const BorderInGameNarrowProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
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

const std::pair<int, int> BorderInGameNarrow::getPartyMemberAreaLocation() const {
  int partyX = style.x + props.outsetBorderSize * style.scale;
  auto [titleX, titleY] = getTitleLocation();
  auto [titleWidth, titleHeight] = getTitleDims();
  int partyY = titleY + titleHeight + props.outsetBorderSize * 2.f * style.scale;
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

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  int width = scaledWidth();
  int height = scaledHeight();
  int sideBorderY = style.y + props.titleHeight * style.scale +
                    props.partyMemberAreaHeight * style.scale;
  int sideBorderHeightScaled = height - props.titleHeight * style.scale -
                               props.partyMemberAreaHeight * style.scale;

  addOutsetRect(style.x, style.y, width / style.scale, props.titleHeight);

  addOutsetRect(style.x,
                style.y + props.titleHeight * style.scale,
                width / style.scale,
                props.partyMemberAreaHeight);

  addOutsetRect(style.x + width - props.sideBorderWidth * style.scale,
                sideBorderY,
                props.sideBorderWidth,
                sideBorderHeightScaled / style.scale);

  addOutsetRect(style.x,
                sideBorderY,
                props.sideBorderWidth,
                sideBorderHeightScaled / style.scale);

  addOutsetRect(style.x + props.sideBorderWidth * style.scale,
                style.y + height -
                    ACTION_BUTTON_SIZE * props.actionButtonsScale * style.scale -
                    props.outsetBorderSize * 2 * style.scale,
                width / style.scale - props.sideBorderWidth * 2,
                ACTION_BUTTON_SIZE * props.actionButtonsScale + props.outsetBorderSize * 2);
}

} // namespace ui
