#pragma once

#include "bmin/String.h"
#include "layers/LayerManager.h"
#include "layers/ui/LayerGiveContext.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UiShowLayerGiveContext : public AbstractAction {
  sdl2w::Window* window;
  bmin::String fromCharacterPlayerId;
  bmin::String itemId;

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
                         bmin::String _fromCharacterPlayerId,
                         bmin::String _itemId)
      : window(_window),
        fromCharacterPlayerId(std::move(_fromCharacterPlayerId)),
        itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state
