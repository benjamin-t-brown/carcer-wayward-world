#include "LayerManagerInterface.h"
#include "layers/LayerManager.h"

namespace state {

layers::LayerManager* LayerManagerInterface::layerManager = nullptr;

void LayerManagerInterface::setLayerManager(layers::LayerManager* _layerManager) {
  layerManager = _layerManager;
}

layers::LayerManager* LayerManagerInterface::getLayerManager() { return layerManager; }

} // namespace state