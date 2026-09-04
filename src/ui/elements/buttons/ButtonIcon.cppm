module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.ui.elements:ButtonIcon;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.UiElement;
import sdl2w;
import :SpriteElement;
import carcer.ui.uiUtils;
#include "macros.h"

export {

// --- from ui/elements/buttons/ButtonIcon.h ---
namespace ui {

struct ButtonIconProps {
  bmin::String regularSprite;
  bmin::String activeSprite;
  int iconSize = 32;
  bool isDisabled = false;
};

// ButtonIcon - clickable button that displays a regular or active sprite.
class ButtonIcon : public UiElement {
private:
  ButtonIconProps props;
  bool isInActiveMode = false;

public:
  static inline constexpr const char* MINUS_ICON1 = "ui_icon_buttons_0";
  static inline constexpr const char* MINUS_ICON2 = "ui_icon_buttons_8";
  static inline constexpr const char* PLUS_ICON1 = "ui_icon_buttons_1";
  static inline constexpr const char* PLUS_ICON2 = "ui_icon_buttons_9";
  static inline constexpr const char* QUESTION_ICON1 = "ui_icon_buttons_2";
  static inline constexpr const char* QUESTION_ICON2 = "ui_icon_buttons_10";
  static inline constexpr const char* HAMBURGER_ICON1 = "ui_icon_buttons_3";
  static inline constexpr const char* HAMBURGER_ICON2 = "ui_icon_buttons_11";

  bool isActive = false;
  ButtonIcon(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonIcon() override = default;

  void setProps(const ButtonIconProps& _props);
  ButtonIconProps& getProps();
  const ButtonIconProps& getProps() const;

  bool checkMouseDownEvent(int mouseX,
                           int mouseY,
                           int button,
                           bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseUpEvent(int mouseX,
                         int mouseY,
                         int button,
                         bmin::DynArray<UiElement*> additionalElements = {}) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

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
  spriteElement->setPos(style.x, style.y);
  spriteElement->setScale(style.scale);
  spriteElement->setProps(SpriteElementProps{
      .width = props.iconSize,
      .height = props.iconSize,
      .spriteName = spriteName,
  });
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
