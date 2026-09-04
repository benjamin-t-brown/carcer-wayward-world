module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <algorithm>

export module carcer.ui.elements:ButtonScroll;
export import carcer.ui.UiElement;
import sdl2w;
import carcer.ui.colors;
import :OutsetRectangle;
import bmin.string_interop;
#include "macros.h"

export {

// --- from ui/elements/buttons/ButtonScroll.h ---
namespace ui {

// Direction enum for scroll buttons
enum class ScrollDirection {
  UP,
  DOWN,
  LEFT,
  RIGHT,
};

// ButtonScroll-specific properties
struct ButtonScrollProps {
  ScrollDirection direction = ScrollDirection::UP;
  bool isSelected = false;
  bool isDisabled = false;
  int width = 32;
  int height = 32;
};

// ButtonScroll element - renders a square clickable button used to scroll windows up/down
// Uses Position, Size, Scale from BaseStyle
class ButtonScroll : public UiElement {
private:
  ButtonScrollProps props;
  bool isInHoverMode = false;
  bool isInActiveMode = false;

public:
  bool isActive = false;
  ButtonScroll(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonScroll() override = default;

  // Setters and getters for button-specific properties
  void setProps(const ButtonScrollProps& _props);
  ButtonScrollProps& getProps();
  const ButtonScrollProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

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

  style.width = props.width;
  style.height = props.height;

  auto rect = new OutsetRectangle(window);
  rect->setPos(style.x, style.y);
  rect->setScale(style.scale);

  OutsetRectangleProps rectProps;
  rectProps.width = props.width;
  rectProps.height = props.height;
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

  children.pushBack(bmin::UniquePtr<UiElement>(rect));
}

void ButtonScroll::render(int dt) {
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
