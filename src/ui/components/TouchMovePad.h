#pragma once

#include "../UiElement.h"
#include "ui/elements/buttons/ButtonMove.h"

namespace ui {

struct TouchMovePadProps {
  int buttonGapH = 10;
  int buttonGapV = 12;
  int padding = 4;
  int borderSize = 4;
  int dragBarHeight = 18;
};

// TouchMovePad - floating directional pad for mouse/touch movement input.
class TouchMovePad : public UiElement {
  struct ButtonPlacement {
    MoveDirection direction;
    const char* id;
  };

  static constexpr int halfButtonW = 22;
  static constexpr int halfButtonH = 22;
  static constexpr int cardButtonW = 44;
  static constexpr int cardButtonH = 22;
  static constexpr int borderButtonW = 18;
  static constexpr int borderButtonH = 8;

  static constexpr ButtonPlacement buttonPlacements[] = {
      {MoveDirection::UpLeft, "move_up_left"},
      {MoveDirection::Up, "move_up"},
      {MoveDirection::UpRight, "move_up_right"},
      {MoveDirection::Left, "move_left"},
      {MoveDirection::Wait, "move_wait"},
      {MoveDirection::Right, "move_right"},
      {MoveDirection::DownLeft, "move_down_left"},
      {MoveDirection::Down, "move_down"},
      {MoveDirection::DownRight, "move_down_right"},
  };

  TouchMovePadProps props;
  bool isDragging = false;
  int dragOffsetX = 0;
  int dragOffsetY = 0;

  int getGridWidth() const;
  int getGridHeight() const;
  int getContentWidth() const;
  int getContentHeight() const;
  int getBodyY() const;
  int getBodyHeight() const;
  int getWideRowWidth() const;
  int getNarrowRowWidth() const;
  std::pair<int, int> getButtonPosition(MoveDirection direction) const;
  bool isInDragBar(int mouseX, int mouseY) const;
  void positionChildren();
  void syncDragHandle();
  void moveTo(int x, int y);

public:
  TouchMovePad(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~TouchMovePad() override = default;

  void setProps(const TouchMovePadProps& _props);
  TouchMovePadProps& getProps();
  const TouchMovePadProps& getProps() const;

  void startDrag(int mouseX, int mouseY);
  void endDrag();

  bool checkMouseDownEvent(int mouseX,
                           int mouseY,
                           int button,
                           bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkMouseUpEvent(int mouseX,
                         int mouseY,
                         int button,
                         bmin::DynArray<UiElement*> additionalElements = {}) override;
  bool checkHoverEvent(int mouseX,
                       int mouseY,
                       bmin::DynArray<UiElement*> additionalElements = {}) override;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
