#pragma once

#include "ui/UiElement.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace ui {

class HorizontalSlider;

struct PopupGivePartyMember {
  bmin::String characterPlayerId;
  bmin::String label;
  bmin::String spriteName;
};

struct PopupGiveProps {
  bmin::String fromCharacterPlayerId;
  bmin::String itemId;
  bmin::String itemLabel;
  int maxQuantity = 1;
  int selectedQuantity = 1;
  bool showQuantitySlider = true;
  bmin::DynArray<PopupGivePartyMember> partyMembers;
};

class PopupGive : public UiElement {
  PopupGiveProps props;
  HorizontalSlider* quantitySlider = nullptr;

public:
  PopupGive(sdl2w::Window* _window, UiElement* _parent = nullptr);

  void setProps(const PopupGiveProps& _props);
  PopupGiveProps& getProps();
  const PopupGiveProps& getProps() const;

  int getSelectedQuantity() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
