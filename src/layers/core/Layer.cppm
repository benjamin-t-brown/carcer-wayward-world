module;
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <utility>

export module carcer.layers.Layer;
export import bmin.containers;
export import carcer.state;
export import carcer.ui.UiElement;
import bmin.string_interop;
import sdl2w;
import carcer.lib.StringUtil;
#include "macros.h"

export {

namespace layers {

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

  template <typename ActionT, typename Fn> void subscribeAction(Fn&& fn) {
    if (!hasStateManager()) {
      return;
    }
    getStateManager()->getActionBus().subscribe(
        this,
        std::type_index(typeid(ActionT)),
        [fn = std::forward<Fn>(fn)](state::AbstractAction& action, state::State& state) {
          if (auto* typed = dynamic_cast<ActionT*>(&action)) {
            fn(*typed, state);
          }
        });
  }

  sdl2w::Window* getWindow() const { return window; }

  virtual void update(int deltaTime);
  virtual void render(int deltaTime);
};

} // namespace layers

} // export

namespace layers {

Layer::Layer(sdl2w::Window* _window, std::string_view _id)
    : window(_window), id(strutil::fromStringView(_id)) {}

Layer::Layer(sdl2w::Window* _window, state::LayerId id)
    : Layer(_window, state::layerIdString(id)) {}

Layer::~Layer() {
  if (hasStateManager()) {
    getStateManager()->getActionBus().unsubscribe(this);
  }
}

bool Layer::assertInterfaces() const {
  if (!hasStateManager()) {
    LOG(ERROR) << "Layer::assertInterfaces: stateManager is not set" << LOG_ENDL;
    return false;
  }
  if (!hasDatabase()) {
    LOG(ERROR) << "Layer::assertInterfaces: database is not set" << LOG_ENDL;
    return false;
  }
  return true;
}

void Layer::setId(std::string_view _id) { id = strutil::fromStringView(_id); }

bmin::String Layer::getId() const { return id; }

void Layer::onMouseDown(int x, int y, int button) {
  if (state != LayerState::ON) {
    return;
  }

  // Check UI elements from back to front (reverse order for proper z-ordering)
  for (auto it = uiElements.rbegin(); it != uiElements.rend(); ++it) {
    if ((*it)->checkMouseDownEvent(x, y, button)) {
      // return; // Event handled, stop propagation
    }
  }
}

void Layer::onMouseUp(int x, int y, int button) {
  if (state != LayerState::ON) {
    return;
  }

  // Check UI elements from back to front (reverse order for proper z-ordering)
  for (auto it = uiElements.rbegin(); it != uiElements.rend(); ++it) {
    if ((*it)->checkMouseUpEvent(x, y, button)) {
      // return; // Event handled, stop propagation
    }
  }
}

void Layer::onMouseWheel(int x, int y, int dir) {
  if (state != LayerState::ON) {
    return;
  }

  // Check UI elements from back to front (reverse order for proper z-ordering)
  for (auto it = uiElements.rbegin(); it != uiElements.rend(); ++it) {
    if ((*it)->checkMouseWheelEvent(x, y, dir)) {
      // return; // Event handled, stop propagation
    }
  }
}

void Layer::onMouseHover(int x, int y) {
  if (state != LayerState::ON) {
    return;
  }

  // Check UI elements from back to front (reverse order for proper z-ordering)
  for (auto it = uiElements.rbegin(); it != uiElements.rend(); ++it) {
    if ((*it)->checkHoverEvent(x, y)) {
      // return; // Event handled, stop propagation
    }
  }
}

void Layer::onKeyDown(std::string_view key, int keyCode) {
  if (state != LayerState::ON) {
    return;
  }
}

void Layer::onKeyUp(std::string_view key, int keyCode) {
  if (state != LayerState::ON) {
    return;
  }
}

void Layer::turnOn() { state = LayerState::ON; }

void Layer::turnOff() { state = LayerState::OFF; }

void Layer::suspend() { state = LayerState::SUSPENDED; }

void Layer::remove() { removeFlag = true; }

bool Layer::shouldRemove() const { return removeFlag; }

LayerState Layer::getState() const { return state; }

void Layer::addUiElement(ui::UiElement* element) {
  uiElements.pushBack(bmin::UniquePtr<ui::UiElement>(element));
}

void Layer::update(int deltaTime) {
  auto& events = window->getEvents();
  auto mouseX = events.mouseX;
  auto mouseY = events.mouseY;

  // Check hover events for all buttons
  for (auto& elem : uiElements) {
    if (elem) {
      elem->checkHoverEvent(mouseX, mouseY);
    }
  }
}

void Layer::render(int deltaTime) {
  // Default implementation - render all children
  for (auto& child : uiElements) {
    child->render(deltaTime);
  }
}

} // namespace layers
