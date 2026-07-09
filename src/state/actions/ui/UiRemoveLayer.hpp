#pragma once

#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "layers/LayerManager.h"
#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UiRemoveLayer : public AbstractAction {
  bmin::String layerId;
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
  UiRemoveLayer(const bmin::String& _layerId) : layerId(_layerId) {}
};

} // namespace actions

} // namespace state