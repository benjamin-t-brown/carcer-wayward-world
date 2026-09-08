#pragma once

namespace sdl2w { class Window; }

#include "model/instances/Player.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

namespace state {

namespace actions {

class UiShowLayerMagic : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerMagic; }
  void act() override {
    if (!state) {
      return;
    }

    // Open magic on the HUD-selected party member.
    auto& player = state->player;
    const int selectedIndex = model::playerFindPartyMemberIndexById(
        player, state->uiState.selectedPartyMemberId);
    player.currentPartyMemberMagicIndex = selectedIndex >= 0 ? selectedIndex : 0;

    pushLayerRequest(*state, LayerRequest{.id = LayerId::Magic});
  }

public:
  explicit UiShowLayerMagic(sdl2w::Window* /*window*/) {}
};

} // namespace actions

} // namespace state
