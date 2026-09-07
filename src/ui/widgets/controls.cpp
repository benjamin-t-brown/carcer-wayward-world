module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <stdexcept>
#include <optional>
#include <string_view>
#include <string>
#include <cmath>

module carcer.ui.widgets.foundation;
#include "macros.h"

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


namespace ui {

class ButtonSpriteDefaultObserver : public UiEventObserver {
  ButtonSprite* buttonSprite;

public:
  ButtonSpriteDefaultObserver(ButtonSprite* _buttonSprite) : buttonSprite(_buttonSprite) {}
  ~ButtonSpriteDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override { buttonSprite->isActive = true; }
  void onMouseUp(int x, int y, int button) override { buttonSprite->isActive = false; }
};

int ButtonSprite::getLogicalWidth() const {
  return props.spriteWidth + props.padding * 2;
}

int ButtonSprite::getLogicalHeight() const {
  return props.spriteHeight + props.padding * 2;
}

ButtonSprite::ButtonSprite(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(new ButtonSpriteDefaultObserver(this));
  shouldPropagateEventsToChildren = false;
}

void ButtonSprite::setProps(const ButtonSpriteProps& _props) {
  props = _props;
  build();
}

ButtonSpriteProps& ButtonSprite::getProps() { return props; }

const ButtonSpriteProps& ButtonSprite::getProps() const { return props; }

const std::pair<int, int> ButtonSprite::getDims() const {
  return {static_cast<int>(getLogicalWidth() * style.scale),
          static_cast<int>(getLogicalHeight() * style.scale)};
}

void ButtonSprite::build() {
  children.clear();

  style.width = getLogicalWidth();
  style.height = getLogicalHeight();

  const int pressOffset = (isInActiveMode && !props.isSelected) ? 1 : 0;

  auto rect = new OutsetRectangle(window);
  rect->setPos(style.x, style.y);
  rect->setScale(style.scale);

  OutsetRectangleProps rectProps;
  rectProps.width = style.width;
  rectProps.height = style.height;
  if (props.isSelected) {
    rectProps.borderSize = isInActiveMode ? 0 : props.selectedBorderSize;
    rectProps.color = props.selectedBgColor;
    rectProps.colorTopRight = props.selectedBgColorTopRight;
    rectProps.colorBottomLeft = props.selectedBgColorBottomLeft;
  } else {
    rectProps.borderSize = isInActiveMode ? 0 : props.borderSize;
    rectProps.color = props.bgColor;
    rectProps.colorTopRight = props.bgColorTopRight;
    rectProps.colorBottomLeft = props.bgColorBottomLeft;
  }
  rect->setProps(rectProps);
  addChild(rect);

  if (!props.spriteName.empty()) {
    auto sprite = new Quad(window, this);
    sprite->setPos(style.x + static_cast<int>((props.padding + pressOffset) * style.scale),
                   style.y + static_cast<int>((props.padding + pressOffset) * style.scale));
    sprite->setScale(style.scale);
    sprite->setProps(QuadProps{
        .width = props.spriteWidth,
        .height = props.spriteHeight,
        .bgSprite = props.spriteName,
    });
    addChild(sprite);
  }
}

void ButtonSprite::render(int dt) {
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


namespace ui {

class ButtonMoveDefaultObserver : public UiEventObserver {
  ButtonMove* buttonMove;

public:
  explicit ButtonMoveDefaultObserver(ButtonMove* _buttonMove) : buttonMove(_buttonMove) {}
  ~ButtonMoveDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override { buttonMove->isActive = true; }
  void onMouseUp(int x, int y, int button) override { buttonMove->isActive = false; }
};

bool ButtonMove::isHalfDirection(MoveDirection direction) {
  switch (direction) {
  case MoveDirection::Up:
  case MoveDirection::Down:
  case MoveDirection::Left:
  case MoveDirection::Right:
    return false;
  default:
    return true;
  }
}

int ButtonMove::getSpriteIndex(MoveDirection direction) {
  switch (direction) {
  case MoveDirection::UpLeft:
    return 0;
  case MoveDirection::UpRight:
    return 1;
  case MoveDirection::DownLeft:
    return 2;
  case MoveDirection::DownRight:
    return 3;
  case MoveDirection::Wait:
    return 4;
  case MoveDirection::Up:
    return 0;
  case MoveDirection::Down:
    return 1;
  case MoveDirection::Right:
    return 2;
  case MoveDirection::Left:
    return 3;
  }
  return 0;
}

bmin::String ButtonMove::getSpriteName(bool pressed) const {
  const bool isHalf = isHalfDirection(props.direction);
  int spriteIndex = getSpriteIndex(props.direction);
  if (isHalf) {
    spriteIndex += halfUnpressedBase;
    if (pressed) {
      spriteIndex += halfPressedRowOffset;
    }
  } else {
    spriteIndex += cardinalUnpressedBase;
    if (pressed) {
      spriteIndex += cardinalPressedRowOffset;
    }
  }

  bmin::StringStream ss;
  ss << (isHalf ? "ui_move_buttons_half" : "ui_move_buttons") << "_" << spriteIndex;
  return bmin::String(ss.str().cStr());
}

ButtonMove::ButtonMove(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(new ButtonMoveDefaultObserver(this));
  shouldPropagateEventsToChildren = false;
}

void ButtonMove::setProps(const ButtonMoveProps& _props) {
  props = _props;
  build();
}

void ButtonMove::setPos(int x, int y) {
  UiElement::setPos(x, y);
  if (!children.empty()) {
    children[0]->setPos(x, y);
  }
}

ButtonMoveProps& ButtonMove::getProps() { return props; }

const ButtonMoveProps& ButtonMove::getProps() const { return props; }

void ButtonMove::build() {
  children.clear();

  const bool isHalf = isHalfDirection(props.direction);
  style.width = isHalf ? halfSpriteWidth : cardinalSpriteWidth;
  style.height = isHalf ? halfSpriteHeight : cardinalSpriteHeight;

  auto spriteElement = bmin::makeUnique<SpriteElement>(window);
  spriteElement->setPos(style.x, style.y);
  spriteElement->setScale(style.scale);
  spriteElement->setProps(SpriteElementProps{
      .width = style.width,
      .height = style.height,
      .spriteName = getSpriteName(isActive),
  });
  children.pushBack(bmin::UniquePtr<UiElement>(spriteElement.release()));
}

void ButtonMove::render(int dt) {
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


namespace ui {

using state::WorldActionType;

class ButtonWorldActionDefaultObserver : public UiEventObserver {
  ButtonWorldAction* buttonWorldAction;

public:
  ButtonWorldActionDefaultObserver(ButtonWorldAction* _buttonWorldAction)
      : buttonWorldAction(_buttonWorldAction) {}
  ~ButtonWorldActionDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override {
    buttonWorldAction->isActive = true;
  }
  void onMouseUp(int x, int y, int button) override {
    buttonWorldAction->isActive = false;
  }
};

ButtonWorldAction::ButtonWorldAction(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(new ButtonWorldActionDefaultObserver(this));
  shouldPropagateEventsToChildren = false;

  style.width = 32;
  style.height = 32;
}

ButtonWorldActionMapping
ButtonWorldAction::getButtonWorldActionMapping(state::WorldActionType worldActionType) {
  switch (worldActionType) {
  case WorldActionType::JUMP:
    return ButtonWorldActionMapping{TRANSLATE("Jump"), 17, true};
    break;
  case WorldActionType::ABILITY:
    return ButtonWorldActionMapping{TRANSLATE("Ability"), 1};
    break;
  case WorldActionType::TALK:
    return ButtonWorldActionMapping{TRANSLATE("Talk"), 3};
    break;
  case WorldActionType::END_FIGHT:
    return ButtonWorldActionMapping{TRANSLATE("End Fight"), 4};
    break;
  case WorldActionType::GET:
    return ButtonWorldActionMapping{TRANSLATE("Get"), 16, true};
    break;
  case WorldActionType::SNEAK:
    return ButtonWorldActionMapping{TRANSLATE("Sneak"), 7};
    break;
  case WorldActionType::START_FIGHT:
    return ButtonWorldActionMapping{TRANSLATE("Start Fight"), 8};
    break;
  case WorldActionType::UNLOCK:
    return ButtonWorldActionMapping{TRANSLATE("Unlock"), 9};
    break;
  case WorldActionType::EXAMINE:
    return ButtonWorldActionMapping{TRANSLATE("Examine"), 10};
    break;
  case WorldActionType::SHOOT:
    return ButtonWorldActionMapping{TRANSLATE("Shoot"), 0, true};
    break;
  case WorldActionType::DEFEND:
    return ButtonWorldActionMapping{TRANSLATE("Defend"), 1, true};
    break;
  case WorldActionType::INTERACT:
    return ButtonWorldActionMapping{TRANSLATE("Interact"), 2, true};
    break;
  case WorldActionType::REST:
    return ButtonWorldActionMapping{TRANSLATE("Rest"), 3, true};
    break;
  case WorldActionType::JOURNAL:
    return ButtonWorldActionMapping{TRANSLATE("Journal"), 4, true};
    break;
  case WorldActionType::STATUS:
    return ButtonWorldActionMapping{TRANSLATE("Status"), 5, true};
    break;
  case WorldActionType::INVENTORY:
    return ButtonWorldActionMapping{TRANSLATE("Inventory"), 6, true};
    break;
  case WorldActionType::MAP:
    return ButtonWorldActionMapping{TRANSLATE("Map"), 7, true};
    break;
  case WorldActionType::MAP_OUTDOOR:
    return ButtonWorldActionMapping{TRANSLATE("Map"), 6};
    break;
  }
  return ButtonWorldActionMapping{TRANSLATE("Examine"), 0};
}

bool ButtonWorldAction::checkIfWorldActionButtonIsSmall(
    state::WorldActionType worldActionType) {
  auto mapping = getButtonWorldActionMapping(worldActionType);
  return mapping.isSmall;
}

void ButtonWorldAction::setProps(const ButtonWorldActionProps& _props) {
  props = _props;
  build();
}

ButtonWorldActionProps& ButtonWorldAction::getProps() { return props; }

const ButtonWorldActionProps& ButtonWorldAction::getProps() const { return props; }

void ButtonWorldAction::build() {
  children.clear();

  auto mapping = getButtonWorldActionMapping(props.worldActionType);

  int spriteW = mapping.isSmall ? 32 : 32;
  int spriteH = mapping.isSmall ? 16 : 32;
  style.width = spriteW;
  style.height = spriteH;

  int startingSpriteIndex =
      mapping.isSmall ? smallStartingSpriteIndex : normalStartingSpriteIndex;
  bmin::StringStream ss;
  ss << spriteSheetName << "_";
  if (mapping.isSmall) {
    ss << "half_";
  }

  if (isActive || isModeSelected) {
    if (mapping.isSmall) {
      startingSpriteIndex += smallSpriteOffsetToActive;
    } else {
      startingSpriteIndex += normalSpriteOffsetToActive;
    }
  }
  ss << (startingSpriteIndex + mapping.spriteIndex);

  auto spriteElement = bmin::makeUnique<SpriteElement>(window);
  spriteElement->setPos(style.x, style.y);
  spriteElement->setScale(style.scale);
  spriteElement->setProps(SpriteElementProps{
      .width = spriteW,
      .height = spriteH,
      .spriteName = bmin::String(ss.str().cStr()),
  });
  children.pushBack(bmin::UniquePtr<UiElement>(spriteElement.release()));
}

void ButtonWorldAction::render(int dt) {
  const bool showActiveSprite = isActive || isModeSelected;
  if (showActiveSprite) {
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
  addEventObserver(new ButtonModalDefaultObserver(this));
  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_BUTTON);
  props.fontFamily = font.fontFamily;
  props.fontSize = font.fontSize;
  props.fontColor = font.fontColor;
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

  style.width = props.width;
  style.height = props.height;

  auto rect = new OutsetRectangle(window);
  rect->setPos(style.x, style.y);
  rect->setScale(style.scale);

  OutsetRectangleProps rectProps;
  if (isInActiveMode) {
    rectProps.borderSize = 0;
  } else {
    rectProps.borderSize = 2;
  }
  rectProps.width = props.width;
  rectProps.height = props.height;
  rectProps.color = props.bgColor;
  rectProps.colorTopRight = props.bgColorTopRight;
  rectProps.colorBottomLeft = props.bgColorBottomLeft;
  rect->setProps(rectProps);

  addChild(rect);

  auto textLine = new TextLine(window, this);
  int textX = style.x + style.width * style.scale / 2;
  int textY = style.y + style.height * style.scale / 2;
  if (isInActiveMode && !props.isSelected) {
    textX -= 1;
  }
  textLine->setPos(textX, textY);
  textLine->setScale(1.f);
  TextLineProps textLineProps;
  textLineProps.fontFamily = props.fontFamily;
  textLineProps.fontSize = props.fontSize;
  textLineProps.fontColor = props.fontColor;
  textLineProps.textAlign = TextAlign::CENTER;
  textLineProps.textBlocks.pushBack(TextBlock{props.text});
  textLine->setProps(textLineProps);
  addChild(textLine);
}

void ButtonModal::render(int dt) {
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


namespace ui {

TextBanner::TextBanner(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT_BOLD);
  props.fontFamily = font.fontFamily;
  props.fontSize = font.fontSize;
  props.fontColor = font.fontColor;
}

void TextBanner::setProps(const TextBannerProps& _props) {
  props = _props;
  build();
}

TextBannerProps& TextBanner::getProps() { return props; }

const TextBannerProps& TextBanner::getProps() const { return props; }

void TextBanner::setPos(int x, int y) {
  UiElement::setPos(x, y);
  build();
}

void TextBanner::setScale(float scale) {
  UiElement::setScale(scale);
  build();
}

std::pair<int, int> TextBanner::measureTextScaled() const {
  if (props.text.empty()) {
    return {0, 0};
  }

  TextLine measureLine(window);
  measureLine.setScale(1.f);
  TextLineProps measureProps;
  measureProps.fontFamily = props.fontFamily;
  measureProps.fontSize = props.fontSize;
  measureProps.fontColor = props.fontColor;
  measureProps.textBlocks.pushBack(TextBlock{.text = props.text});
  measureLine.setProps(measureProps);
  return measureLine.getDims();
}

std::pair<int, int>
TextBanner::calculateBannerScreenPosition(int bannerScaledWidth,
                                          int bannerScaledHeight) const {
  const int containerX = style.x + static_cast<int>(props.location.first * style.scale);
  const int containerY = style.y + static_cast<int>(props.location.second * style.scale);
  const int containerWidth = static_cast<int>(props.dims.first * style.scale);
  const int containerHeight = static_cast<int>(props.dims.second * style.scale);

  int bannerX = containerX;
  int bannerY = containerY;

  switch (props.corner) {
  case TextBannerCorner::LEFT_TOP:
    break;
  case TextBannerCorner::RIGHT_TOP:
    bannerX = containerX + containerWidth - bannerScaledWidth;
    break;
  case TextBannerCorner::LEFT_BOTTOM:
    bannerY = containerY + containerHeight - bannerScaledHeight;
    break;
  case TextBannerCorner::RIGHT_BOTTOM:
    bannerX = containerX + containerWidth - bannerScaledWidth;
    bannerY = containerY + containerHeight - bannerScaledHeight;
    break;
  }

  return {bannerX, bannerY};
}

const std::pair<int, int> TextBanner::getDims() const {
  const auto [textWidth, textHeight] = measureTextScaled();
  const int borderScaled = static_cast<int>(props.outsetBorderSize * style.scale);
  const int paddingScaled = static_cast<int>(props.padding * style.scale);
  const int bannerWidth = textWidth + paddingScaled * 2 + borderScaled * 2;
  const int bannerHeight = textHeight + paddingScaled * 2 + borderScaled * 2;
  return {bannerWidth, bannerHeight};
}

void TextBanner::build() {
  children.clear();

  auto textLine = new TextLine(window, this);
  textLine->setScale(1.f);
  TextLineProps lineProps;
  lineProps.fontFamily = props.fontFamily;
  lineProps.fontSize = props.fontSize;
  lineProps.fontColor = props.fontColor;
  lineProps.textAlign = TextAlign::LEFT_CENTER;
  lineProps.textBlocks.pushBack(TextBlock{.text = props.text});
  textLine->setProps(lineProps);

  auto [textWidth, textHeight] = textLine->getDims();
  auto paddingScaled = static_cast<int>(props.padding * style.scale);
  int textX = 0;
  int textY = 0;
  switch (props.corner) {
  case TextBannerCorner::LEFT_TOP:
    textX = props.location.first + paddingScaled;
    textY = props.location.second + (paddingScaled * 2 + textHeight) / 2;
    break;
  case TextBannerCorner::RIGHT_TOP:
    textX = props.location.first + props.dims.first - textWidth - paddingScaled;
    textY = props.location.second + (paddingScaled * 2 + textHeight) / 2;
    break;
  case TextBannerCorner::LEFT_BOTTOM:
    textX = props.location.first + paddingScaled;
    textY =
        props.location.second + props.dims.second - (paddingScaled * 2 + textHeight) / 2;
    break;
  case TextBannerCorner::RIGHT_BOTTOM:
    textX = props.location.first + props.dims.first - textWidth - paddingScaled;
    textY =
        props.location.second + props.dims.second - (paddingScaled * 2 + textHeight) / 2;
    break;
  }
  textLine->setPos(textX, textY);

  auto background = new OutsetRectangle(window, this);
  background->setPos(textX - paddingScaled,
                     textY - (paddingScaled * 2 + textHeight) / 2);
  background->setScale(1.f);
  background->setProps(OutsetRectangleProps{
      .width = textWidth + paddingScaled * 2,
      .height = textHeight + paddingScaled * 2,
      .color = props.backgroundColor,
      .borderSize = props.outsetBorderSize,
  });

  addChild(background);
  addChild(textLine);

  auto [bw, bh] = getDims();
  style.width = style.scale > 0.f ? static_cast<int>(bw / style.scale) : bw;
  style.height = style.scale > 0.f ? static_cast<int>(bh / style.scale) : bh;
}

void TextBanner::render(int dt) { UiElement::render(dt); }

} // namespace ui


namespace ui {

namespace {

std::pair<int, int> measureLine(sdl2w::Draw& draw,
                                const bmin::String& lineText,
                                const sdl2w::RenderTextParams& params) {
  const bmin::String& sample = lineText.empty() ? bmin::String(" ") : lineText;
  return draw.measureText(bmin::toStringView(sample), params);
}

// Blank lines already store their gap height; content lines scale only the advance
// between lines so glyphs can still paint at full measured height.
int lineBoxAdvance(int glyphOrBlankHeight, bool isBlank, float lineHeightScale) {
  if (isBlank) {
    return glyphOrBlankHeight;
  }
  return std::max(1, static_cast<int>(glyphOrBlankHeight * lineHeightScale));
}

} // namespace

TextParagraph::TextParagraph(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  quad = bmin::makeUnique<Quad>(window, this);
  quad->setId("textParagraphQuad");
}

void TextParagraph::setProps(const TextParagraphProps& _props) {
  props = _props;
  build();
}

TextParagraphProps& TextParagraph::getProps() { return props; }

const TextParagraphProps& TextParagraph::getProps() const { return props; }

void TextParagraph::setPos(int x, int y) {
  UiElement::setPos(x, y);
  build();
}

void TextParagraph::setScale(float scale) {
  UiElement::setScale(scale);
  build();
}

size_t TextParagraph::getNumLines() const {
  if (generatedBlocks.empty()) {
    return 0;
  }

  int maxLineNumber = 0;
  for (const auto& block : generatedBlocks) {
    if (block.lineNumber > maxLineNumber) {
      maxLineNumber = block.lineNumber;
    }
  }

  return static_cast<size_t>(maxLineNumber + 1);
}

int TextParagraph::getContentHeight() const {
  if (generatedBlocks.empty()) {
    return 0;
  }

  // Line boxes may be shorter than glyphs (lineHeightScale < 1), so height is the
  // max bottom edge of full glyph bounds, not the sum of scaled line boxes alone.
  int y = 0;
  int maxBottom = 0;
  int currentLine = -1;
  int lineMaxGlyphH = 0;
  bool lineIsBlank = true;
  int linesCounted = 0;

  auto finishLine = [&]() {
    if (currentLine < 0) {
      return;
    }
    if (linesCounted > 0) {
      y += props.lineSpacing;
    }
    maxBottom = std::max(maxBottom, y + lineMaxGlyphH);
    y += lineBoxAdvance(lineMaxGlyphH, lineIsBlank, props.lineHeightScale);
    linesCounted++;
  };

  for (const auto& block : generatedBlocks) {
    if (block.lineNumber != currentLine) {
      finishLine();
      currentLine = block.lineNumber;
      lineMaxGlyphH = block.textHeight;
      lineIsBlank = block.text.empty();
    } else {
      lineMaxGlyphH = std::max(lineMaxGlyphH, block.textHeight);
      if (!block.text.empty()) {
        lineIsBlank = false;
      }
    }
  }
  finishLine();
  return maxBottom;
}

const std::pair<int, int> TextParagraph::getDims() const {
  const int contentHeight = getContentHeight();
  const int logicalW = style.width + 2 * props.padding;
  const int logicalH = contentHeight + 2 * props.padding;
  return {static_cast<int>(logicalW * style.scale),
          static_cast<int>(logicalH * style.scale)};
}

void TextParagraph::build() {
  generatedBlocks.clear();
  style.width = props.width;
  auto& draw = window->getDraw();
  int fontScale = 0;
  try {
    auto stateManager = getStateManager();
    if (!stateManager) {
      throw std::runtime_error("StateManager not set");
    }
    fontScale = stateManager->getState().settings.fontScale;
  } catch (...) {
    // Some isolated UI tests do not initialize a StateManager.
    fontScale = 0;
  }

  int lineNumber = 0;
  int currentLineWidth = 0;
  bmin::String segmentText;
  bmin::String nextWord;

  // Emit the current TextBlock's open segment. endLine advances to the next
  // visual line; emitEmptyIfNoSegment keeps blank-line (`\n`) behavior.
  auto emitSegment = [&](const TextBlock& block,
                         const sdl2w::RenderTextParams& params,
                         bool endLine,
                         bool emitEmptyIfNoSegment) {
    if (!segmentText.empty()) {
      auto [textWidth, textHeight] = measureLine(draw, segmentText, params);
      // Store full glyph height; lineHeightScale is applied when advancing lines.
      generatedBlocks.pushBack(TextParagraphGeneratedBlock{
          lineNumber,
          block,
          segmentText,
          textWidth,
          textHeight});
      segmentText.clear();
    } else if (emitEmptyIfNoSegment && currentLineWidth == 0) {
      // Blank line from a double line-break (`\n\n`): scaled paragraph gap.
      auto [textWidth, textHeight] = measureLine(draw, segmentText, params);
      (void)textWidth;
      const int blankLineHeight = std::max(
          0, static_cast<int>(textHeight * props.blankLineHeightScale));
      generatedBlocks.pushBack(TextParagraphGeneratedBlock{
          lineNumber, block, segmentText, 0, blankLineHeight});
    }

    if (endLine) {
      lineNumber++;
      currentLineWidth = 0;
    }
  };

  auto appendToCurrentLine = [&](const bmin::String& text,
                                 const TextBlock& block,
                                 const sdl2w::RenderTextParams& params) {
    if (text.empty()) {
      return;
    }
    auto [pieceWidth, pieceHeight] = measureLine(draw, text, params);
    (void)pieceHeight;

    if (currentLineWidth > 0 && currentLineWidth + pieceWidth >= style.width) {
      // Wrap: close this visual line (keep any prior same-line segments).
      emitSegment(block, params, true, false);
      segmentText = text;
      currentLineWidth = pieceWidth;
      if (pieceWidth >= style.width) {
        emitSegment(block, params, true, false);
      }
    } else {
      const bool startedFreshLine = segmentText.empty() && currentLineWidth == 0;
      segmentText += text;
      currentLineWidth += pieceWidth;
      if (startedFreshLine && pieceWidth >= style.width) {
        emitSegment(block, params, true, false);
      }
    }
  };

  for (const auto& block : props.textBlocks) {
    if (block.text.empty()) {
      continue;
    }

    auto fontFamily = block.fontFamily.value_or(props.fontFamily);
    auto fontSize = block.fontSize.value_or(props.fontSize);
    auto fontColor = block.fontColor.value_or(props.fontColor);
    auto fontName = TextLine::getFontNameFromFamily(fontFamily);

    sdl2w::RenderTextParams params;
    params.fontName = fontName.cStr();
    params.fontSize = ui::applyFontScale(fontSize, fontScale);
    params.color = fontColor;
    params.centered = false;

    for (size_t i = 0; i < block.text.size(); i++) {
      auto c = block.text[i];
      if (c == '\r') {
        continue;
      }
      if (i > 0 && block.text[i - 1] == ' ' && c == ' ') {
        // skip multiple spaces in a row
        continue;
      }
      auto isLastLetter = i + 1 == block.text.size();
      if (c == '\n') {
        if (!nextWord.empty()) {
          appendToCurrentLine(nextWord, block, params);
          nextWord.clear();
        }
        emitSegment(block, params, true, true);
      } else if (c == ' ') {
        appendToCurrentLine(nextWord + ' ', block, params);
        nextWord.clear();
      } else if (isLastLetter) {
        appendToCurrentLine(nextWord + c, block, params);
        nextWord.clear();
      } else {
        nextWord += c;
      }
    }

    if (!nextWord.empty()) {
      appendToCurrentLine(nextWord, block, params);
      nextWord.clear();
    }

    // Mid-line TextBlock switch: flush this block's segment without ending the
    // visual line so later genBlocks can share the same lineNumber.
    if (!segmentText.empty()) {
      emitSegment(block, params, false, false);
    }
  }

  while (!quad->getChildren().empty()) {
    quad->removeChildAtIndex(0);
  }

  // Create TextLine children inside the quad (texture-local coordinates)
  if (!generatedBlocks.empty()) {
    auto currentLineNumber = -1;
    bmin::DynArray<TextBlock> currentLineBlocks;
    auto currentY = props.padding;
    int currentLineMaxHeight = 0;
    bool currentLineIsBlank = true;

    auto flushLine = [&]() {
      if (currentLineBlocks.empty()) {
        return;
      }

      auto textLine = new TextLine(window, quad.get());
      textLine->setPos(props.padding, currentY);
      textLine->setScale(1.f);

      TextLineProps lineProps;
      lineProps.textBlocks = currentLineBlocks;
      lineProps.fontFamily =
          currentLineBlocks[0].fontFamily.value_or(props.fontFamily);
      lineProps.fontSize = currentLineBlocks[0].fontSize.value_or(props.fontSize);
      lineProps.fontColor = currentLineBlocks[0].fontColor.value_or(props.fontColor);
      lineProps.textAlign = props.textAlign;
      textLine->setProps(lineProps);

      quad->addChild(textLine);
      currentLineBlocks.clear();

      currentY += lineBoxAdvance(currentLineMaxHeight,
                                 currentLineIsBlank,
                                 props.lineHeightScale) +
                  props.lineSpacing;
      currentLineMaxHeight = 0;
      currentLineIsBlank = true;
    };

    for (const auto& genBlock : generatedBlocks) {
      if (genBlock.lineNumber != currentLineNumber) {
        if (currentLineNumber >= 0) {
          flushLine();
        }
        currentLineNumber = genBlock.lineNumber;
      }

      currentLineMaxHeight = std::max(currentLineMaxHeight, genBlock.textHeight);
      if (!genBlock.text.empty()) {
        currentLineIsBlank = false;
      }

      TextBlock textBlock;
      textBlock.text = genBlock.text;
      textBlock.fontFamily = genBlock.textBlock.fontFamily.value_or(props.fontFamily);
      textBlock.fontSize = genBlock.textBlock.fontSize.value_or(props.fontSize);
      textBlock.fontColor = genBlock.textBlock.fontColor.value_or(props.fontColor);
      currentLineBlocks.pushBack(textBlock);
    }

    if (!currentLineBlocks.empty()) {
      const auto& lastGenBlock = generatedBlocks.back();
      // Last line: place glyphs but do not advance — container height comes from
      // getContentHeight(), which reserves full glyph bounds for descenders.
      auto textLine = new TextLine(window, quad.get());
      textLine->setPos(props.padding, currentY);
      textLine->setScale(1.f);

      TextLineProps lineProps;
      lineProps.textBlocks = currentLineBlocks;
      lineProps.fontFamily =
          lastGenBlock.textBlock.fontFamily.value_or(props.fontFamily);
      lineProps.fontSize = lastGenBlock.textBlock.fontSize.value_or(props.fontSize);
      lineProps.fontColor = lastGenBlock.textBlock.fontColor.value_or(props.fontColor);
      lineProps.textAlign = props.textAlign;
      textLine->setProps(lineProps);

      quad->addChild(textLine);
    }
  }

  const int contentHeight = getContentHeight();
  style.height = contentHeight + 2 * props.padding;

  quad->setPos(style.x, style.y);
  quad->setScale(style.scale);
  QuadProps quadProps;
  quadProps.width = style.width + 2 * props.padding;
  quadProps.height = contentHeight + 2 * props.padding;
  quadProps.bgColor = props.bgColor;
  quad->setProps(quadProps);
}

void TextParagraph::render(int dt) {
  if (quad) {
    quad->render(dt);
  }
}

} // namespace ui


namespace ui {

class ButtonListDefaultObserver : public UiEventObserver {
  ButtonList* buttonList;

public:
  ButtonListDefaultObserver(ButtonList* _buttonList) : buttonList(_buttonList) {}
  ~ButtonListDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override { buttonList->isActive = true; }
  void onMouseUp(int x, int y, int button) override { buttonList->isActive = false; }
};

int ButtonList::yForListRow(int rowHeight, int btnLogicalSize, float scale) {
  const int scaledBtnSize = static_cast<int>(btnLogicalSize * scale);
  return (rowHeight - scaledBtnSize) / 2;
}

ButtonList::ButtonList(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(new ButtonListDefaultObserver(this));
  shouldPropagateEventsToChildren = false;
  style.width = defaultLogicalSize;
  style.height = defaultLogicalSize;
}

void ButtonList::setProps(const ButtonListProps& _props) {
  props = _props;
  build();
}

ButtonListProps& ButtonList::getProps() { return props; }

const ButtonListProps& ButtonList::getProps() const { return props; }

void ButtonList::build() {
  children.clear();

  style.width = props.width;
  style.height = props.height;

  auto rect = new OutsetRectangle(window);
  rect->setPos(style.x, style.y);
  rect->setScale(style.scale);

  OutsetRectangleProps rectProps;
  rectProps.width = props.width;
  rectProps.height = props.height;
  if (isInActiveMode) {
    rectProps.borderSize = 0;
  } else {
    rectProps.borderSize = 2;
  }
  rectProps.color = props.bgColor;
  rectProps.colorTopRight = props.bgColorTopRight;
  rectProps.colorBottomLeft = props.bgColorBottomLeft;
  rect->setProps(rectProps);

  addChild(rect);

  if (!props.arrow.has_value() && !props.text.empty()) {
    auto textLine = new TextLine(window, this);
    int textX = style.x + style.width * style.scale / 2;
    int textY = style.y + style.height * style.scale / 2;
    if (isInActiveMode && !props.isSelected) {
      textX -= 1;
    }
    textLine->setPos(textX, textY);
    textLine->setScale(1.f);
    TextLineProps listTextProps;
    TextFontProps font;
    setBaseFontConfig(font, BaseFontConfig::MODAL_BUTTON);
    listTextProps.fontFamily = font.fontFamily;
    listTextProps.fontSize = font.fontSize;
    listTextProps.fontColor = props.fontColor;
    listTextProps.textAlign = TextAlign::CENTER;
    listTextProps.textBlocks.pushBack(TextBlock{props.text});
    textLine->setProps(listTextProps);
    addChild(textLine);
  }
}

void ButtonList::render(int dt) {
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
                  props.bgColor);
  }
  UiElement::render(dt);

  if (!props.arrow.has_value()) {
    return;
  }

  auto scaledX = static_cast<int>(style.x);
  auto scaledY = static_cast<int>(style.y);
  auto scaledWidth = static_cast<int>(style.width * style.scale);
  auto scaledHeight = static_cast<int>(style.height * style.scale);
  auto centerX = scaledX + scaledWidth / 2;
  auto centerY = scaledY + scaledHeight / 2;
  const auto arrowBasis = std::max(scaledWidth, scaledHeight);
  const auto arrowLength = std::max(arrowBasis / 4, 5) - 2;
  if (isInActiveMode) {
    centerX -= static_cast<int>(style.scale);
  }

  auto& draw = window->getDraw();
  if (*props.arrow == ScrollDirection::UP) {
    draw.drawLine({centerX - arrowLength, centerY},
                  {centerX, centerY - arrowLength},
                  style.scale,
                  props.arrowColor);
    draw.drawLine({centerX, centerY + arrowLength},
                  {centerX, centerY - arrowLength},
                  style.scale,
                  props.arrowColor);
    draw.drawLine({centerX + arrowLength, centerY},
                  {centerX, centerY - arrowLength},
                  style.scale,
                  props.arrowColor);
  } else if (*props.arrow == ScrollDirection::DOWN) {
    draw.drawLine({centerX - arrowLength, centerY},
                  {centerX, centerY + arrowLength},
                  style.scale,
                  props.arrowColor);
    draw.drawLine({centerX, centerY - arrowLength},
                  {centerX, centerY + arrowLength},
                  style.scale,
                  props.arrowColor);
    draw.drawLine({centerX + arrowLength, centerY},
                  {centerX, centerY + arrowLength},
                  style.scale,
                  props.arrowColor);
  }
}

} // namespace ui


namespace ui {

class HorizontalSliderButtonObserver : public UiEventObserver {
  HorizontalSlider* slider;
  bool incrementDirection;

public:
  HorizontalSliderButtonObserver(HorizontalSlider* _slider, bool _incrementDirection)
      : slider(_slider), incrementDirection(_incrementDirection) {}

  void onClick(int x, int y, int button) override {
    if (!slider) {
      return;
    }
    if (incrementDirection) {
      slider->increment();
    } else {
      slider->decrement();
    }
  }
};

HorizontalSlider::HorizontalSlider(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
}

void HorizontalSlider::setProps(const HorizontalSliderProps& _props) {
  props = _props;
  props.minValue = std::max(1, props.minValue);
  props.maxValue = std::max(props.minValue, props.maxValue);
  props.value = std::clamp(props.value, props.minValue, props.maxValue);
  build();
}

HorizontalSliderProps& HorizontalSlider::getProps() { return props; }

const HorizontalSliderProps& HorizontalSlider::getProps() const { return props; }

void HorizontalSlider::setPos(int x, int y) {
  UiElement::setPos(x, y);
  build();
}

void HorizontalSlider::setScale(float scale) {
  UiElement::setScale(scale);
  build();
}

bool HorizontalSlider::isInSliderTrack(int mouseX, int mouseY) const {
  const int buttonSize = props.sliderBarHeight * style.scale;
  const int scaledWidth = static_cast<int>(props.width * style.scale);
  const int trackX = style.x + buttonSize;
  const int trackW = std::max<int>(0, scaledWidth - buttonSize * 2);
  const int trackH = props.sliderBarHeight * style.scale;
  return isInBounds(mouseX, mouseY, trackX, style.y, trackW, trackH);
}

bool HorizontalSlider::hitIndicator(int mouseX, int mouseY) {
  auto* indicator = getChildById("thumb");
  return indicator != nullptr && isInBoundsScaled(mouseX, mouseY, indicator);
}

bool HorizontalSlider::hitButton(int mouseX, int mouseY) {
  auto* leftButton = getChildById("leftButton");
  auto* rightButton = getChildById("rightButton");
  return (leftButton != nullptr && isInBoundsScaled(mouseX, mouseY, leftButton)) ||
         (rightButton != nullptr && isInBoundsScaled(mouseX, mouseY, rightButton));
}

void HorizontalSlider::setValueFromIndicatorMouseX(int mouseX) {
  const int range = std::max<int>(0, props.maxValue - props.minValue);
  if (range <= 0) {
    props.value = props.minValue;
    refreshValueUi();
    return;
  }

  const int buttonSize = props.sliderBarHeight * style.scale;
  const int scaledWidth = static_cast<int>(props.width * style.scale);
  const int trackX = style.x + buttonSize;
  const int trackW = std::max<int>(0, scaledWidth - buttonSize * 2);
  const int thumbW = props.indicatorWidth * style.scale;
  const int availableSpace = std::max<int>(0, trackW - thumbW);
  if (availableSpace <= 0) {
    return;
  }

  const int thumbHalf = thumbW / 2;
  const int relativeX = mouseX - trackX - thumbHalf;
  const float ratio =
      std::clamp(static_cast<float>(relativeX) / availableSpace, 0.f, 1.f);
  props.value =
      props.minValue + static_cast<int>(ratio * static_cast<float>(range) + 0.5f);
  refreshValueUi();
}

void HorizontalSlider::refreshValueUi() {
  const int buttonSize = props.sliderBarHeight * style.scale;
  const int scaledWidth = static_cast<int>(style.width * style.scale);
  const int trackX = style.x + buttonSize;
  const int trackWidth = std::max<int>(0, scaledWidth - buttonSize * 2);
  const int range = std::max<int>(1, props.maxValue - props.minValue);
  const float pct = static_cast<float>(props.value - props.minValue) / range;

  if (auto* thumb = dynamic_cast<Quad*>(getChildById("thumb"))) {
    const int maxThumbOffset =
        std::max<int>(0, trackWidth - props.indicatorWidth * style.scale);
    thumb->setPos(trackX + static_cast<int>(pct * maxThumbOffset), style.y);
  }

  if (auto* valueLabel = dynamic_cast<TextLine*>(getChildById("valueLabel"))) {
    TextLineProps valueProps = valueLabel->getProps();
    valueProps.textBlocks.clear();
    valueProps.textBlocks.pushBack({
        .text = bmin::toString(props.value) + " / " + bmin::toString(props.maxValue),
        .fontColor = props.labelColor,
    });
    valueProps.fontColor = props.labelColor;
    valueLabel->setProps(valueProps);
  }
}

void HorizontalSlider::increment() {
  props.value = std::min(props.maxValue, props.value + 1);
  refreshValueUi();
}

void HorizontalSlider::decrement() {
  props.value = std::max(props.minValue, props.value - 1);
  refreshValueUi();
}

bool HorizontalSlider::checkMouseDownEvent(
    int mouseX, int mouseY, int button, bmin::DynArray<UiElement*> additionalElements) {
  const bool handled = UiElement::checkMouseDownEvent(mouseX, mouseY, button);
  if (isInSliderTrack(mouseX, mouseY) && !hitButton(mouseX, mouseY)) {
    setValueFromIndicatorMouseX(mouseX);
    if (hitIndicator(mouseX, mouseY)) {
      isDraggingIndicator = true;
    }
  }
  return handled;
}

bool HorizontalSlider::checkMouseUpEvent(
    int mouseX, int mouseY, int button, bmin::DynArray<UiElement*> additionalElements) {
  isDraggingIndicator = false;
  return UiElement::checkMouseUpEvent(mouseX, mouseY, button);
}

bool HorizontalSlider::checkHoverEvent(int mouseX,
                                       int mouseY,
                                       bmin::DynArray<UiElement*> additionalElements) {
  if (isDraggingIndicator) {
    setValueFromIndicatorMouseX(mouseX);
    return true;
  }
  return UiElement::checkHoverEvent(mouseX, mouseY);
}

const std::pair<int, int> HorizontalSlider::getDims() const {
  return {static_cast<int>(props.width * style.scale),
          static_cast<int>(props.height * style.scale)};
}

void HorizontalSlider::build() {
  children.clear();

  style.width = props.width;
  style.height = props.height;

  const int scaledWidth = static_cast<int>(props.width * style.scale);
  const int scaledBarHeight = props.sliderBarHeight * style.scale;
  const int buttonSize = scaledBarHeight;
  const int trackX = style.x + buttonSize;
  const int trackWidth = std::max<int>(0, scaledWidth - buttonSize * 2);

  auto leftButton = new ButtonScroll(window, this);
  leftButton->setId("leftButton");
  leftButton->setPos(style.x, style.y);
  leftButton->setScale(style.scale);
  leftButton->setProps(ButtonScrollProps{
      .direction = ScrollDirection::LEFT,
      .width = props.sliderBarHeight,
      .height = props.sliderBarHeight,
  });
  leftButton->addEventObserver(new HorizontalSliderButtonObserver(this, false));
  addChild(leftButton);

  auto rightButton = new ButtonScroll(window, this);
  rightButton->setId("rightButton");
  rightButton->setPos(style.x + scaledWidth - buttonSize, style.y);
  rightButton->setScale(style.scale);
  rightButton->setProps(ButtonScrollProps{
      .direction = ScrollDirection::RIGHT,
      .width = props.sliderBarHeight,
      .height = props.sliderBarHeight,
  });
  rightButton->addEventObserver(new HorizontalSliderButtonObserver(this, true));
  addChild(rightButton);

  auto track = new Quad(window, this);
  track->setId("track");
  track->setPos(trackX, style.y);
  track->setScale(1.f);
  track->setProps(QuadProps{
      .width = trackWidth,
      .height = scaledBarHeight,
      .bgColor = Colors::Transparent,
      .borderColor = Colors::Black,
      .borderSize = 0,
  });
  addChild(track);

  auto thumb = new Quad(window, this);
  thumb->setId("thumb");
  thumb->setPos(trackX, style.y);
  thumb->setScale(1.f);
  thumb->setProps(QuadProps{
      .width = props.indicatorWidth * static_cast<int>(style.scale),
      .height = scaledBarHeight,
      .bgColor = Colors::Grey,
      .borderColor = Colors::Black,
      .borderSize = 0,
  });
  addChild(thumb);

  auto valueLabel = new TextLine(window, this);
  valueLabel->setId("valueLabel");
  valueLabel->setPos(style.x + scaledWidth / 2,
                     style.y + scaledBarHeight + static_cast<int>(8 * style.scale));
  valueLabel->setScale(1.f);
  TextLineProps valueLabelProps;
  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT);
  valueLabelProps.fontFamily = font.fontFamily;
  valueLabelProps.fontSize = font.fontSize;
  valueLabelProps.fontColor = props.labelColor;
  valueLabelProps.textAlign = TextAlign::CENTER;
  valueLabelProps.textBlocks.pushBack({.text = ""});
  valueLabel->setProps(valueLabelProps);
  addChild(valueLabel);
  refreshValueUi();
}

void HorizontalSlider::render(int dt) { UiElement::render(dt); }

} // namespace ui


namespace ui {

class SectionScrollableScrollObserver : public UiEventObserver {
  SectionScrollable* sectionScrollable;
  bool isUpButton;

public:
  SectionScrollableScrollObserver(SectionScrollable* _sectionScrollable, bool _isUpButton)
      : sectionScrollable(_sectionScrollable), isUpButton(_isUpButton) {}
  ~SectionScrollableScrollObserver() override = default;

  void onMouseDown(int x, int y, int button) override {
    if (isUpButton) {
      sectionScrollable->scrollUp();
    } else {
      sectionScrollable->scrollDown();
    }
  }
  void onMouseUp(int x, int y, int button) override {}
  void onClick(int x, int y, int button) override {}
};

class SectionScrollableScrollWheelObserver : public UiEventObserver {
  SectionScrollable* sectionScrollable;

public:
  SectionScrollableScrollWheelObserver(SectionScrollable* _sectionScrollable)
      : sectionScrollable(_sectionScrollable) {}
  ~SectionScrollableScrollWheelObserver() override = default;

  void onMouseWheel(int x, int y, int delta) override {
    if (delta > 0) {
      sectionScrollable->scrollUp();
    } else {
      sectionScrollable->scrollDown();
    }
  }
};

SectionScrollable::SectionScrollable(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  shouldPropagateEventsToChildren = true;
  addEventObserver(new SectionScrollableScrollWheelObserver(this));

  outerQuad = bmin::makeUnique<Quad>(window, this);
  outerQuad->setId("outerQuad");

  innerQuad = new Quad(window, outerQuad.get());
  innerQuad->setId("innerQuad");

  outerQuad->addChild(innerQuad);
  outerQuad->build();

  // dont need to add child for outerQuad, not managed by children
}

std::pair<int, int> SectionScrollable::getContentDims() const {
  return {style.width * style.scale - props.scrollBarWidth * style.scale,
          style.height * style.scale};
}

UiElement* SectionScrollable::getChildById(std::string_view searchId) {
  if (outerQuad) {
    if (auto* found = outerQuad->getChildById(searchId)) {
      return found;
    }
  }
  return UiElement::getChildById(searchId);
}

int SectionScrollable::getScrollIndicatorY(int offset) const {
  const int scaledHeight = static_cast<int>(style.height * style.scale);
  const float availableSpace =
      scaledHeight - (2 * props.scrollBarWidth + props.indicatorHeight) * style.scale;
  int indicatorY = style.y + props.scrollBarWidth * style.scale;
  if (maxScrollOffset > 0 && availableSpace > 0) {
    const float scrollRatio =
        static_cast<float>(offset) / static_cast<float>(maxScrollOffset);
    indicatorY += static_cast<int>(scrollRatio * availableSpace);
  }
  return indicatorY;
}

void SectionScrollable::updateScrollIndicatorPosition() {
  auto elem = getChildById("scrollIndicator");
  if (elem) {
    Quad* indicator = dynamic_cast<Quad*>(elem);
    if (indicator) {
      const int scaledContentWidth = (style.width - props.scrollBarWidth) * style.scale;
      indicator->setPos(style.x + scaledContentWidth, getScrollIndicatorY(scrollOffset));
    }
  }
  updateScrollButtonStates();
}

void SectionScrollable::updateScrollButtonStates() {
  const int currentIndicatorY = getScrollIndicatorY(scrollOffset);

  const int upOffset = std::max(0, scrollOffset - props.scrollStep);
  const bool upDisabled = getScrollIndicatorY(upOffset) == currentIndicatorY;

  const int downOffset = std::min(maxScrollOffset, scrollOffset + props.scrollStep);
  const bool downDisabled = getScrollIndicatorY(downOffset) == currentIndicatorY;

  auto* upButton = dynamic_cast<ButtonScroll*>(getChildById("scrollUpButton"));
  if (upButton && upButton->getProps().isDisabled != upDisabled) {
    ButtonScrollProps upProps = upButton->getProps();
    upProps.isDisabled = upDisabled;
    upButton->setProps(upProps);
  }

  auto* downButton = dynamic_cast<ButtonScroll*>(getChildById("scrollDownButton"));
  if (downButton && downButton->getProps().isDisabled != downDisabled) {
    ButtonScrollProps downProps = downButton->getProps();
    downProps.isDisabled = downDisabled;
    downButton->setProps(downProps);
  }
}

void SectionScrollable::setProps(const SectionScrollableProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }
  build();
}

void SectionScrollable::setPos(int x, int y) {
  UiElement::setPos(x, y);
  build();
}

void SectionScrollable::setScale(float scale) {
  UiElement::setScale(scale);
  build();
}

SectionScrollableProps& SectionScrollable::getProps() { return props; }

const SectionScrollableProps& SectionScrollable::getProps() const { return props; }

bool SectionScrollable::isInScrollTrack(int mouseX, int mouseY) const {
  const int scaledContentWidth = (style.width - props.scrollBarWidth) * style.scale;
  const int barX = style.x + scaledContentWidth;
  const int barW = props.scrollBarWidth * style.scale;
  const int trackTop = style.y + props.scrollBarWidth * style.scale;
  const int trackBottom = style.y + static_cast<int>(style.height * style.scale) -
                          props.scrollBarWidth * style.scale;
  return isInBounds(mouseX, mouseY, barX, trackTop, barW, trackBottom - trackTop);
}

bool SectionScrollable::hitScrollIndicator(int mouseX, int mouseY) {
  auto* indicator = getChildById("scrollIndicator");
  return indicator != nullptr && isInBoundsScaled(mouseX, mouseY, indicator);
}

bool SectionScrollable::hitScrollButton(int mouseX, int mouseY) {
  auto* upButton = getChildById("scrollUpButton");
  auto* downButton = getChildById("scrollDownButton");
  return (upButton != nullptr && isInBoundsScaled(mouseX, mouseY, upButton)) ||
         (downButton != nullptr && isInBoundsScaled(mouseX, mouseY, downButton));
}

void SectionScrollable::scrollFromIndicatorMouseY(int mouseY) {
  if (maxScrollOffset <= 0) {
    return;
  }

  const int scaledHeight = static_cast<int>(style.height * style.scale);
  const int trackTop = style.y + props.scrollBarWidth * style.scale;
  const float availableSpace =
      scaledHeight - (2 * props.scrollBarWidth + props.indicatorHeight) * style.scale;
  if (availableSpace <= 0) {
    return;
  }

  const int indicatorHalf = static_cast<int>(props.indicatorHeight * style.scale / 2);
  const int relativeY = mouseY - trackTop - indicatorHalf;
  const float ratio =
      std::clamp(static_cast<float>(relativeY) / availableSpace, 0.f, 1.f);
  scrollTo(static_cast<int>(ratio * maxScrollOffset + 0.5f));
}

namespace {

bmin::DynArray<UiElement*> additionalWithQuad(Quad* quad) {
  bmin::DynArray<UiElement*> elements;
  if (quad != nullptr) {
    elements.pushBack(quad);
  }
  return elements;
}

} // namespace

bool SectionScrollable::checkMouseDownEvent(
    int mouseX, int mouseY, int button, bmin::DynArray<UiElement*> additionalElements) {
  const bool handled = UiElement::checkMouseDownEvent(
      mouseX, mouseY, button, additionalWithQuad(outerQuad.get()));
  if (maxScrollOffset > 0 && isInScrollTrack(mouseX, mouseY) &&
      !hitScrollButton(mouseX, mouseY)) {
    scrollFromIndicatorMouseY(mouseY);
    if (hitScrollIndicator(mouseX, mouseY)) {
      isDraggingIndicator = true;
    }
  }
  return handled;
}

bool SectionScrollable::checkMouseUpEvent(int mouseX,
                                          int mouseY,
                                          int button,
                                          bmin::DynArray<UiElement*> additionalElements) {
  isDraggingIndicator = false;
  return UiElement::checkMouseUpEvent(
      mouseX, mouseY, button, additionalWithQuad(outerQuad.get()));
}

bool SectionScrollable::checkHoverEvent(int mouseX,
                                        int mouseY,
                                        bmin::DynArray<UiElement*> additionalElements) {
  if (isDraggingIndicator) {
    scrollFromIndicatorMouseY(mouseY);
    return true;
  }
  return UiElement::checkHoverEvent(mouseX, mouseY, additionalWithQuad(outerQuad.get()));
}

bool SectionScrollable::checkMouseWheelEvent(
    int mouseX, int mouseY, int delta, bmin::DynArray<UiElement*> additionalElements) {
  return UiElement::checkMouseWheelEvent(
      mouseX, mouseY, delta, additionalWithQuad(outerQuad.get()));
}

void SectionScrollable::scrollUp() {
  if (scrollOffset > 0) {
    scrollOffset -= props.scrollStep; // Scroll step
    if (scrollOffset < 0) {
      scrollOffset = 0;
    }
    if (innerQuad) {
      innerQuad->setPos(0, -scrollOffset);
    }
    // Update indicator position
    updateScrollIndicatorPosition();
  }
}

void SectionScrollable::scrollDown() {
  if (scrollOffset < maxScrollOffset) {
    scrollOffset += props.scrollStep; // Scroll step
    if (scrollOffset > maxScrollOffset) {
      scrollOffset = maxScrollOffset;
    }
  }
  if (innerQuad) {
    innerQuad->setPos(0, -scrollOffset);
  }
  // Update indicator position
  updateScrollIndicatorPosition();
}

void SectionScrollable::scrollTo(int offset) {
  scrollOffset = offset;
  if (scrollOffset < 0) {
    scrollOffset = 0;
  }
  if (scrollOffset > maxScrollOffset) {
    scrollOffset = maxScrollOffset;
  }
  if (innerQuad) {
    innerQuad->setPos(0, -scrollOffset);
  }
  // Update indicator position
  updateScrollIndicatorPosition();
}

void SectionScrollable::addChild(UiElement* child) {
  if (innerQuad) {
    innerQuad->addChild(child);
  }
}

void SectionScrollable::build() {
  if (props.width > 0) {
    style.width = props.width;
  }
  if (props.height > 0) {
    style.height = props.height;
  }

  innerHeightScaled = 0;
  for (auto& child : innerQuad->getChildren()) {
    innerHeightScaled += child->getDims().second;
  }

  int scaledContentWidth = (style.width - props.scrollBarWidth) * style.scale;
  int scaledContentHeight = innerHeightScaled;
  int scaledHeight = static_cast<int>(style.height * style.scale);

  innerQuad->setPos(0, -scrollOffset);
  innerQuad->setScale(1.f);
  innerQuad->setProps(QuadProps{
      .width = scaledContentWidth,
      .height = std::max(scaledContentHeight, scaledHeight),
  });

  outerQuad->setPos(style.x, style.y);
  outerQuad->setScale(1.f);
  outerQuad->setProps(QuadProps{
      .width = scaledContentWidth,
      .height = scaledHeight,
      .bgColor = props.bgColor,
      .borderColor = props.borderColor,
      .borderSize = props.borderSize,
  });

  // Create scroll up button
  auto scrollUpButton = new ButtonScroll(window);
  scrollUpButton->setPos(style.x + scaledContentWidth, style.y);
  scrollUpButton->setScale(style.scale);
  scrollUpButton->setProps(ButtonScrollProps{
      .direction = ScrollDirection::UP,
      .isSelected = false,
      .width = props.scrollBarWidth,
      .height = props.scrollBarWidth,
  });
  scrollUpButton->setId("scrollUpButton");
  scrollUpButton->addEventObserver(new SectionScrollableScrollObserver(this, true));

  // Create scroll down button
  auto scrollDownButton = new ButtonScroll(window);
  scrollDownButton->setPos(style.x + scaledContentWidth,
                           style.y + scaledHeight - props.scrollBarWidth * style.scale);
  scrollDownButton->setScale(style.scale);
  scrollDownButton->setProps(ButtonScrollProps{
      .direction = ScrollDirection::DOWN,
      .isSelected = false,
      .width = props.scrollBarWidth,
      .height = props.scrollBarWidth,
  });
  scrollDownButton->setId("scrollDownButton");
  scrollDownButton->addEventObserver(new SectionScrollableScrollObserver(this, false));

  // Calculate maxScrollOffset before creating indicator
  maxScrollOffset = std::max(0, scaledContentHeight - scaledHeight);

  // Create scroll indicator (rectangle showing scroll position)
  auto scrollIndicator = new Quad(window);
  scrollIndicator->setPos(style.x + scaledContentWidth,
                          getScrollIndicatorY(scrollOffset));
  scrollIndicator->setScale(1.f);
  scrollIndicator->setProps(QuadProps{
      .width = props.scrollBarWidth * static_cast<int>(style.scale),
      .height = props.indicatorHeight * static_cast<int>(style.scale),
      .bgColor = Colors::LightGrey,
      .borderColor = Colors::Transparent,
      .borderSize = 0,
  });
  scrollIndicator->setId("scrollIndicator");

  children.clear();

  UiElement::addChild(scrollUpButton);
  UiElement::addChild(scrollDownButton);
  UiElement::addChild(scrollIndicator);

  updateScrollButtonStates();
}

void SectionScrollable::render(int dt) {
  // auto& draw = window->getDraw();
  // auto [scaledWidth, scaledHeight] = getDims();
  // draw.drawRect(style.x, style.y, scaledWidth, scaledHeight, Colors::LightBlue);
  outerQuad->render(dt);
  UiElement::render(dt);
}

} // namespace ui


namespace ui {

ButtonGroup::ButtonGroup(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {}

void ButtonGroup::setProps(const ButtonGroupProps& _props) {
  props = _props;
  if (props.width > 0) {
    style.width = props.width;
  }
  build();
}

ButtonGroupProps& ButtonGroup::getProps() { return props; }

const ButtonGroupProps& ButtonGroup::getProps() const { return props; }

void ButtonGroup::addObserverToButtonAtIndex(int index, UiEventObserver* observer) {
  if (index >= 0 && index < static_cast<int>(children.size())) {
    children[index]->addEventObserver(observer);
  } else {
    LOG(ERROR) << "ButtonGroup: Index out of bounds when adding observer: " << index
               << LOG_ENDL;
  }
}

void ButtonGroup::build() {
  children.clear();

  if (props.width > 0) {
    style.width = props.width;
  }

  if (props.buttons.empty()) {
    return;
  }

  const int count = static_cast<int>(props.buttons.size());
  const int totalButtonWidth =
      count * props.buttonWidth + (count - 1) * props.buttonSpacing;
  const int contentLogicalWidth = props.padding * 2 + totalButtonWidth;
  style.width = std::max(style.width, contentLogicalWidth);
  style.height = props.padding * 2 + props.buttonHeight;

  const int alignmentScaledWidth = static_cast<int>(style.width * style.scale);

  const auto scaledPadding = static_cast<int>(props.padding * style.scale);
  const auto scaledButtonWidth = static_cast<int>(props.buttonWidth * style.scale);
  const auto scaledButtonSpacing = static_cast<int>(props.buttonSpacing * style.scale);
  const int rowWidth =
      count * scaledButtonWidth + (count - 1) * scaledButtonSpacing;

  for (int i = 0; i < count; ++i) {
    const auto& buttonProps = props.buttons[static_cast<size_t>(i)];

    int buttonX = 0;
    switch (props.alignment) {
    case ButtonGroupAlignment::LEFT:
      buttonX = style.x + scaledPadding + i * (scaledButtonWidth + scaledButtonSpacing);
      break;
    case ButtonGroupAlignment::CENTER:
      buttonX = style.x + (alignmentScaledWidth - rowWidth) / 2 +
                i * (scaledButtonWidth + scaledButtonSpacing);
      break;
    case ButtonGroupAlignment::RIGHT:
      buttonX = style.x + alignmentScaledWidth - scaledPadding -
                (i + 1) * scaledButtonWidth - i * scaledButtonSpacing;
      break;
    }

    const int buttonY = style.y + scaledPadding;

    switch (buttonProps.type) {
    case ButtonGroupButtonType::MODAL: {
      auto button = new ButtonModal(window, this);
      button->setId("buttonGroupButton_" + bmin::toString(i));
      button->setPos(buttonX, buttonY);
      button->setScale(style.scale);
      button->setProps(ButtonModalProps{
          .text = buttonProps.label,
          .width = props.buttonWidth,
          .height = props.buttonHeight,
      });
      addChild(button);
      break;
    }
    case ButtonGroupButtonType::SPRITE: {
      auto button = new ButtonSprite(window, this);
      button->setId("buttonGroupButton_" + bmin::toString(i));
      button->setPos(buttonX, buttonY);
      button->setScale(style.scale);
      button->setProps(ButtonSpriteProps{
          .spriteName = buttonProps.spriteName,
          .spriteWidth = buttonProps.spriteWidth,
          .spriteHeight = buttonProps.spriteHeight,
          .padding = buttonProps.spritePadding,
          .isSelected = buttonProps.isSelected,
          .bgColor = props.spriteBgColor,
          .bgColorTopRight = props.spriteBgColorTopRight,
          .bgColorBottomLeft = props.spriteBgColorBottomLeft,
          .borderSize = props.spriteBorderSize,
          .selectedBgColor = props.spriteSelectedBgColor,
          .selectedBgColorTopRight = props.spriteSelectedBgColorTopRight,
          .selectedBgColorBottomLeft = props.spriteSelectedBgColorBottomLeft,
          .selectedBorderSize = props.spriteSelectedBorderSize,
      });
      addChild(button);
      break;
    }
    }
  }
}

void ButtonGroup::render(int dt) { UiElement::render(dt); }

} // namespace ui


namespace ui {

class ButtonTextWrapDefaultObserver : public UiEventObserver {
  ButtonTextWrap* buttonTextWrap;

public:
  ButtonTextWrapDefaultObserver(ButtonTextWrap* _buttonTextWrap)
      : buttonTextWrap(_buttonTextWrap) {}
  ~ButtonTextWrapDefaultObserver() override = default;
  void onMouseDown(int x, int y, int button) override { buttonTextWrap->isActive = true; }
  void onMouseUp(int x, int y, int button) override { buttonTextWrap->isActive = false; }
};

ButtonTextWrap::ButtonTextWrap(sdl2w::Window* _window, UiElement* _parent)
    : UiElement(_window, _parent) {
  addEventObserver(new ButtonTextWrapDefaultObserver(this));
  TextFontProps font;
  setBaseFontConfig(font, BaseFontConfig::MODAL_TEXT);
  props.textParagraph.fontFamily = font.fontFamily;
  props.textParagraph.fontSize = font.fontSize;
  props.textParagraph.fontColor = Colors::Black;
  shouldPropagateEventsToChildren = false;
}

void ButtonTextWrap::setProps(const ButtonTextWrapProps& _props) {
  props = _props;
  build();
}

ButtonTextWrapProps& ButtonTextWrap::getProps() { return props; }

const ButtonTextWrapProps& ButtonTextWrap::getProps() const { return props; }

void ButtonTextWrap::build() {
  children.clear();

  const float scale = style.scale > 0.f ? style.scale : 1.f;
  style.width = props.textParagraph.width + props.horizontalPadding * 2;

  auto textParagraph = new TextParagraph(window, this);
  textParagraph->setPos(style.x + static_cast<int>(props.horizontalPadding * scale),
                        style.y + static_cast<int>(props.verticalPadding * scale));
  textParagraph->setScale(scale);
  textParagraph->setProps(props.textParagraph);

  addChild(textParagraph);

  const int paragraphHeightScaled = textParagraph->getDims().second;
  style.height = static_cast<int>(std::round(paragraphHeightScaled / scale)) +
                 props.verticalPadding * 2;
}

void ButtonTextWrap::render(int dt) {
  SDL_Color bgColor = Colors::Transparent;

  if (isActive) {
    bgColor = SDL_Color{0, 0, 0, 25};
  } else if (isHovered) {
    // bgColor = SDL_Color{0, 0, 0, 50};
  }

  auto& draw = window->getDraw();
  auto dims = getDims();
  int borderSize = 0;
  draw.drawRect(style.x - borderSize,
                style.y - borderSize,
                dims.first + borderSize * 2,
                dims.second + borderSize * 2,
                bgColor);

  UiElement::render(dt);
}

} // namespace ui

