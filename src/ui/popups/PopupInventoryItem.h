#pragma once

#include "ui/UiElement.h"
#include "bmin/String.h"

namespace ui {

enum PopupOrientation { NARROW, WIDE };

struct PopupInventoryItemProps {
  bmin::String characterPlayerId;
  model::ItemInstance item;
  bmin::String label;
  bmin::String description;
  bmin::String spriteName;
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
