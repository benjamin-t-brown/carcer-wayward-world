#pragma once

namespace sdl2w { class Window; }

#include "bmin/String.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class UiShowLayerSpellInfo : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerSpellInfo; }
  bmin::String spellName;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::SpellInfo, .a = spellName});
  }

public:
  UiShowLayerSpellInfo(sdl2w::Window* /*window*/,
                       const bmin::String& _spellName)
      : spellName(_spellName) {}
};

} // namespace actions

} // namespace state
