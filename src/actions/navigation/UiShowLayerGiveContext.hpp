#pragma once

namespace sdl2w { class Window; }

#include "bmin/String.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class UiShowLayerGiveContext : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerGiveContext; }
  bmin::String fromCharacterPlayerId;
  bmin::String itemId;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::GiveContext,
                                  .a = fromCharacterPlayerId,
                                  .b = itemId});
  }

public:
  UiShowLayerGiveContext(sdl2w::Window* /*window*/,
                         bmin::String _fromCharacterPlayerId,
                         bmin::String _itemId)
      : fromCharacterPlayerId(std::move(_fromCharacterPlayerId)),
        itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state
