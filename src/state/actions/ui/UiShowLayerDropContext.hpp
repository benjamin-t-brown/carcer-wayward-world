#pragma once

#include "layers/LayerManager.h"
#include "layers/ui/LayerDropConfirm.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UiShowLayerDropContext : public AbstractAction {
  sdl2w::Window* window;
  std::string characterPlayerId;
  std::string itemId;

  void act() override {
    auto layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }
    auto layer = new layers::LayerDropConfirm(window, characterPlayerId, itemId);
    layerManager->addLayer(layer);
    layerManager->moveToFront(layer);
  }

public:
  UiShowLayerDropContext(sdl2w::Window* _window,
                         std::string _characterPlayerId,
                         std::string _itemId)
      : window(_window),
        characterPlayerId(std::move(_characterPlayerId)),
        itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state
