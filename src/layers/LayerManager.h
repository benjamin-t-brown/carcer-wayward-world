#pragma once

#include "Layer.h"
#include "bmin/DynArray.h"
#include "state/StateManagerInterface.h"

namespace layers {

class LayerManager : public state::StateManagerInterface,
                     public state::DatabaseInterface {
private:
  bmin::DynArray<Layer*> layers;
  sdl2w::Window* window;
  bmin::DynArray<Layer*> layerEventsStack;

  void removeLayer(const Layer* layer);
  void removeLayerAt(size_t index);
  void clearLayers();

public:
  explicit LayerManager(sdl2w::Window* _window);
  ~LayerManager();

  // Layer management
  void addLayer(Layer* layer);
  void moveToFront(Layer* layer);

  // void popFront();

  // Event handling - pass events to layers from top to bottom
  void handleMouseDown(int x, int y, int button);
  void handleMouseUp(int x, int y, int button);
  void handleMouseWheel(int x, int y, int dir);
  void handleKeyDown(std::string_view key, int keyCode);
  void handleKeyUp(std::string_view key, int keyCode);

  // Getters
  bmin::DynArray<Layer*>& getLayers();
  const bmin::DynArray<Layer*>& getLayers() const;
  size_t getLayerCount() const;
  Layer* getLayerAt(size_t index);
  Layer* getLayerById(std::string_view id);
  Layer* getLastActiveLayer();

  // Update and draw all active layers
  void update(int deltaTime);
  void render(int deltaTime);
};

} // namespace layers
