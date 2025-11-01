#pragma once

#include "layers/LayerManager.h"

// namespace layers {

// class LayerManager;

// } // namespace layers

namespace state {

class LayerManagerInterface {
private:
  static layers::LayerManager* layerManager;

public:
  virtual ~LayerManagerInterface() = default;

  static void setLayerManager(layers::LayerManager* _layerManager);
  static layers::LayerManager* getLayerManager();
};

} // namespace state