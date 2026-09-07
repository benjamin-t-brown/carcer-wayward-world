#pragma once

namespace sdl2w { class Window; }

#include "model/instances/Player.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class UiShowLayerInventory : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerInventory; }
  void act() override {
    if (!state) {
      return;
    }

    // Open inventory on the HUD-selected party member.
    auto& player = state->player;
    const int selectedIndex = model::playerFindPartyMemberIndexById(
        player, state->uiState.selectedPartyMemberId);
    player.currentPartyMemberInventoryIndex = selectedIndex >= 0 ? selectedIndex : 0;

    pushLayerRequest(*state, LayerRequest{.id = LayerId::Inventory});
  }

public:
  explicit UiShowLayerInventory(sdl2w::Window* /*window*/) {}
};

} // namespace actions

} // namespace state
