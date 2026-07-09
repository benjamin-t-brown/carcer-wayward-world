#pragma once

#include "bmin/String.h"
#include "layers/LayerManager.h"
#include "layers/ui/LayerInventoryContext.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UiShowLayerInventoryContext : public AbstractAction {
  sdl2w::Window* window;
  bmin::String itemName;
  bmin::String itemId; // id of item in inventory
  void act() override {
    auto layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }
    auto layer = new layers::LayerInventoryContext(window, itemId, itemName);
    layerManager->addLayer(layer);
    layerManager->moveToFront(layer);
  }

public:
  UiShowLayerInventoryContext(sdl2w::Window* _window,
                              bmin::String itemName,
                              bmin::String itemId)
      : window(_window), itemName(itemName), itemId(itemId) {}
};

} // namespace actions

} // namespace state