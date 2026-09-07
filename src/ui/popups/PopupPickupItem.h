#pragma once

#include "state/LayerRequest.h"
#include "ui/popups/PopupInventoryItem.h"
#include "ui/UiElement.h"
#include "bmin/String.h"

namespace ui {

struct PopupPickupItemProps {
  bmin::String spriteName;
  bmin::String label;
  bmin::String description;
  int weight = 0;
  int value = 0;
  PopupOrientation orientation = WIDE;
};

// PopupPickupItem - shows info about an item that can be picked up
class PopupPickupItem : public UiElement {
  PopupPickupItemProps props;
  state::LayerId ownerLayerId;

public:
  PopupPickupItem(sdl2w::Window* _window,
                  state::LayerId ownerLayerId,
                  PopupOrientation _orientation = WIDE);

  void setProps(const PopupPickupItemProps& _props);
  PopupPickupItemProps& getProps();
  const PopupPickupItemProps& getProps() const;

  void build() override;
  void render(int dt) override;
};

} // namespace ui
