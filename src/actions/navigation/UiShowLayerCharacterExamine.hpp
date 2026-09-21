#pragma once

namespace sdl2w { class Window; }

#include "bmin/String.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class UiShowLayerCharacterExamine : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::UiShowLayerCharacterExamine;
  }
  bmin::String characterId;

  void act() override {
    if (!state || characterId.empty()) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::CharacterExamine, .a = characterId});
  }

public:
  UiShowLayerCharacterExamine(sdl2w::Window* /*window*/,
                              const bmin::String& _characterId)
      : characterId(_characterId) {}
};

} // namespace actions

} // namespace state
