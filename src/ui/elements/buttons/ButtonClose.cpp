#include "ButtonClose.h"
#include "ui/colors.h"
#include "ui/elements/OutsetRectangle.h"

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
  style.textAlign = TextAlign::CENTER;
  style.fontSize = sdl2w::TEXT_SIZE_20;
  shouldPropagateEventsToChildren = false;

  // Set default size to 32x32px
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

  auto rect = new OutsetRectangle(window);
  auto& rectStyle = rect->getStyle();
  rectStyle.x = style.x;
  rectStyle.y = style.y;
  rectStyle.width = style.width;
  rectStyle.height = style.height;
  rectStyle.scale = style.scale;

  auto& rectProps = rect->getProps();
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

  children.pushBack(UniquePtr<UiElement>(rect));
}

void ButtonClose::render(int dt) {
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
