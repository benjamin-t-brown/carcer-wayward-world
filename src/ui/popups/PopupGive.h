#pragma once

#include "ui/UiElement.h"
#include "lib/Types.h"

namespace ui {

class HorizontalSlider;

struct PopupGivePartyMember {
  String characterPlayerId;
  String label;
  String spriteName;
};

struct PopupGiveProps {
  String fromCharacterPlayerId;
  String itemId;
  String itemLabel;
  int maxQuantity = 1;
  int selectedQuantity = 1;
  bool showQuantitySlider = true;
  DynArray<PopupGivePartyMember> partyMembers;
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
