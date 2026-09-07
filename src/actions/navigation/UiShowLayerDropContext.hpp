#pragma once

namespace sdl2w { class Window; }

#include "bmin/String.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class UiShowLayerDropContext : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerDropContext; }
  bmin::String characterPlayerId;
  bmin::String itemId;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(
        *state,
        LayerRequest{.id = LayerId::DropConfirm, .a = characterPlayerId, .b = itemId});
  }

public:
  UiShowLayerDropContext(sdl2w::Window* /*window*/,
                         bmin::String _characterPlayerId,
                         bmin::String _itemId)
      : characterPlayerId(std::move(_characterPlayerId)),
        itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state
