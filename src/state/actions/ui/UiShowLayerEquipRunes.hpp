#pragma once

#include "bmin/String.h"
#include "layers/LayerManager.h"
#include "layers/ui/LayerEquipRunes.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UiShowLayerEquipRunes : public AbstractAction {
  sdl2w::Window* window;
  bmin::String characterPlayerId;

  void act() override {
    auto layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }
    auto existing = layerManager->getLayerById(layers::LayerEquipRunes::LAYER_ID);
    if (existing != nullptr) {
      layerManager->moveToFront(existing);
      return;
    }

    auto layer = new layers::LayerEquipRunes(window, characterPlayerId);
    layerManager->addLayer(layer);
    layerManager->moveToFront(layer);
  }

public:
  UiShowLayerEquipRunes(sdl2w::Window* _window, const bmin::String& _characterPlayerId)
      : window(_window), characterPlayerId(_characterPlayerId) {}
};

} // namespace actions

} // namespace state
