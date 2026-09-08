#pragma once

namespace sdl2w { class Window; }

#include "bmin/String.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class UiShowLayerEquipRunes : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerEquipRunes; }
  bmin::String characterPlayerId;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::EquipRunes, .a = characterPlayerId});
  }

public:
  UiShowLayerEquipRunes(sdl2w::Window* /*window*/,
                        const bmin::String& _characterPlayerId)
      : characterPlayerId(_characterPlayerId) {}
};

} // namespace actions

} // namespace state
