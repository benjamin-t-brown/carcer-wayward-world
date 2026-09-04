module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.ui.BorderInGameWide;
export import carcer.ui.BorderInGame;
import sdl2w;
import carcer.ui.core;
import bmin.string_interop;
#include "macros.h"

export {

// --- from ui/components/borders/BorderInGameWide.h ---
namespace ui {

struct BorderInGameWideProps : BorderInGameProps {
  int width = 0;
  int height = 0;
  int subtitleHeight = 24;
  int partyMemberAreaWidth = 76;
  int leftBorderWidth = 16;
};

// BorderInGameWide component - renders a wide in-game border layout
class BorderInGameWide : public BorderInGame {
private:
  BorderInGameWideProps props;

protected:
  const BorderInGameProps& inGameProps() const override { return props; }

public:
  BorderInGameWide(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~BorderInGameWide() override = default;

  void setProps(const BorderInGameWideProps& _props);
  BorderInGameWideProps& getProps();
  const BorderInGameWideProps& getProps() const;

  const std::pair<int, int> getContentAreaLocation() const override;
  const std::pair<int, int> getContentDims() const override;
  const std::pair<int, int> getPartyMemberAreaLocation() const override;
  const std::pair<int, int> getActionButtonsAreaLocation() const override;

  void build() override;
};

} // namespace ui

} // export

namespace ui {

BorderInGameWide::BorderInGameWide(sdl2w::Window* _window, UiElement* _parent)
    : BorderInGame(_window, _parent) {}

void BorderInGameWide::setProps(const BorderInGameWideProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
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

  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

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
