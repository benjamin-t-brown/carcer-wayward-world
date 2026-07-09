#include "LayerManager.h"
#include "lib/sdl2w/Logger.h"

namespace layers {

LayerManager::LayerManager(sdl2w::Window* _window) : window(_window) {}

LayerManager::~LayerManager() { clearLayers(); }

void LayerManager::removeLayer(const Layer* layer) {
  if (layer == nullptr) {
    return;
  }
  layers.eraseIf([layer](Layer* entry) { return entry == layer; });
  delete layer;
}

void LayerManager::removeLayerAt(size_t index) {
  if (index < layers.size()) {
    delete layers[index];
    layers.erase(layers.begin() + index);
  }
}

void LayerManager::clearLayers() {
  for (auto layer : layers) {
    delete layer;
  }
  layers.clear();
}

void LayerManager::addLayer(Layer* layer) { layers.pushBack(layer); }

// set a layer to be the "front" layer, and suspend all other layers
void LayerManager::moveToFront(Layer* layer) {
  if (layer == nullptr) {
    return;
  }

  LOG(DEBUG) << "LayerManager::moveToFront: moving layer to front: " << layer->getId()
             << LOG_ENDL;

  layerEventsStack.pushBack(layer);

  bool found = false;
  for (auto l : layers) {
    if (l != layer) {
      l->suspend();
    } else if (l == layer) {
      found = true;
      l->turnOn();
    }
  }
  if (!found) {
    LOG(ERROR)
        << "LayerManager::moveToFront: provided layer pointer not found in list of layers"
        << LOG_ENDL;
  }
}

// // remove a layer from the "front" layer, and turn on the previously suspended layer
// void LayerManager::popFront() {
//   LOG(DEBUG) << "LayerManager::popFront: popping front layer" << LOG_ENDL;
//   if (layerEventsStack.empty()) {
//     // if there's no event stack, just turn all suspended layers back on
//     for (auto l : layers) {
//       if (l->getState() == LayerState::SUSPENDED) {
//         l->turnOn();
//       }
//     }
//   } else {
//     layerEventsStack.pop_back();
//     if (layerEventsStack.empty()) {
//       for (auto l : layers) {
//         if (l->getState() == LayerState::SUSPENDED) {
//           l->turnOn();
//         }
//       }
//       return;
//     }
//     auto layer = layerEventsStack.back();
//     bool found = false;
//     for (auto l : layers) {
//       if (l != layer && l->getState() == LayerState::ON) {
//         l->suspend();
//       } else if (l == layer) {
//         found = true;
//         l->turnOn();
//       }
//     }

//     // if the layer pointer was not valid, then pop again
//     if (!found) {
//       popFront();
//     }
//   }
// }

void LayerManager::handleMouseDown(int x, int y, int button) {
  // Process layers from top to bottom (reverse iteration)
  for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
    auto& layer = *it;
    if (layer->getState() == LayerState::ON) {
      layer->onMouseDown(x, y, button);
      // Note: If you want to stop propagation after a layer handles the event,
      // you'll need to modify Layer::onMouseDown to return a bool
    }
  }
}

void LayerManager::handleMouseUp(int x, int y, int button) {
  // Process layers from top to bottom (reverse iteration)
  for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
    auto& layer = *it;
    if (layer->getState() == LayerState::ON) {
      layer->onMouseUp(x, y, button);
    }
  }
}

void LayerManager::handleMouseWheel(int x, int y, int dir) {
  // Process layers from top to bottom (reverse iteration)
  for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
    auto& layer = *it;
    if (layer->getState() == LayerState::ON) {
      layer->onMouseWheel(x, y, dir);
    }
  }
}
void LayerManager::handleKeyDown(std::string_view key, int keyCode) {
  // Process layers from top to bottom (reverse iteration)
  for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
    auto& layer = *it;
    if (layer->getState() == LayerState::ON) {
      layer->onKeyDown(key, keyCode);
    }
  }
}

void LayerManager::handleKeyUp(std::string_view key, int keyCode) {
  // Process layers from top to bottom (reverse iteration)
  for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
    auto& layer = *it;
    if (layer->getState() == LayerState::ON) {
      layer->onKeyUp(key, keyCode);
    }
  }
}

DynArray<Layer*>& LayerManager::getLayers() { return layers; }

const DynArray<Layer*>& LayerManager::getLayers() const { return layers; }

size_t LayerManager::getLayerCount() const { return layers.size(); }

Layer* LayerManager::getLayerAt(size_t index) {
  if (index < layers.size()) {
    return layers[index];
  }
  return nullptr;
}

Layer* LayerManager::getLayerById(std::string_view id) {
  for (auto& layer : layers) {
    if (layer->getId() == id) {
      return layer;
    }
  }
  return nullptr;
}

Layer* LayerManager::getLastActiveLayer() {
  if (layers.empty()) {
    return nullptr;
  }
  for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
    auto* layer = *it;
    if (layer && !layer->shouldRemove()) {
      return layer;
    }
  }
  return nullptr;
}

void LayerManager::update(int deltaTime) {
  DynArray<Layer*> layersToBeRemoved;
  for (unsigned int i = 0; i < layers.size(); i++) {
    auto& layer = layers[i];
    if (layer->getState() == LayerState::ON) {
      layer->update(deltaTime);
    }
    if (layer->shouldRemove()) {
      layersToBeRemoved.pushBack(layer);
    }
  }
  for (auto& layer : layersToBeRemoved) {
    removeLayer(layer);
  }
  layersToBeRemoved.clear();
}

void LayerManager::render(int deltaTime) {
  for (auto& layer : layers) {
    auto state = layer->getState();
    if (state == LayerState::ON || state == LayerState::SUSPENDED) {
      layer->render(deltaTime);
    }
  }
}

} // namespace layers
