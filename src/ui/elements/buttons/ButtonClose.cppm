module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.ui.elements:ButtonClose;
export import carcer.ui.core;
import sdl2w;
import carcer.ui.core;
import :OutsetRectangle;
import bmin.string_interop;
#include "macros.h"

export {

// --- from ui/elements/buttons/ButtonClose.h ---
namespace ui {

// Close type enum for close buttons
enum class CloseType { MODAL, POPUP };

// ButtonClose-specific properties
struct ButtonCloseProps {
  CloseType closeType = CloseType::MODAL;
  int xLength = 13;
};

// ButtonClose element - renders a clickable button typically used to close a modal or
// popup window Uses Position, Size, Scale from BaseStyle
class ButtonClose : public UiElement {
private:
  ButtonCloseProps props;
  bool isInHoverMode = false;
  bool isInActiveMode = false;

public:
  bool isActive = false;
  constexpr static int closeButtonSize = 32;
  ButtonClose(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonClose() override = default;

  // Setters and getters for button-specific properties
  void setProps(const ButtonCloseProps& _props);
  ButtonCloseProps& getProps();
  const ButtonCloseProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

namespace ui {

class ButtonCloseDefaultObserver : public UiEventObserver {
  ButtonClose* buttonClose;

public:
  ButtonCloseDefaultObserver(ButtonClose* _buttonClose) : buttonClose(_buttonClose) {}
  ~ButtonCloseDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override { buttonClose->isActive = true; }
  void onMouseUp(int x, int y, int button) override { buttonClose->isActive = false; }
};

ButtonClose::ButtonClose(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(new ButtonCloseDefaultObserver(this));
  shouldPropagateEventsToChildren = false;

  style.width = closeButtonSize;
  style.height = closeButtonSize;
}

void ButtonClose::setProps(const ButtonCloseProps& _props) {
  props = _props;
  build();
}

ButtonCloseProps& ButtonClose::getProps() { return props; }

const ButtonCloseProps& ButtonClose::getProps() const { return props; }

void ButtonClose::build() {
  children.clear();

  style.width = closeButtonSize;
  style.height = closeButtonSize;

  auto rect = new OutsetRectangle(window);
  rect->setPos(style.x, style.y);
  rect->setScale(style.scale);

  OutsetRectangleProps rectProps;
  rectProps.width = closeButtonSize;
  rectProps.height = closeButtonSize;
  if (isInActiveMode) {
    rectProps.borderSize = 0;
  } else {
    rectProps.borderSize = 2;
  }
  if (props.closeType == CloseType::MODAL) {
    rectProps.color = Colors::ButtonCloseRed;
    rectProps.colorTopRight = Colors::ButtonCloseRedBorder1;
    rectProps.colorBottomLeft = Colors::ButtonCloseRedBorder2;
  } else if (props.closeType == CloseType::POPUP) {
    rectProps.color = Colors::White;
    rectProps.colorTopRight = Colors::Transparent;
    rectProps.colorBottomLeft = Colors::Transparent;
    rectProps.borderSize = 0;
  }
  rect->setProps(rectProps);

  children.pushBack(bmin::UniquePtr<UiElement>(rect));
}

void ButtonClose::render(int dt) {
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

  UiElement::render(dt);

  auto scaledX = static_cast<int>(style.x);
  auto scaledY = static_cast<int>(style.y);
  auto scaledWidth = static_cast<int>(style.width * style.scale);
  auto scaledHeight = static_cast<int>(style.height * style.scale);

  auto centerX = scaledX + scaledWidth / 2;
  auto centerY = scaledY + scaledHeight / 2;

  if (isInActiveMode) {
    centerX -= style.scale;
  }

  auto scaledLength = static_cast<int>(props.xLength * style.scale);
  auto color = Colors::ButtonCloseTextWhite;
  if (props.closeType == CloseType::POPUP) {
    color = Colors::ButtonCloseTextGrey;
  }

  auto& draw = window->getDraw();
  draw.drawLine({centerX - scaledLength / 2, centerY - scaledLength / 2},
                {centerX + scaledLength / 2, centerY + scaledLength / 2},
                style.scale,
                color);
  draw.drawLine({centerX + scaledLength / 2, centerY - scaledLength / 2},
                {centerX - scaledLength / 2, centerY + scaledLength / 2},
                style.scale,
                color);
}

} // namespace ui
