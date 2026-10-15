#include "ButtonModal.h"
#include "../TextLine.h"
#include "ui/colors.hpp"
#include "ui/elements/OutsetRectangle.h"
#include "ui/uiUtils.hpp"

namespace ui {

class ButtonModalDefaultObserver : public UiEventObserver {
  ButtonModal* buttonModal;

public:
  ButtonModalDefaultObserver(ButtonModal* _buttonModal) : buttonModal(_buttonModal) {}
  ~ButtonModalDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override {
    if (buttonModal->getProps().isDisabled) {
      return;
    }
    buttonModal->isActive = true;
  }
  void onMouseUp(int x, int y, int button) override {
    if (buttonModal->getProps().isDisabled) {
      return;
    }
    buttonModal->isActive = false;
  }
};

ButtonModal::ButtonModal(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(bmin::UniquePtr<ui::UiEventObserver>(new ButtonModalDefaultObserver(this)));
  playClickSound = true;
  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_BUTTON);
  props.fontFamily = font.fontFamily;
  props.fontSize = font.fontSize;
  props.fontColor = font.fontColor;
  shouldPropagateEventsToChildren = false;
}

void ButtonModal::setProps(const ButtonModalProps& _props) {
  props = _props;
  playClickSound = !props.isDisabled;
  build();
}

ButtonModalProps& ButtonModal::getProps() { return props; }

const ButtonModalProps& ButtonModal::getProps() const { return props; }

bool ButtonModal::checkMouseDownEvent(int mouseX,
                                      int mouseY,
                                      int button,
                                      bmin::DynArray<UiElement*> additionalElements) {
  if (props.isDisabled) {
    return isInBoundsScaled(mouseX, mouseY, this);
  }
  return UiElement::checkMouseDownEvent(mouseX, mouseY, button, additionalElements);
}

bool ButtonModal::checkMouseUpEvent(int mouseX,
                                    int mouseY,
                                    int button,
                                    bmin::DynArray<UiElement*> additionalElements) {
  if (props.isDisabled) {
    isClicked = false;
    return isInBoundsScaled(mouseX, mouseY, this);
  }
  return UiElement::checkMouseUpEvent(mouseX, mouseY, button, additionalElements);
}

void ButtonModal::build() {
  children.clear();

  style.width = props.width;
  style.height = props.height;

  auto rect = new OutsetRectangle(window);
  rect->setPos(style.x, style.y);
  rect->setScale(style.scale);

  OutsetRectangleProps rectProps;
  if (isInActiveMode && !props.isDisabled) {
    rectProps.borderSize = 0;
  } else {
    rectProps.borderSize = 2;
  }
  rectProps.width = props.width;
  rectProps.height = props.height;
  if (props.isDisabled) {
    rectProps.color = Colors::DarkGrey;
    rectProps.colorTopRight = Colors::Grey2;
    rectProps.colorBottomLeft = Colors::Charcoal;
  } else {
    rectProps.color = props.bgColor;
    rectProps.colorTopRight = props.bgColorTopRight;
    rectProps.colorBottomLeft = props.bgColorBottomLeft;
  }
  rect->setProps(rectProps);

  addChild(bmin::UniquePtr<ui::UiElement>(rect));

  auto textLine = new TextLine(window, this);
  int textX = style.x + style.width * style.scale / 2;
  int textY = style.y + style.height * style.scale / 2;
  if (isInActiveMode && !props.isSelected && !props.isDisabled) {
    textX -= 1;
  }
  textLine->setPos(textX, textY);
  textLine->setScale(1.f);
  TextLineProps textLineProps;
  textLineProps.fontFamily = props.fontFamily;
  textLineProps.fontSize = props.fontSize;
  textLineProps.fontColor = props.isDisabled ? Colors::Grey : props.fontColor;
  textLineProps.textAlign = TextAlign::CENTER;
  textLineProps.textBlocks.pushBack(TextBlock{props.text});
  textLine->setProps(textLineProps);
  addChild(bmin::UniquePtr<ui::UiElement>(textLine));
}

void ButtonModal::render(int dt) {
  if (props.isDisabled) {
    isActive = false;
    if (isInActiveMode) {
      isInActiveMode = false;
      build();
    }
  } else if (isActive) {
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
                  props.bgColor);
  }
  UiElement::render(dt);

  if (isActive) {
    auto& draw = window->getDraw();
    auto scaledWidth = static_cast<int>(style.width * style.scale);
    auto scaledHeight = static_cast<int>(style.height * style.scale);
    draw.drawRect(style.x, style.y, scaledWidth, scaledHeight, SDL_Color{0, 0, 0, 25});
  }
}

} // namespace ui
