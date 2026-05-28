#pragma once

#include "layers/LayerManager.h"
#include "layers/ui/LayerGiveContext.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UiShowLayerGiveContext : public AbstractAction {
  sdl2w::Window* window;
  std::string fromCharacterPlayerId;
  std::string itemId;

  void act() override {
    auto layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }
    auto layer =
        new layers::LayerGiveContext(window, fromCharacterPlayerId, itemId);
    layerManager->addLayer(layer);
    layerManager->moveToFront(layer);
  }

public:
  UiShowLayerGiveContext(sdl2w::Window* _window,
                         std::string _fromCharacterPlayerId,
                         std::string _itemId)
      : window(_window),
        fromCharacterPlayerId(std::move(_fromCharacterPlayerId)),
        itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state
