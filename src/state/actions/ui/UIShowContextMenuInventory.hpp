#pragma once

#include "layers/ui/LayerContextMenuInventoryItem.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UIShowContextMenuInventory : public AbstractAction {
  sdl2w::Window* window;
  std::string itemName;
  std::string itemId; // id of item in inventory
  void act() override {
    auto layerManager = getLayerManager();
    auto layer = new layers::LayerContextMenuInventoryItem(window, itemId, itemName);
    layerManager->addLayer(layer);
    layerManager->moveToFront(layer);
  }

public:
  UIShowContextMenuInventory(sdl2w::Window* _window,
                             std::string itemName,
                             std::string itemId)
      : window(_window), itemName(itemName), itemId(itemId) {}
};

} // namespace actions

} // namespace state