#pragma once

#include "layers/LayerManager.h"
#include "layers/ui/LayerPickUp.h"
#include "state/AbstractAction.h"
#include <optional>

namespace state {

namespace actions {

class UiShowLayerPickUp : public AbstractAction {
  sdl2w::Window* window;
  std::optional<std::pair<int, int>> containerTile;

  void act() override {
    auto layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }
    auto existing = layerManager->getLayerById(layers::LayerPickUp::LAYER_ID);
    if (existing != nullptr) {
      existing->remove();
    }
    auto layer = containerTile
                     ? new layers::LayerPickUp(window, containerTile->first,
                                               containerTile->second)
                     : new layers::LayerPickUp(window);
    layerManager->addLayer(layer);
    layerManager->moveToFront(layer);
  }

public:
  explicit UiShowLayerPickUp(sdl2w::Window* _window) : window(_window) {}

  UiShowLayerPickUp(sdl2w::Window* _window, int containerX, int containerY)
      : window(_window), containerTile(std::make_pair(containerX, containerY)) {}
};

} // namespace actions

} // namespace state
