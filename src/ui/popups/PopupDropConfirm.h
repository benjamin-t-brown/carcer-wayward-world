#pragma once

#include "ui/UiElement.h"
#include "bmin/String.h"

namespace ui {

struct PopupDropConfirmProps {
  bmin::String characterPlayerId;
  bmin::String itemId;
  bmin::String itemLabel;
};

class PopupDropConfirm : public UiElement {
  PopupDropConfirmProps props;

public:
  PopupDropConfirm(sdl2w::Window* _window, UiElement* _parent = nullptr);
  ~PopupDropConfirm() override;

  void setProps(const PopupDropConfirmProps& _props);
  PopupDropConfirmProps& getProps();
  const PopupDropConfirmProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
