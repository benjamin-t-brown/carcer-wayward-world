module;
#include <cstddef>
#include <string>
#include <string_view>

export module carcer.layers;
export import bmin.containers;
export import carcer.state;
export import carcer.layers.Layer;
import sdl2w;

export {

namespace layers {

class LayerManager : public state::StateManagerInterface, public state::DatabaseInterface {
private:
  bmin::DynArray<Layer*> layers;
  sdl2w::Window* window;
  bmin::DynArray<Layer*> layerEventsStack;

  void removeLayer(const Layer* layer);
  void removeLayerAt(size_t index);
  void clearLayers();
  void scrubFromStack(const Layer* layer);
  bool isLiveLayer(const Layer* layer) const;
  void activateLayerNoPush(Layer* layer);
  void restoreFrontAfterClose();

public:
  explicit LayerManager(sdl2w::Window* _window);
  ~LayerManager();

  void addLayer(Layer* layer);
  void moveToFront(Layer* layer);
  void closeLayer(Layer* layer);

  void handleMouseDown(int x, int y, int button);
  void handleMouseUp(int x, int y, int button);
  void handleMouseWheel(int x, int y, int dir);
  void handleKeyDown(std::string_view key, int keyCode);
  void handleKeyUp(std::string_view key, int keyCode);

  bmin::DynArray<Layer*>& getLayers();
  const bmin::DynArray<Layer*>& getLayers() const;
  size_t getLayerCount() const;
  Layer* getLayerAt(size_t index);
  Layer* getLayerById(std::string_view id);
  Layer* getLayerById(state::LayerId id) {
    return getLayerById(state::layerIdString(id));
  }
  Layer* getLastActiveLayer();

  void update(int deltaTime);
  void render(int deltaTime);

  void showSpecialEvent(sdl2w::Window* window,
                        const bmin::String& eventId,
                        state::State& state);
};

} // namespace layers

} // export
