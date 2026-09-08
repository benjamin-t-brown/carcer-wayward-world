#pragma once

namespace sdl2w { class Window; }

#include "model/instances/ItemInstance.hpp"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class UiShowLayerPickupContext : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerPickupContext; }
  model::ItemInstance item;
  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::PickUpContext, .a = item.id});
  }

public:
  UiShowLayerPickupContext(sdl2w::Window* /*window*/,
                           const model::ItemInstance& item)
      : item(item) {}
};

} // namespace actions

} // namespace state
