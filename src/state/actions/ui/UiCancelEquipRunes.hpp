#pragma once

#include "layers/LayerManager.h"
#include "layers/ui/LayerEquipRunes.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

/** Restore equipped runes from the open-editor snapshot and close the layer. */
class UiCancelEquipRunes : public AbstractAction {
  void act() override {
    auto* layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }
    auto* layer = layerManager->getLayerById(layers::LayerEquipRunes::LAYER_ID);
    if (layer == nullptr) {
      return;
    }
    if (auto* equipLayer = dynamic_cast<layers::LayerEquipRunes*>(layer)) {
      equipLayer->restoreSnapshot();
    }
    layerManager->closeLayer(layer);
  }
};

} // namespace actions

} // namespace state
