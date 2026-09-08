#pragma once

#include "Layer.h"
#include "bmin/DynArray.h"
#include "bmin/UniquePtr.h"
#include "state/StateManagerInterface.h"
#include "state/LayerRequest.h"
#include <functional>

namespace layers {

class LayerManager : public state::StateManagerInterface,
                     public state::DatabaseInterface {
public:
  using LayerFactory = std::function<bmin::UniquePtr<Layer>(const state::LayerRequest&)>;

private:
  // Sole authoritative order of the live layers; the last non-removed entry is
  // the front (active) layer. Sole owner of every layer.
  bmin::DynArray<bmin::UniquePtr<Layer>> layers;
  sdl2w::Window* window;
  LayerFactory layerFactory;

  void removeLayer(Layer* layer);
  bmin::UniquePtr<Layer> createLayer(const state::LayerRequest& request);
  // Drain the state command queue into the authoritative layer list.
  void applyLayerCommands();
  void applyPush(const state::LayerRequest& request);
  void applyRemove(state::LayerId id);
  // Move the entry at index to the back (front) without destroying it.
  void moveToBack(size_t index);
  // Make `target` the sole ON layer, firing onActivate every call.
  void focusLayer(Layer* target);
  // Transition-guarded: ensure the current front is ON and the rest SUSPENDED.
  void activateFront();
  void bindEvents();

public:
  explicit LayerManager(sdl2w::Window* _window, LayerFactory layerFactory = {});

  void start();

  // Layer management
  void addLayer(bmin::UniquePtr<Layer> layer);
  void moveToFront(Layer* layer);

  // Event handling - pass events to layers from top to bottom
  void handleMouseDown(int x, int y, int button);
  void handleMouseUp(int x, int y, int button);
  void handleMouseWheel(int x, int y, int dir);
  void handleKeyDown(std::string_view key, int keyCode);
  void handleKeyUp(std::string_view key, int keyCode);

  // Getters
  size_t getLayerCount() const;
  Layer* getLayerAt(size_t index);
  Layer* getLayerById(std::string_view id);
  Layer* getLastActiveLayer();
  // Read-only stack query for layers (via their back-pointer) and UI.
  bool containsLayer(state::LayerId id) const;

  // Update and draw all active layers
  void update(int deltaTime);
  void render(int deltaTime);
};

} // namespace layers
