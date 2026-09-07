module;
#include <cstddef>
#include <string_view>
#include <utility>

export module carcer.ui.layers;
export import carcer.ui.core;
export import carcer.state;
export import bmin.containers;
import sdl2w;
import bmin.string_interop;

export namespace layers {

enum class LayerState { ON, OFF, SUSPENDED };

class Layer : public state::StateManagerInterface, public state::DatabaseInterface {
protected:
  sdl2w::Window* window;
  LayerState state = LayerState::ON;
  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> uiElements;
  bool removeFlag = false;
  bmin::String id;

public:
  explicit Layer(sdl2w::Window* _window, std::string_view _id = "");
  explicit Layer(sdl2w::Window* _window, state::LayerId id);
  virtual ~Layer();

  bool assertInterfaces() const;

  virtual void onMouseDown(int x, int y, int button);
  virtual void onMouseUp(int x, int y, int button);
  virtual void onMouseHover(int x, int y);
  virtual void onMouseWheel(int x, int y, int dir);
  virtual void onKeyDown(std::string_view key, int keyCode);
  virtual void onKeyUp(std::string_view key, int keyCode);

  void turnOn();
  void turnOff();
  void suspend();
  void remove();
  bool shouldRemove() const;
  LayerState getState() const;
  bmin::String getId() const;
  void setId(std::string_view id);

  void addUiElement(ui::UiElement* element);

  template <typename T> T* getUiElement(std::string_view elementId) {
    for (auto& elem : uiElements) {
      if (elem->getId() == elementId) {
        return dynamic_cast<T*>(elem.get());
      }
    }
    return nullptr;
  }

  template <state::ActionEvent Event, typename Fn> void subscribeAction(Fn&& fn) {
    if (!hasStateManager()) {
      return;
    }
    getStateManager()->getActionBus().subscribe(
        this,
        Event,
        [fn = std::forward<Fn>(fn)](state::AbstractAction& action, state::State& state) {
          fn(action, state);
        });
  }

  sdl2w::Window* getWindow() const { return window; }

  virtual void update(int deltaTime);
  virtual void render(int deltaTime);
};

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

Layer* createWorldLayer(sdl2w::Window* window, float mapScale = 1.f);
Layer* createInventoryLayer(sdl2w::Window* window);
Layer* createPickUpLayer(sdl2w::Window* window);

} // namespace layers
