#include "LayerManager.h"
#include "bmin/StringInterop.h"
#include "layers/ui/LayerDropConfirm.h"
#include "layers/ui/LayerEquipRunes.h"
#include "layers/ui/LayerGiveContext.h"
#include "layers/ui/LayerInventory.h"
#include "layers/ui/LayerInventoryContext.h"
#include "layers/ui/LayerMagic.h"
#include "layers/ui/LayerPickUp.h"
#include "layers/ui/LayerPickUpContext.h"
#include "layers/ui/LayerPopupText.h"
#include "layers/ui/LayerSpecialEvent.h"
#include "layers/ui/LayerSpellCast.h"
#include "layers/ui/LayerSpellInfo.h"
#include "layers/ui/LayerWorld.h"
#include "layers/createInventoryLayer.h"
#include "layers/createPickUpLayer.h"
#include "layers/createWorldLayer.h"
#include "sdl2w/Draw.h"
#include "sdl2w/Events.h"
#include "sdl2w/Init.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "state/State.h"
#include <exception>

namespace layers {

LayerManager::LayerManager(sdl2w::Window* _window, LayerFactory layerFactory)
    : window(_window), layerFactory(std::move(layerFactory)) {}

LayerManager::~LayerManager() { clearLayers(); }

void LayerManager::bindEvents() {
  if (!window) {
    return;
  }
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
                       [this](int x, int y, int direction) {
                         handleMouseWheel(x, y, direction);
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
  if (!window) {
    return;
  }
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
    if (auto* stateManager = getStateManager()) {
      stateManager->update(deltaTime);
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

void LayerManager::removeLayer(Layer* layer) {
  if (layer == nullptr) {
    return;
  }
  scrubFromStack(layer);
  layers.eraseIf([layer](Layer* entry) { return entry == layer; });
  layer->turnOff();
  delete layer;
}

void LayerManager::removeLayerAt(size_t index) {
  if (index < layers.size()) {
    Layer* layer = layers[index];
    scrubFromStack(layer);
    layer->turnOff();
    delete layer;
    layers.erase(layers.begin() + index);
  }
}

void LayerManager::clearLayers() {
  layerEventsStack.clear();
  for (auto layer : layers) {
    layer->turnOff();
    delete layer;
  }
  layers.clear();
}

void LayerManager::addLayer(Layer* layer) {
  if (!layer) {
    return;
  }
  layers.pushBack(layer);
  if (auto* stateManager = getStateManager()) {
    if (auto id = state::layerIdFromString(bmin::toStringView(layer->getId()))) {
      bool present = false;
      for (const auto& request : stateManager->getState().uiState.layerStack) {
        if (request.id == *id) {
          present = true;
          break;
        }
      }
      if (!present) {
        state::pushLayerRequest(stateManager->getState(), state::LayerRequest{.id = *id});
      }
    }
  }
}

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
  if (auto* stateManager = getStateManager()) {
    if (auto id = state::layerIdFromString(bmin::toStringView(layer->getId()))) {
      state::removeLayerRequest(stateManager->getState(), *id);
    }
  }
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
    if (bmin::toStringView(layer->getId()) == id) {
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

Layer* LayerManager::createLayer(const state::LayerRequest& request) {
  if (layerFactory) {
    return layerFactory(request);
  }
  switch (request.id) {
  case state::LayerId::World:
    return createWorldLayer(window);
  case state::LayerId::Inventory:
    return createInventoryLayer(window);
  case state::LayerId::InventoryContext:
    return new LayerInventoryContext(window, request.a, request.b);
  case state::LayerId::Magic:
    return new LayerMagic(window);
  case state::LayerId::SpellCast:
    return new LayerSpellCast(window, request.a);
  case state::LayerId::SpellInfo:
    return new LayerSpellInfo(window, request.a);
  case state::LayerId::EquipRunes:
    return new LayerEquipRunes(window, request.a);
  case state::LayerId::PickUp:
    return request.hasPosition
               ? static_cast<Layer*>(new LayerPickUp(window, request.x, request.y))
               : createPickUpLayer(window);
  case state::LayerId::PickUpContext: {
    auto* stateManager = getStateManager();
    if (!stateManager) {
      return nullptr;
    }
    for (const auto& item : stateManager->getState().world.activeMap.items) {
      if (item.id == request.a) {
        return new LayerPickUpContext(window, item);
      }
    }
    return nullptr;
  }
  case state::LayerId::DropConfirm:
    return new LayerDropConfirm(window, request.a, request.b);
  case state::LayerId::GiveContext:
    return new LayerGiveContext(window, request.a, request.b);
  case state::LayerId::PopupText:
    return new LayerPopupText(window, request.a, request.b);
  case state::LayerId::SpecialEvent: {
    auto* stateManager = getStateManager();
    auto* database = getDatabase();
    if (!stateManager || !database) {
      return nullptr;
    }
    try {
      const auto& event = database->getGameEvent(request.a.sliceView());
      return new LayerSpecialEvent(window,
                                   event,
                                   database->getGameEvents(),
                                   stateManager->getState().specialEventStorage);
    } catch (const std::exception& error) {
      LOG(ERROR) << "LayerManager: cannot create special-event layer: "
                 << error.what() << LOG_ENDL;
      return nullptr;
    }
  }
  }
  return nullptr;
}

void LayerManager::reconcileRequests() {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  auto& requests = stateManager->getState().uiState.layerStack;

  for (auto* layer : layers) {
    const auto id = state::layerIdFromString(bmin::toStringView(layer->getId()));
    if (!id || layer->shouldRemove()) {
      continue;
    }
    bool requested = false;
    for (const auto& request : requests) {
      if (request.id == *id) {
        requested = true;
        break;
      }
    }
    if (!requested) {
      layer->remove();
      scrubFromStack(layer);
    }
  }

  const auto requestedLayers = requests;
  for (const auto& request : requestedLayers) {
    const auto id = state::layerIdString(request.id);
    if (!getLayerById(id)) {
      if (auto* layer = createLayer(request)) {
        if (layer->shouldRemove()) {
          delete layer;
          state::removeLayerRequest(stateManager->getState(), request.id);
        } else {
          addLayer(layer);
        }
      } else {
        state::removeLayerRequest(stateManager->getState(), request.id);
      }
    }
  }

  if (!requests.empty()) {
    if (auto* front = getLayerById(state::layerIdString(requests.back().id))) {
      if (front->getState() != LayerState::ON || layerEventsStack.empty() ||
          layerEventsStack.back() != front) {
        moveToFront(front);
      }
    }
  } else {
    restoreFrontAfterClose();
  }
}

void LayerManager::update(int deltaTime) {
  reconcileRequests();
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
