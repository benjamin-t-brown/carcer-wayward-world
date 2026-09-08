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
  // Sole owner of the live layers. layerEventsStack below holds only non-owning
  // observers into this container.
  bmin::DynArray<bmin::UniquePtr<Layer>> layers;
  sdl2w::Window* window;
  bmin::DynArray<Layer*> layerEventsStack;
  LayerFactory layerFactory;

  void removeLayer(Layer* layer);
  void scrubFromStack(const Layer* layer);
  bool isLiveLayer(const Layer* layer) const;
  void activateLayerNoPush(Layer* layer);
  void restoreFrontAfterClose();
  bmin::UniquePtr<Layer> createLayer(const state::LayerRequest& request);
  void reconcileRequests();
  void bindEvents();

public:
  explicit LayerManager(sdl2w::Window* _window, LayerFactory layerFactory = {});

  void start();

  // Layer management
  void addLayer(bmin::UniquePtr<Layer> layer);
  void moveToFront(Layer* layer);
  // Mark layer for removal and restore the previous live front layer.
  void closeLayer(Layer* layer);

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

  // Update and draw all active layers
  void update(int deltaTime);
  void render(int deltaTime);
};

} // namespace layers
