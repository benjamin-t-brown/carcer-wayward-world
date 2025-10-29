#include "ButtonModal.h"
#include "Quad.h"
#include "TextLine.h"
#include "ui/colors.h"
#include <memory>

namespace ui {

class ButtonModalDefaultObserver : public UiEventObserver {
  ButtonModal* buttonModal;

public:
  ButtonModalDefaultObserver(ButtonModal* _buttonModal) : buttonModal(_buttonModal) {}
  ~ButtonModalDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override { buttonModal->isActive = true; }
  void onMouseUp(int x, int y, int button) override { buttonModal->isActive = false; }
};

ButtonModal::ButtonModal(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(std::make_unique<ButtonModalDefaultObserver>(this));
  style.textAlign = TextAlign::CENTER;
  style.fontSize = sdl2w::TEXT_SIZE_20;
  style.fontColor = Colors::White;
  shouldPropagateEventsToChildren = false;
}

void ButtonModal::setProps(const ButtonModalProps& _props) {
  props = _props;
  build();
}

ButtonModalProps& ButtonModal::getProps() { return props; }

const ButtonModalProps& ButtonModal::getProps() const { return props; }

void ButtonModal::build() {
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

  auto textLine = std::make_unique<TextLine>(window, this);
  BaseStyle textStyle;
  textStyle.x = style.width / 2;
  textStyle.y = style.height / 2;
  textStyle.textAlign = style.textAlign;
  textStyle.fontSize = style.fontSize;
  textStyle.fontColor = style.fontColor;
  textStyle.textAlign = TextAlign::CENTER;
  textStyle.scale = style.scale;
  textLine->setStyle(textStyle);
  TextLineProps textLineProps;
  textLineProps.textBlocks = {TextBlock{props.text}};
  textLine->setProps(textLineProps);
  q->getChildren().push_back(std::move(textLine));

  children.push_back(std::move(q));
}

void ButtonModal::render(int dt) {
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
  UiElement::render(dt);
}

} // namespace ui