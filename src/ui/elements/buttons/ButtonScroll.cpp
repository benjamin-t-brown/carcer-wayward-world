#include "ButtonScroll.h"
#include "ui/colors.h"
#include "ui/elements/OutsetRectangle.h"
#include <algorithm>

namespace ui {

class ButtonScrollDefaultObserver : public UiEventObserver {
  ButtonScroll* buttonScroll;

public:
  ButtonScrollDefaultObserver(ButtonScroll* _buttonScroll)
      : buttonScroll(_buttonScroll) {}
  ~ButtonScrollDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override { buttonScroll->isActive = true; }
  void onMouseUp(int x, int y, int button) override { buttonScroll->isActive = false; }
};

ButtonScroll::ButtonScroll(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(new ButtonScrollDefaultObserver(this));
  shouldPropagateEventsToChildren = false;

  // Set default size to 32x32px
  style.width = 32;
  style.height = 32;
}

void ButtonScroll::setProps(const ButtonScrollProps& _props) {
  props = _props;
  build();
}

ButtonScrollProps& ButtonScroll::getProps() { return props; }

const ButtonScrollProps& ButtonScroll::getProps() const { return props; }

void ButtonScroll::build() {
  children.clear();

  auto rect = new OutsetRectangle(window);
  auto& rectStyle = rect->getStyle();
  rectStyle.x = style.x;
  rectStyle.y = style.y;
  rectStyle.width = style.width;
  rectStyle.height = style.height;
  rectStyle.scale = style.scale;

  auto& rectProps = rect->getProps();
  if (props.isDisabled) {
    rectProps.borderSize = 0;
    rectProps.color = Colors::Grey;
    rectProps.colorTopRight = Colors::Grey;
    rectProps.colorBottomLeft = Colors::Grey;
  } else if (isInActiveMode) {
    rectProps.borderSize = 0;
    rectProps.color = Colors::ButtonModalGrey1;
    rectProps.colorTopRight = Colors::Colors::ButtonModalGrey2;
    rectProps.colorBottomLeft = Colors::Colors::ButtonModalGrey3;
  } else {
    rectProps.borderSize = 2;
    rectProps.color = Colors::ButtonModalGrey1;
    rectProps.colorTopRight = Colors::Colors::ButtonModalGrey2;
    rectProps.colorBottomLeft = Colors::Colors::ButtonModalGrey3;
  }

  rect->setProps(rectProps);

  children.pushBack(UniquePtr<UiElement>(rect));
}

void ButtonScroll::render(int dt) {
  // if (isHovered) {
  //   if (!isInHoverMode) {
  //     isInHoverMode = true;
  //     build();
  //   }
  // } else {
  //   if (isInHoverMode) {
  //     isInHoverMode = false;
  //     build();
  //   }
  // }

  if (!props.isDisabled) {
    if (isActive) {
      if (!isInActiveMode) {
        isInActiveMode = true;
        build();
      }
    } else {
      if (isInActiveMode) {
        isInActiveMode = false;
        build();
      }
    }
  } else if (isInActiveMode) {
    isInActiveMode = false;
    build();
  }

  if (props.isSelected) {
    auto& draw = window->getDraw();
    auto scaledWidth = static_cast<int>(style.width * style.scale);
    auto scaledHeight = static_cast<int>(style.height * style.scale);
    int borderSize = 2;

    draw.drawRect(style.x - borderSize,
                  style.y - borderSize,
                  scaledWidth + borderSize * 2,
                  scaledHeight + borderSize * 2,
                  Colors::ButtonModalSelected);
  }
  UiElement::render(dt);

  auto scaledX = static_cast<int>(style.x);
  auto scaledY = static_cast<int>(style.y);
  auto scaledWidth = static_cast<int>(style.width * style.scale);
  auto scaledHeight = static_cast<int>(style.height * style.scale);
  auto centerX = scaledX + scaledWidth / 2;
  auto centerY = scaledY + scaledHeight / 2;
  auto arrowLength = std::max(scaledWidth / 8, 4);
  if (isInActiveMode) {
    centerX -= style.scale;
  }

  if (props.direction == ScrollDirection::UP) {
    auto& draw = window->getDraw();
    draw.drawLine({centerX - arrowLength, centerY},
                  {centerX, centerY - arrowLength},
                  style.scale,
                  Colors::White);
    draw.drawLine({centerX, centerY + arrowLength},
                  {centerX, centerY - arrowLength},
                  style.scale,
                  Colors::White);
    draw.drawLine({centerX + arrowLength, centerY},
                  {centerX, centerY - arrowLength},
                  style.scale,
                  Colors::White);
  } else if (props.direction == ScrollDirection::DOWN) {
    auto& draw = window->getDraw();
    draw.drawLine({centerX - arrowLength, centerY},
                  {centerX, centerY + arrowLength},
                  style.scale,
                  Colors::White);
    draw.drawLine({centerX, centerY - arrowLength},
                  {centerX, centerY + arrowLength},
                  style.scale,
                  Colors::White);
    draw.drawLine({centerX + arrowLength, centerY},
                  {centerX, centerY + arrowLength},
                  style.scale,
                  Colors::White);
  } else if (props.direction == ScrollDirection::LEFT) {
    auto& draw = window->getDraw();
    draw.drawLine({centerX, centerY - arrowLength},
                  {centerX - arrowLength, centerY},
                  style.scale,
                  Colors::White);
    draw.drawLine({centerX + arrowLength, centerY},
                  {centerX - arrowLength, centerY},
                  style.scale,
                  Colors::White);
    draw.drawLine({centerX, centerY + arrowLength},
                  {centerX - arrowLength, centerY},
                  style.scale,
                  Colors::White);
  } else if (props.direction == ScrollDirection::RIGHT) {
    auto& draw = window->getDraw();
    draw.drawLine({centerX, centerY - arrowLength},
                  {centerX + arrowLength, centerY},
                  style.scale,
                  Colors::White);
    draw.drawLine({centerX - arrowLength, centerY},
                  {centerX + arrowLength, centerY},
                  style.scale,
                  Colors::White);
    draw.drawLine({centerX, centerY + arrowLength},
                  {centerX + arrowLength, centerY},
                  style.scale,
                  Colors::White);
  }
}

} // namespace ui
