#pragma once

#include "Layer.h"
#include "state/StateManagerInterface.h"
#include <vector>

namespace layers {

class LayerManager : public state::StateManagerInterface,
                     public state::DatabaseInterface {
private:
  std::vector<Layer*> layers;
  sdl2w::Window* window;
  std::vector<Layer*> layerEventsStack;

  void removeLayer(Layer* layer);
  void removeLayerAt(size_t index);
  void clearLayers();

public:
  explicit LayerManager(sdl2w::Window* _window);
  ~LayerManager();

  // Layer management
  void addLayer(Layer* layer);
  void moveToFront(Layer* layer);
  void popFront();

  // Event handling - pass events to layers from top to bottom
  void handleMouseDown(int x, int y, int button);
  void handleMouseUp(int x, int y, int button);
  void handleKeyDown(const std::string& key, int keyCode);
  void handleKeyUp(const std::string& key, int keyCode);

  // Getters
  std::vector<Layer*>& getLayers();
  const std::vector<Layer*>& getLayers() const;
  size_t getLayerCount() const;
  Layer* getLayerAt(size_t index);

  // Update and draw all active layers
  void update(int deltaTime);
  void render(int deltaTime);
};

} // namespace layers
