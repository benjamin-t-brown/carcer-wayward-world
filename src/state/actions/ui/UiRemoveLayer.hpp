#pragma once

#include "layers/LayerManager.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UiRemoveLayer : public AbstractAction {
  String layerId;
  void act() override {
    auto layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }
    auto layer = layerManager->getLayerById(bmin::toStringView(layerId));
    if (layer == nullptr) {
      return;
    }
    layer->remove();
    layerManager->moveToFront(layerManager->getLastActiveLayer());
  }

public:
  UiRemoveLayer(const String& _layerId) : layerId(_layerId) {}
};

} // namespace actions

} // namespace state