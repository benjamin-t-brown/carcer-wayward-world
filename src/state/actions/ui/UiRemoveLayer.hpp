#pragma once

#include "layers/LayerManager.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UiRemoveLayer : public AbstractAction {
  std::string layerId;
  void act() override {
    auto layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }
    auto layer = layerManager->getLayerById(layerId);
    if (layer == nullptr) {
      return;
    }
    layer->remove();
    layerManager->moveToFront(layerManager->getLastActiveLayer());
  }

public:
  UiRemoveLayer(std::string_view layerId) : layerId(layerId) {}
};

} // namespace actions

} // namespace state