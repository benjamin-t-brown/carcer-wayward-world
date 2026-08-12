#pragma once

#include "layers/LayerManager.h"
#include "layers/ui/LayerEquipRunes.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

/** Keep live equipped-rune edits and close the Equip Runes layer. */
class UiCommitEquipRunes : public AbstractAction {
  void act() override {
    auto* layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }
    auto* layer = layerManager->getLayerById(layers::LayerEquipRunes::LAYER_ID);
    if (layer == nullptr) {
      return;
    }
    layerManager->closeLayer(layer);
  }
};

} // namespace actions

} // namespace state
