#include "BorderModalStandard.h"
#include "ui/elements/OutsetRectangle.h"
#include "ui/elements/buttons/ButtonClose.h"

namespace ui {

BorderModalStandard::BorderModalStandard(sdl2w::Window* _window, UiElement* _parent)
    : BorderModalSmall(_window, _parent) {}

const std::pair<int, int> BorderModalStandard::getContentDims() const {
  auto [scaledWidth, scaledHeight] = getDims();
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  int scaledHeaderHeight = static_cast<int>(props.headerHeight * style.scale);
  return {scaledWidth - scaledBorderWidth * 2,
          scaledHeight - scaledBorderWidth * 2 - scaledHeaderHeight -
              BOTTOM_BORDER_HEIGHT * style.scale};
}

const std::pair<int, int> BorderModalStandard::getContentLocation() const {
  auto scaledBorder = static_cast<int>(props.borderWidth * style.scale);
  int scaledHeaderHeight = static_cast<int>(props.headerHeight * style.scale);
  return {style.x + scaledBorder, style.y + scaledBorder + scaledHeaderHeight};
}

const std::pair<int, int> BorderModalStandard::getIconSectionCenter() const {
  auto scaledBorder = static_cast<int>(props.borderWidth * style.scale);
  const int sectionSize = static_cast<int>(props.headerHeight * style.scale);
  const int centerX = style.x + scaledBorder + sectionSize / 2;
  const int centerY = style.y + scaledBorder + sectionSize / 2;
  return {centerX, centerY};
}

const std::pair<int, int> BorderModalStandard::getTitleLocation() const {
  auto scaledBorder = static_cast<int>(props.borderWidth * style.scale);
  auto iconAreaSizeScaled = (props.headerHeight + 4 + 4) * style.scale;
  auto locX = style.x + scaledBorder + iconAreaSizeScaled;
  auto locY = style.y + scaledBorder +
              (static_cast<float>(props.headerHeight) / 4.f) * style.scale;
  return {locX, locY};
}

const std::pair<int, int> BorderModalStandard::getSubTitleLocation() const {
  auto scaledBorder = static_cast<int>(props.borderWidth * style.scale);
  auto iconAreaSizeScaled = (props.headerHeight + 4 + 4) * style.scale;
  auto locX = style.x + scaledBorder + iconAreaSizeScaled;
  auto locY = style.y + scaledBorder +
              (static_cast<float>(props.headerHeight) * 3.f / 4.f) * style.scale;
  return {locX, locY};
}

const std::pair<int, int> BorderModalStandard::getSubTitleDims() const {
  auto scaledBorder = static_cast<int>(props.borderWidth * style.scale);
  auto iconAreaSizeScaled = (props.headerHeight + 4 + 4) * style.scale;
  return {style.width * style.scale - scaledBorder * 2 - iconAreaSizeScaled,
          (static_cast<float>(props.headerHeight) / 2.f) * style.scale};
}

const std::pair<int, int> BorderModalStandard::getCloseButtonLocation() const {
  auto [scaledWidth, scaledHeight] = getDims();
  OutsetRectangleProps outsetRectProps;
  int innerBorderSize = outsetRectProps.borderSize;
  int closeButtonSize = ButtonClose::closeButtonSize;
  int scaledBorderWidth = static_cast<int>(props.borderWidth * style.scale);
  return {style.x + scaledWidth - scaledBorderWidth - closeButtonSize * style.scale -
              innerBorderSize * style.scale,
          style.y + scaledBorderWidth + innerBorderSize * style.scale};
}

void BorderModalStandard::build() {
  children.clear();

  auto [scaledWidth, scaledHeight] = getDims();
  auto scaledBorder = static_cast<int>(props.borderWidth * style.scale);

  auto iconOutsetRect = new OutsetRectangle(window, this);
  iconOutsetRect->setPos(style.x + scaledBorder, style.y + scaledBorder);
  iconOutsetRect->setScale(style.scale);
  iconOutsetRect->setProps(OutsetRectangleProps{
      .width = props.headerHeight,
      .height = props.headerHeight,
  });
  iconOutsetRect->setId("iconOutsetRect");
  addChild(iconOutsetRect);

  auto topBarOutsetRect = new OutsetRectangle(window, this);
  topBarOutsetRect->setPos(style.x + scaledBorder + props.headerHeight * style.scale,
                           style.y + scaledBorder);
  topBarOutsetRect->setScale(style.scale);
  topBarOutsetRect->setProps(OutsetRectangleProps{
      .width = static_cast<int>(scaledWidth / style.scale - props.borderWidth * 2 -
                                props.headerHeight),
      .height = props.headerHeight / 2,
  });
  addChild(topBarOutsetRect);

  auto bottomBarOutsetRect = new OutsetRectangle(window, this);
  bottomBarOutsetRect->setPos(
      style.x + scaledBorder,
      style.y + scaledHeight - scaledBorder - BOTTOM_BORDER_HEIGHT * style.scale);
  bottomBarOutsetRect->setScale(style.scale);
  bottomBarOutsetRect->setProps(OutsetRectangleProps{
      .width = static_cast<int>(scaledWidth / style.scale - props.borderWidth * 2),
      .height = BOTTOM_BORDER_HEIGHT,
  });
  addChild(bottomBarOutsetRect);
}

void BorderModalStandard::render(int dt) { BorderModalSmall::render(dt); }

} // namespace ui
