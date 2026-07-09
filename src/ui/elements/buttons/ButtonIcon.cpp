#include "ButtonIcon.h"
#include "bmin/UniquePtr.h"
#include "ui/elements/SpriteElement.h"
#include "ui/uiUtils.h"

namespace ui {

class ButtonIconDefaultObserver : public UiEventObserver {
  ButtonIcon* buttonIcon;

public:
  ButtonIconDefaultObserver(ButtonIcon* _buttonIcon) : buttonIcon(_buttonIcon) {}
  ~ButtonIconDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override {
    if (!buttonIcon->getProps().isDisabled) {
      buttonIcon->isActive = true;
    }
  }
  void onMouseUp(int x, int y, int button) override {
    if (!buttonIcon->getProps().isDisabled) {
      buttonIcon->isActive = false;
    }
  }
};

ButtonIcon::ButtonIcon(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(new ButtonIconDefaultObserver(this));
  shouldPropagateEventsToChildren = false;
}

void ButtonIcon::setProps(const ButtonIconProps& _props) {
  props = _props;
  build();
}

ButtonIconProps& ButtonIcon::getProps() { return props; }

const ButtonIconProps& ButtonIcon::getProps() const { return props; }

bool ButtonIcon::checkMouseDownEvent(int mouseX,
                                     int mouseY,
                                     int button,
                                     bmin::DynArray<UiElement*> additionalElements) {
  if (props.isDisabled) {
    return isInBoundsScaled(mouseX, mouseY, this);
  }
  return UiElement::checkMouseDownEvent(mouseX, mouseY, button, additionalElements);
}

bool ButtonIcon::checkMouseUpEvent(int mouseX,
                                   int mouseY,
                                   int button,
                                   bmin::DynArray<UiElement*> additionalElements) {
  if (props.isDisabled) {
    isClicked = false;
    return isInBoundsScaled(mouseX, mouseY, this);
  }
  return UiElement::checkMouseUpEvent(mouseX, mouseY, button, additionalElements);
}

void ButtonIcon::build() {
  children.clear();

  style.width = props.iconSize;
  style.height = props.iconSize;

  const bool showActive = props.isDisabled || isActive;
  const bmin::String& spriteName = showActive ? props.activeSprite : props.regularSprite;
  if (spriteName.empty()) {
    return;
  }

  auto spriteElement = bmin::makeUnique<SpriteElement>(window);
  BaseStyle spriteStyle;
  spriteStyle.x = style.x;
  spriteStyle.y = style.y;
  spriteStyle.width = props.iconSize;
  spriteStyle.height = props.iconSize;
  spriteStyle.scale = style.scale;
  spriteElement->setStyle(spriteStyle);
  spriteElement->setSprite(spriteName);
  children.pushBack(bmin::UniquePtr<UiElement>(spriteElement.release()));
}

void ButtonIcon::render(int dt) {
  if (props.isDisabled) {
    isActive = false;
    if (!isInActiveMode) {
      isInActiveMode = true;
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

  UiElement::render(dt);
}

} // namespace ui
