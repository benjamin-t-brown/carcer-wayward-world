#include "ButtonClose.h"
#include "Quad.h"
#include "TextLine.h"
#include "ui/colors.h"
#include <memory>

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
  addEventObserver(std::make_unique<ButtonCloseDefaultObserver>(this));
  style.textAlign = TextAlign::CENTER;
  style.fontSize = sdl2w::TEXT_SIZE_20;
  shouldPropagateEventsToChildren = false;

  // Set default size to 32x32px
  style.width = 32;
  style.height = 32;
}

void ButtonClose::setProps(const ButtonCloseProps& _props) {
  props = _props;
  build();
}

ButtonCloseProps& ButtonClose::getProps() { return props; }

const ButtonCloseProps& ButtonClose::getProps() const { return props; }

void ButtonClose::build() {
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

  // Style based on closeType
  if (props.closeType == CloseType::MODAL) {
    // Modal: red square with white X
    if (isActive) {
      quadProps.bgColor = Colors::ButtonCloseRedActive;
    } else if (isHovered) {
      quadProps.bgColor = Colors::ButtonCloseRedHover;
    } else {
      quadProps.bgColor = Colors::ButtonCloseRed;
    }
    quadProps.borderColor = Colors::ButtonCloseRed;
    quadProps.borderSize = 0; // No border for modal
  } else {
    if (isActive) {
      quadProps.bgColor = SDL_Color{0, 0, 100, 75};
    } else if (isHovered) {
      quadProps.bgColor = SDL_Color{0, 0, 100, 35};
    } else {
      quadProps.bgColor = Colors::Transparent;
    }
    // Popup: transparent square with grey X
    // quadProps.bgColor = Colors::Transparent;
    quadProps.borderColor = Colors::ButtonCloseTextGrey;
    quadProps.borderSize = 0;
  }

  q->setProps(quadProps);

  // Add X text
  auto textLine = std::make_unique<TextLine>(window, this);
  TextLineProps textLineProps;
  textLineProps.textBlocks = {TextBlock{"X"}};
  textLine->setProps(textLineProps);
  BaseStyle textStyle;
  textStyle.x = style.width / 2 - 0;
  textStyle.y = style.height / 2 - 2;
  textStyle.textAlign = TextAlign::CENTER;
  textStyle.fontSize = style.fontSize;
  textStyle.fontFamily = FontFamily::PARAGRAPH;
  textStyle.scale = style.scale;

  // Set text color based on closeType
  if (props.closeType == CloseType::MODAL) {
    textStyle.fontColor = Colors::ButtonCloseTextWhite;
  } else {
    if (isActive) {
      textStyle.fontColor = Colors::ButtonModalGrey3;
    } else if (isHovered) {
      textStyle.fontColor = Colors::ButtonModalGrey2;
    } else {
      textStyle.fontColor = Colors::ButtonCloseTextGrey;
    }
  }

  textLine->setStyle(textStyle);

  q->getChildren().push_back(std::move(textLine));

  children.push_back(std::move(q));
}

void ButtonClose::render(int dt) {
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

  UiElement::render(dt);
}

} // namespace ui
