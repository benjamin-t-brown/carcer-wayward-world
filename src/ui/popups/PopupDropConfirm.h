#pragma once

#include "ui/UiElement.h"
#include <string>

namespace ui {

struct PopupDropConfirmProps {
  std::string characterPlayerId;
  std::string itemId;
  std::string itemLabel;
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
