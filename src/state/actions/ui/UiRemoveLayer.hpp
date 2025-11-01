#pragma once

#include "state/AbstractAction.h"

namespace state {

namespace actions {

class UiRemoveLayer : public AbstractAction {
  layers::Layer* layer;
  bool shouldPopFront = false;
  void act() override {
    if (layer == nullptr) {
      return;
    }
    layer->remove();
    if (shouldPopFront) {
      getLayerManager()->popFront();
    }
  }

public:
  UiRemoveLayer(layers::Layer* layer, bool shouldPopFront = false)
      : layer(layer), shouldPopFront(shouldPopFront) {}
};

} // namespace actions

} // namespace state