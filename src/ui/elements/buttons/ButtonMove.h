#pragma once

#include "../../UiElement.h"

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
