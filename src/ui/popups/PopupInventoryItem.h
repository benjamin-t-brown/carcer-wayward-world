#pragma once

#include "model/templates/Items.h"
#include "ui/UiElement.h"
#include <string>

namespace ui {

enum PopupOrientation { NARROW, WIDE };

struct PopupInventoryItemProps {
  std::string characterPlayerId;
  model::ItemInstance item;
  std::string label;
  std::string description;
  std::string spriteName;
  int weight = 0;
  int value = 0;
  bool usable = false;
  bool equippable = false;
  PopupOrientation orientation = WIDE;
};

class PopupInventoryItem : public UiElement {
  PopupInventoryItemProps props;

public:
  PopupInventoryItem(sdl2w::Window* _window,
                     UiElement* _parent,
                     PopupOrientation _orientation = WIDE);

  void setProps(const PopupInventoryItemProps& _props);
  PopupInventoryItemProps& getProps();
  const PopupInventoryItemProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
