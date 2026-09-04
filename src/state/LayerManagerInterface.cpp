module;
#include <cstddef>

module carcer.state;

namespace state {

void* LayerManagerInterface::layerManager = nullptr;

void LayerManagerInterface::setLayerManager(void* _layerManager) {
  layerManager = _layerManager;
}

void* LayerManagerInterface::getLayerManager() { return layerManager; }

} // namespace state
