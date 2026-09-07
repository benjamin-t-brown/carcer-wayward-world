#pragma once

namespace sdl2w { class Window; }

#include "bmin/String.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class UiShowLayerInventoryContext : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerInventoryContext; }
  bmin::String itemName;
  bmin::String itemId; // id of item in inventory
  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(
        *state,
        LayerRequest{.id = LayerId::InventoryContext, .a = itemId, .b = itemName});
  }

public:
  UiShowLayerInventoryContext(sdl2w::Window* /*window*/,
                              bmin::String itemName,
                              bmin::String itemId)
      : itemName(itemName), itemId(itemId) {}
};

} // namespace actions

} // namespace state
