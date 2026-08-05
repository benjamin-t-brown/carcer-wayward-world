#pragma once

#include "layers/LayerManager.h"
#include "layers/ui/LayerInventory.h"
#include "model/instances/Player.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class UiShowLayerInventory : public AbstractAction {
  sdl2w::Window* window;

  void act() override {
    auto layerManager = getLayerManager();
    if (!layerManager || !state) {
      return;
    }
    auto existing = layerManager->getLayerById(layers::LayerInventory::LAYER_ID);
    if (existing != nullptr) {
      layerManager->moveToFront(existing);
      return;
    }

    // Open inventory on the HUD-selected party member.
    auto& player = state->player;
    const int selectedIndex = model::playerFindPartyMemberIndexById(
        player, state->uiState.selectedPartyMemberId);
    player.currentPartyMemberInventoryIndex = selectedIndex >= 0 ? selectedIndex : 0;

    auto layer = new layers::LayerInventory(window);
    layerManager->addLayer(layer);
    layerManager->moveToFront(layer);
  }

public:
  explicit UiShowLayerInventory(sdl2w::Window* _window) : window(_window) {}
};

} // namespace actions

} // namespace state
