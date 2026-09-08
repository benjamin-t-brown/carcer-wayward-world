#pragma once

namespace sdl2w { class Window; }

#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class UiShowLayerSpellCast : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerSpellCast; }
  bmin::String chId;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::SpellCast, .a = chId});
  }

public:
  explicit UiShowLayerSpellCast(sdl2w::Window* /*window*/,
                                const bmin::String& chId)
      : chId(chId) {}
};

} // namespace actions

} // namespace state
