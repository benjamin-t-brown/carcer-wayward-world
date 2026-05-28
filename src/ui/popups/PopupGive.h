#pragma once

#include "ui/UiElement.h"
#include <string>
#include <vector>

namespace ui {

class HorizontalSlider;

struct PopupGivePartyMember {
  std::string characterPlayerId;
  std::string label;
  std::string spriteName;
};

struct PopupGiveProps {
  std::string fromCharacterPlayerId;
  std::string itemId;
  std::string itemLabel;
  int maxQuantity = 1;
  int selectedQuantity = 1;
  bool showQuantitySlider = true;
  std::vector<PopupGivePartyMember> partyMembers;
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
