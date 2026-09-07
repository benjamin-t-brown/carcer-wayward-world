module;
#include <cstddef>
#include <functional>
#include <string_view>
#include <typeinfo>

module carcer.ui.layers;
import sdl2w;
import bmin.string_interop;
#include "macros.h"

namespace layers {

LayerManager::LayerManager(sdl2w::Window* _window) : window(_window) {
  state::LayerManagerInterface::setLayerManager(this);
}

LayerManager::~LayerManager() {
  clearLayers();
  if (state::LayerManagerInterface::getLayerManager() == this) {
    state::LayerManagerInterface::setLayerManager(nullptr);
  }
}

void LayerManager::bindEvents() {
  auto& events = window->getEvents();
  events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_DOWN,
                       [this](int x, int y, int button) {
                         handleMouseDown(x, y, button);
                       });
  events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_UP,
                       [this](int x, int y, int button) {
                         handleMouseUp(x, y, button);
                       });
  events.setMouseEvent(sdl2w::MouseEventCb::ON_MOUSE_WHEEL,
                       [this](int x, int y, int delta) {
                         handleMouseWheel(x, y, delta);
                       });
  events.setKeyboardEvent(sdl2w::KeyboardEventCb::ON_KEY_DOWN,
                          [this](std::string_view key, int keyCode) {
                            handleKeyDown(key, keyCode);
                          });
  events.setKeyboardEvent(sdl2w::KeyboardEventCb::ON_KEY_UP,
                          [this](std::string_view key, int keyCode) {
                            handleKeyUp(key, keyCode);
                          });
}

void LayerManager::start() {
  bindEvents();

  auto initialize = [this]() {
    sdl2w::renderSplash(*window);
    return true;
  };
  auto initialized = []() {};
  auto frame = [this]() {
    window->getDraw().setBackgroundColor({10, 10, 10});

#ifndef __EMSCRIPTEN__
    if (window->getEvents().isKeyPressed("Escape")) {
      return false;
    }
#endif

    const auto deltaTime = static_cast<int>(window->getDeltaTime());
    if (hasStateManager()) {
      getStateManager()->update(deltaTime);
    }
    update(deltaTime);
    render(deltaTime);
    return true;
  };

  window->startRenderLoop(initialize, initialized, frame);
}

void LayerManager::scrubFromStack(const Layer* layer) {
  if (layer == nullptr) {
    return;
  }
  layerEventsStack.eraseIf([layer](Layer* entry) { return entry == layer; });
}

bool LayerManager::isLiveLayer(const Layer* layer) const {
  if (layer == nullptr || layer->shouldRemove()) {
    return false;
  }
  for (auto* entry : layers) {
    if (entry == layer) {
      return true;
    }
  }
  return false;
}

void LayerManager::activateLayerNoPush(Layer* layer) {
  if (layer == nullptr) {
    return;
  }
  for (auto* entry : layers) {
    if (entry->shouldRemove()) {
      continue;
    }
    if (entry == layer) {
      entry->turnOn();
    } else {
      entry->suspend();
    }
  }
}

void LayerManager::restoreFrontAfterClose() {
  while (!layerEventsStack.empty() && !isLiveLayer(layerEventsStack.back())) {
    layerEventsStack.popBack();
  }

  if (!layerEventsStack.empty()) {
    activateLayerNoPush(layerEventsStack.back());
    return;
  }

  Layer* fallback = getLastActiveLayer();
  if (fallback == nullptr) {
    return;
  }
  // Re-establish a baseline front so subsequent close/open cycles stay consistent.
  layerEventsStack.pushBack(fallback);
  activateLayerNoPush(fallback);
}

void LayerManager::removeLayer(const Layer* layer) {
  if (layer == nullptr) {
    return;
  }
  scrubFromStack(layer);
  layers.eraseIf([layer](Layer* entry) { return entry == layer; });
  delete layer;
}

void LayerManager::removeLayerAt(size_t index) {
  if (index < layers.size()) {
    Layer* layer = layers[index];
    scrubFromStack(layer);
    delete layer;
    layers.erase(layers.begin() + index);
  }
}

void LayerManager::clearLayers() {
  layerEventsStack.clear();
  for (auto layer : layers) {
    delete layer;
  }
  layers.clear();
}

void LayerManager::addLayer(Layer* layer) { layers.pushBack(layer); }

// set a layer to be the "front" layer, and suspend all other layers
void LayerManager::moveToFront(Layer* layer) {
  if (layer == nullptr || layer->shouldRemove()) {
    return;
  }

  LOG(DEBUG) << "LayerManager::moveToFront: moving layer to front: " << layer->getId()
             << LOG_ENDL;

  bool found = false;
  for (auto* entry : layers) {
    if (entry == layer) {
      found = true;
      break;
    }
  }
  if (!found) {
    LOG(ERROR)
        << "LayerManager::moveToFront: provided layer pointer not found in list of layers"
        << LOG_ENDL;
    return;
  }

  if (layerEventsStack.empty() || layerEventsStack.back() != layer) {
    layerEventsStack.pushBack(layer);
  }

  activateLayerNoPush(layer);
}

void LayerManager::closeLayer(Layer* layer) {
  if (layer == nullptr) {
    return;
  }
  LOG(DEBUG) << "LayerManager::closeLayer: closing layer: " << layer->getId()
             << LOG_ENDL;
  layer->remove();
  scrubFromStack(layer);
  restoreFrontAfterClose();
}

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

bmin::DynArray<Layer*>& LayerManager::getLayers() { return layers; }

const bmin::DynArray<Layer*>& LayerManager::getLayers() const { return layers; }

size_t LayerManager::getLayerCount() const { return layers.size(); }

Layer* LayerManager::getLayerAt(size_t index) {
  if (index < layers.size()) {
    return layers[index];
  }
  return nullptr;
}

Layer* LayerManager::getLayerById(std::string_view id) {
  for (auto& layer : layers) {
    if (layer->shouldRemove()) {
      continue;
    }
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
  bmin::DynArray<Layer*> layersToBeRemoved;
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
