#include "BorderModalStandard.h"
#include "ui/elements/ButtonClose.h"
#include "ui/elements/OutsetRectangle.h"

namespace ui {

BorderModalStandard::BorderModalStandard(sdl2w::Window* _window, UiElement* _parent)
    : BorderModalSmall(_window, _parent) {}

// void BorderModalStandard::setProps(const BorderModalStandardProps& _props) {
//   props = _props;
//   build();
// }

// BorderModalStandardProps& BorderModalStandard::getProps() { return props; }

// const BorderModalStandardProps& BorderModalStandard::getProps() const { return props; }

const std::pair<int, int> BorderModalStandard::getContentDims() const {
  auto [scaledWidth, scaledHeight] = getDims();
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  int scaledHeaderHeight = static_cast<int>(props.headerHeight * style.scale);
  return {scaledWidth - scaledBorderWidth * 2,
          scaledHeight - scaledBorderWidth * 2 - scaledHeaderHeight -
              BOTTOM_BORDER_HEIGHT * style.scale};
}

const std::pair<int, int> BorderModalStandard::getTitleLocation() const {
  auto scaledBorder = static_cast<int>(props.borderWidth * style.scale);
  auto locX = style.x + scaledBorder + props.headerHeight + 4 + 4;
  auto locY = style.y + scaledBorder + props.headerHeight / 4;
  return {locX, locY};
}

const std::pair<int, int> BorderModalStandard::getCloseButtonLocation() const {
  auto [scaledWidth, scaledHeight] = getDims();
  OutsetRectangleProps outsetRectProps;
  ButtonClose buttonClose(window, nullptr);
  int innerBorderSize = outsetRectProps.borderSize;
  int closeButtonSize = buttonClose.getStyle().width;
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  return {style.x + scaledWidth - scaledBorderWidth - closeButtonSize * style.scale -
              innerBorderSize * style.scale,
          style.y + scaledBorderWidth + innerBorderSize * style.scale};
}

const std::pair<int, int> BorderModalStandard::getContentLoc() const {
  auto scaledBorder = static_cast<int>(props.borderWidth * style.scale);
  int scaledHeaderHeight = static_cast<int>(props.headerHeight * style.scale);
  return {style.x + scaledBorder, style.y + scaledBorder + scaledHeaderHeight};
}

void BorderModalStandard::build() {
  children.clear();

  auto [scaledWidth, scaledHeight] = getDims();
  auto scaledBorder = static_cast<int>(props.borderWidth * style.scale);
  // auto scaledHeaderHeight = static_cast<int>(props.headerHeight * style.scale);

  auto iconOutsetRect = new OutsetRectangle(window, this);
  auto& iconOutsetRectStyle = iconOutsetRect->getStyle();
  iconOutsetRectStyle.x = style.x + scaledBorder;
  iconOutsetRectStyle.y = style.y + scaledBorder;
  iconOutsetRectStyle.width = props.headerHeight;
  iconOutsetRectStyle.height = props.headerHeight;
  iconOutsetRectStyle.scale = style.scale;
  iconOutsetRect->setProps(OutsetRectangleProps{});
  iconOutsetRect->setId("iconOutsetRect");
  children.push_back(std::unique_ptr<UiElement>(iconOutsetRect));

  auto topBarOutsetRect = new OutsetRectangle(window, this);
  auto& topBarOutsetRectStyle = topBarOutsetRect->getStyle();
  topBarOutsetRectStyle.x = iconOutsetRectStyle.x + iconOutsetRectStyle.width;
  topBarOutsetRectStyle.y = iconOutsetRectStyle.y;
  topBarOutsetRectStyle.width =
      scaledWidth - props.borderWidth * 2 - iconOutsetRectStyle.width;
  topBarOutsetRectStyle.height = props.headerHeight / 2;
  topBarOutsetRectStyle.scale = style.scale;
  topBarOutsetRect->setProps(OutsetRectangleProps{});
  children.push_back(std::unique_ptr<UiElement>(topBarOutsetRect));

  auto bottomBarOutsetRect = new OutsetRectangle(window, this);
  auto& bottomBarOutsetRectStyle = bottomBarOutsetRect->getStyle();
  bottomBarOutsetRectStyle.x = style.x + scaledBorder;
  bottomBarOutsetRectStyle.y =
      style.y + scaledHeight - scaledBorder - BOTTOM_BORDER_HEIGHT * style.scale;
  bottomBarOutsetRectStyle.width = scaledWidth - props.borderWidth * 2;
  bottomBarOutsetRectStyle.height = BOTTOM_BORDER_HEIGHT;
  bottomBarOutsetRectStyle.scale = style.scale;
  bottomBarOutsetRect->setProps(OutsetRectangleProps{});
  children.push_back(std::unique_ptr<UiElement>(bottomBarOutsetRect));
}

void BorderModalStandard::render(int dt) {
  BorderModalSmall::render(dt);
  // auto& draw = window->getDraw();
}

} // namespace ui
