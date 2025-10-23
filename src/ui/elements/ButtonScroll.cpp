#include "ButtonScroll.h"
#include "Quad.h"
#include "ui/colors.h"
#include <memory>

namespace ui {

class ButtonScrollDefaultObserver : public UiEventObserver {
  ButtonScroll* buttonScroll;

public:
  ButtonScrollDefaultObserver(ButtonScroll* _buttonScroll) : buttonScroll(_buttonScroll) {}
  ~ButtonScrollDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override { buttonScroll->isActive = true; }
  void onMouseUp(int x, int y, int button) override { buttonScroll->isActive = false; }
};

ButtonScroll::ButtonScroll(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(std::make_unique<ButtonScrollDefaultObserver>(this));
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

  auto q = std::make_unique<Quad>(window);
  BaseStyle quadStyle;
  quadStyle.x = style.x;
  quadStyle.y = style.y;
  quadStyle.width = style.width;
  quadStyle.height = style.height;
  quadStyle.scale = style.scale;
  q->setStyle(quadStyle);
  QuadProps quadProps;

  // Change background color based on hover state
  if (isActive) {
    quadProps.bgColor = Colors::ButtonModalGrey3;
  } else if (isHovered) {
    quadProps.bgColor = Colors::ButtonModalGrey2;
  } else {
    quadProps.bgColor = Colors::ButtonModalGrey1;
  }

  quadProps.borderColor = Colors::ButtonModalGrey2;
  quadProps.borderSize = 4;
  q->setProps(quadProps);

  children.push_back(std::move(q));
}

void ButtonScroll::render() {
  if (isHovered) {
    if (!isInHoverMode) {
      isInHoverMode = true;
      build();
    }
  } else {
    if (isInHoverMode) {
      isInHoverMode = false;
      build();
    }
  }

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
  UiElement::render();
}

} // namespace ui
