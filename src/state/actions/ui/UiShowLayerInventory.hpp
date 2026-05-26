#pragma once

#include "layers/LayerManager.h"
#include "layers/ui/LayerInventory.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UiShowLayerInventory : public AbstractAction {
  sdl2w::Window* window;

  void act() override {
    auto layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }
    auto existing = layerManager->getLayerById(layers::LayerInventory::LAYER_ID);
    if (existing != nullptr) {
      layerManager->moveToFront(existing);
      return;
    }
    auto layer = new layers::LayerInventory(window);
    layerManager->addLayer(layer);
    layerManager->moveToFront(layer);
  }

public:
  explicit UiShowLayerInventory(sdl2w::Window* _window) : window(_window) {}
};

} // namespace actions

} // namespace state
