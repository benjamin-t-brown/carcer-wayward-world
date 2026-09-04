module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.ui.elements:ButtonMove;
export import bmin.containers;
import bmin.string_interop;
export import carcer.ui.UiElement;
import sdl2w;
import :SpriteElement;
#include "macros.h"

export {

// --- from ui/elements/buttons/ButtonMove.h ---
namespace ui {

enum class MoveDirection {
  UpLeft,
  Up,
  UpRight,
  Left,
  Wait,
  Right,
  DownLeft,
  Down,
  DownRight,
};

struct ButtonMoveProps {
  MoveDirection direction = MoveDirection::Up;
};

// ButtonMove - clickable sprite button for touch movement directions.
class ButtonMove : public UiElement {
  // move_buttons.png layout (256x88):
  // rows 0-1: 22x22 half/diagonal buttons (5 per row, 11 columns in the sprite grid)
  // rows 2-3: 44x22 cardinal buttons (4 per row, 5 columns in the sprite grid)
  static constexpr int halfSpriteWidth = 22;
  static constexpr int halfSpriteHeight = 22;
  static constexpr int cardinalSpriteWidth = 44;
  static constexpr int cardinalSpriteHeight = 22;
  static constexpr int halfUnpressedBase = 0;
  static constexpr int halfPressedRowOffset = 11;
  static constexpr int cardinalUnpressedBase = 10;
  static constexpr int cardinalPressedRowOffset = 5;

  ButtonMoveProps props;
  bool isInActiveMode = false;

  static bool isHalfDirection(MoveDirection direction);
  static int getSpriteIndex(MoveDirection direction);
  bmin::String getSpriteName(bool pressed) const;

public:
  bool isActive = false;
  ButtonMove(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~ButtonMove() override = default;

  void setProps(const ButtonMoveProps& _props);
  ButtonMoveProps& getProps();
  const ButtonMoveProps& getProps() const;

  void setPos(int x, int y) override;
  void build() override;
  void render(int dt) override;
};

} // namespace ui

} // export

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
