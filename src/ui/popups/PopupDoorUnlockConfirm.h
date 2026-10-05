#pragma once

#include "ui/UiElement.h"
#include "bmin/String.h"

namespace ui {

struct PopupDoorUnlockConfirmProps {
  int worldX = 0;
  int worldY = 0;
  bmin::String message;
  bool dismissOnly = false;
};

class PopupDoorUnlockConfirm : public UiElement {
  PopupDoorUnlockConfirmProps props;

public:
  PopupDoorUnlockConfirm(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PopupDoorUnlockConfirm() override;

  void setProps(const PopupDoorUnlockConfirmProps& _props);
  PopupDoorUnlockConfirmProps& getProps();
  const PopupDoorUnlockConfirmProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
