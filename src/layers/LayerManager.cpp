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
#include "sdl2w/Draw.h"
#include "sdl2w/Events.h"
#include "sdl2w/Init.h"
#include "sdl2w/Logger.h"
#include "sdl2w/Window.h"
#include "state/State.hpp"
#include <exception>

namespace layers {

LayerManager::LayerManager(sdl2w::Window* _window, LayerFactory layerFactory)
    : window(_window), layerFactory(std::move(layerFactory)) {}

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

void LayerManager::moveToBack(size_t index) {
  if (index + 1 >= layers.size()) {
    return;
  }
  bmin::UniquePtr<Layer> entry = bmin::move(layers[index]);
  layers.erase(index);
  layers.pushBack(bmin::move(entry));
}

// Make target the sole ON layer, unconditionally firing onActivate so that
// opening (or re-opening) a layer always runs its activation logic.
void LayerManager::focusLayer(Layer* target) {
  if (target == nullptr) {
    return;
  }
  for (const auto& entry : layers) {
    if (entry->shouldRemove()) {
      continue;
    }
    if (entry.get() == target) {
      entry->turnOn();
    } else if (entry->getState() != LayerState::SUSPENDED) {
      entry->suspend();
    }
  }
}

// Transition-guarded reconciliation: the last live layer becomes the front and
// everything else suspends. Fires callbacks only on real state changes so it is
// safe to run after removals without spurious onActivate/onSuspend.
void LayerManager::activateFront() {
  Layer* front = getLastActiveLayer();
  for (const auto& entry : layers) {
    if (entry->shouldRemove()) {
      continue;
    }
    if (entry.get() == front) {
      if (entry->getState() != LayerState::ON) {
        entry->turnOn();
      }
    } else if (entry->getState() != LayerState::SUSPENDED) {
      entry->suspend();
    }
  }
}

void LayerManager::removeLayer(Layer* layer) {
  if (layer == nullptr) {
    return;
  }
  layer->turnOff();
  layers.eraseIf(
      [layer](const bmin::UniquePtr<Layer>& entry) { return entry.get() == layer; });
}

void LayerManager::addLayer(bmin::UniquePtr<Layer> layer) {
  if (!layer) {
    return;
  }
  layer->setLayerManager(this);
  layers.pushBack(bmin::move(layer));
}

// Bring an existing layer to the front and activate it.
void LayerManager::moveToFront(Layer* layer) {
  if (layer == nullptr || layer->shouldRemove()) {
    return;
  }

  LOG(DEBUG) << "LayerManager::moveToFront: moving layer to front: " << layer->getId()
             << LOG_ENDL;

  for (size_t i = 0; i < layers.size(); ++i) {
    if (layers[i].get() == layer) {
      moveToBack(i);
      focusLayer(layer);
      return;
    }
  }
  LOG(ERROR)
      << "LayerManager::moveToFront: provided layer pointer not found in list of layers"
      << LOG_ENDL;
}

void LayerManager::applyPush(const state::LayerRequest& request) {
  const auto idString = state::layerIdString(request.id);
  for (size_t i = 0; i < layers.size(); ++i) {
    if (layers[i]->shouldRemove()) {
      continue;
    }
    if (bmin::toStringView(layers[i]->getId()) == idString) {
      Layer* target = layers[i].get();
      moveToBack(i);
      focusLayer(target);
      return;
    }
  }
  auto layer = createLayer(request);
  if (!layer || layer->shouldRemove()) {
    // Factory rejected the request (e.g. missing item/event); drop it.
    return;
  }
  Layer* raw = layer.get();
  addLayer(bmin::move(layer));
  focusLayer(raw);
}

void LayerManager::applyRemove(state::LayerId id) {
  const auto idString = state::layerIdString(id);
  for (const auto& entry : layers) {
    if (entry->shouldRemove()) {
      continue;
    }
    if (bmin::toStringView(entry->getId()) == idString) {
      // Deferred: actual erase happens in update() so callbacks that fire during
      // removal cannot invalidate the layer currently being iterated.
      entry->remove();
    }
  }
}

void LayerManager::applyLayerCommands() {
  auto* stateManager = getStateManager();
  if (!stateManager) {
    return;
  }
  auto& queue = stateManager->getState().uiState.layerCommands;
  if (queue.empty()) {
    return;
  }
  // Snapshot and clear first so any command a layer enqueues while being
  // created/activated lands in the next update rather than this drain.
  const auto commands = queue;
  queue.clear();
  for (const auto& command : commands) {
    switch (command.type) {
    case state::LayerCommandType::Push:
      applyPush(command.request);
      break;
    case state::LayerCommandType::Remove:
      applyRemove(command.request.id);
      break;
    }
  }
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

size_t LayerManager::getLayerCount() const { return layers.size(); }

Layer* LayerManager::getLayerAt(size_t index) {
  if (index < layers.size()) {
    return layers[index].get();
  }
  return nullptr;
}

Layer* LayerManager::getLayerById(std::string_view id) {
  for (const auto& layer : layers) {
    if (layer->shouldRemove()) {
      continue;
    }
    if (bmin::toStringView(layer->getId()) == id) {
      return layer.get();
    }
  }
  return nullptr;
}

Layer* LayerManager::getLastActiveLayer() {
  if (layers.empty()) {
    return nullptr;
  }
  for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
    auto* layer = (*it).get();
    if (layer && !layer->shouldRemove()) {
      return layer;
    }
  }
  return nullptr;
}

bmin::UniquePtr<Layer> LayerManager::createLayer(const state::LayerRequest& request) {
  if (layerFactory) {
    return layerFactory(request);
  }
  switch (request.id) {
  case state::LayerId::World:
    return bmin::UniquePtr<Layer>(new LayerWorld(window));
  case state::LayerId::Inventory:
    return bmin::UniquePtr<Layer>(new LayerInventory(window));
  case state::LayerId::InventoryContext:
    return bmin::UniquePtr<Layer>(new LayerInventoryContext(window, request.a, request.b));
  case state::LayerId::Magic:
    return bmin::UniquePtr<Layer>(new LayerMagic(window));
  case state::LayerId::SpellCast:
    return bmin::UniquePtr<Layer>(new LayerSpellCast(window, request.a));
  case state::LayerId::SpellInfo:
    return bmin::UniquePtr<Layer>(new LayerSpellInfo(window, request.a));
  case state::LayerId::EquipRunes:
    return bmin::UniquePtr<Layer>(new LayerEquipRunes(window, request.a));
  case state::LayerId::PickUp:
    return request.hasPosition
               ? bmin::UniquePtr<Layer>(new LayerPickUp(window, request.x, request.y))
               : bmin::UniquePtr<Layer>(new LayerPickUp(window));
  case state::LayerId::PickUpContext: {
    auto* stateManager = getStateManager();
    if (!stateManager) {
      return bmin::UniquePtr<Layer>();
    }
    for (const auto& item : stateManager->getState().world.activeMap.items) {
      if (item.id == request.a) {
        return bmin::UniquePtr<Layer>(new LayerPickUpContext(window, item));
      }
    }
    return bmin::UniquePtr<Layer>();
  }
  case state::LayerId::DropConfirm:
    return bmin::UniquePtr<Layer>(new LayerDropConfirm(window, request.a, request.b));
  case state::LayerId::GiveContext:
    return bmin::UniquePtr<Layer>(new LayerGiveContext(window, request.a, request.b));
  case state::LayerId::PopupText:
    return bmin::UniquePtr<Layer>(new LayerPopupText(window, request.a, request.b));
  case state::LayerId::SpecialEvent: {
    auto* stateManager = getStateManager();
    auto* database = getDatabase();
    if (!stateManager || !database) {
      return bmin::UniquePtr<Layer>();
    }
    try {
      const auto& event = database->getGameEvent(request.a.sliceView());
      return bmin::UniquePtr<Layer>(
          new LayerSpecialEvent(window,
                                event,
                                database->getGameEvents(),
                                stateManager->getState().specialEventStorage));
    } catch (const std::exception& error) {
      LOG(ERROR) << "LayerManager: cannot create special-event layer: "
                 << error.what() << LOG_ENDL;
      return bmin::UniquePtr<Layer>();
    }
  }
  }
  return bmin::UniquePtr<Layer>();
}

bool LayerManager::containsLayer(state::LayerId id) const {
  const auto idString = state::layerIdString(id);
  for (const auto& entry : layers) {
    if (entry->shouldRemove()) {
      continue;
    }
    if (bmin::toStringView(entry->getId()) == idString) {
      return true;
    }
  }
  return false;
}

void LayerManager::update(int deltaTime) {
  applyLayerCommands();

  bmin::DynArray<Layer*> layersToBeRemoved;
  for (unsigned int i = 0; i < layers.size(); i++) {
    auto& layer = layers[i];
    if (layer->getState() == LayerState::ON) {
      layer->update(deltaTime);
    }
    if (layer->shouldRemove()) {
      layersToBeRemoved.pushBack(layer.get());
    }
  }
  for (auto& layer : layersToBeRemoved) {
    removeLayer(layer);
  }
  // A layer that removed itself (or was removed by command) may have exposed a
  // new front; reactivate it. Transition-guarded, so no-op when nothing changed.
  if (!layersToBeRemoved.empty()) {
    activateFront();
  }
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
