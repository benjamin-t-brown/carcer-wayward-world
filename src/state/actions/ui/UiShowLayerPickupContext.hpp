#pragma once

#include "layers/LayerManager.h"
#include "layers/ui/LayerPickupContext.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UiShowLayerPickupContext : public AbstractAction {
  sdl2w::Window* window;
  model::ItemInstance item;
  void act() override {
    auto layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }
    auto layer = new layers::LayerPickUpContext(window, item);
    layerManager->addLayer(layer);
    layerManager->moveToFront(layer);
  }

public:
  UiShowLayerPickupContext(sdl2w::Window* _window, const model::ItemInstance& item)
      : window(_window), item(item) {}
};

} // namespace actions

} // namespace state