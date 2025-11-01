#pragma once

#include "layers/ui/LayerInventoryItemContextMenu.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UiShowInventoryItemContextMenu : public AbstractAction {
  sdl2w::Window* window;
  std::string itemName;
  std::string itemId; // id of item in inventory
  void act() override {
    auto layerManager = getLayerManager();
    auto layer = new layers::LayerInventoryItemContextMenu(window, itemId, itemName);
    layerManager->addLayer(layer);
    layerManager->moveToFront(layer);
  }

public:
  UiShowInventoryItemContextMenu(sdl2w::Window* _window,
                                 std::string itemName,
                                 std::string itemId)
      : window(_window), itemName(itemName), itemId(itemId) {}
};

} // namespace actions

} // namespace state