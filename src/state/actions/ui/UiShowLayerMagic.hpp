#pragma once

#include "layers/LayerManager.h"
#include "layers/ui/LayerMagic.h"
#include "model/instances/Player.h"
#include "state/AbstractAction.h"
#include "state/State.h"

namespace state {

namespace actions {

class UiShowLayerMagic : public AbstractAction {
  sdl2w::Window* window;

  void act() override {
    auto layerManager = getLayerManager();
    if (!layerManager || !state) {
      return;
    }
    auto existing = layerManager->getLayerById(layers::LayerMagic::LAYER_ID);
    if (existing != nullptr) {
      layerManager->moveToFront(existing);
      return;
    }

    // Open magic on the HUD-selected party member.
    auto& player = state->player;
    const int selectedIndex = model::playerFindPartyMemberIndexById(
        player, state->uiState.selectedPartyMemberId);
    player.currentPartyMemberMagicIndex = selectedIndex >= 0 ? selectedIndex : 0;

    auto layer = new layers::LayerMagic(window);
    layerManager->addLayer(layer);
    layerManager->moveToFront(layer);
  }

public:
  explicit UiShowLayerMagic(sdl2w::Window* _window) : window(_window) {}
};

} // namespace actions

} // namespace state
