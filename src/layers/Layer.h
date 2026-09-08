#pragma once

#include "bmin/String.h"
#include "state/AbstractAction.hpp"
#include "state/DatabaseInterface.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include <string_view>
#include <utility>

namespace sdl2w {
class Window;
}

namespace layers {

enum class LayerState { ON, OFF, SUSPENDED };

class Layer : public state::StateManagerInterface, public state::DatabaseInterface {
protected:
  sdl2w::Window* window;
  LayerState state = LayerState::ON;
  bool removeFlag = false;
  bmin::String id;

public:
  explicit Layer(sdl2w::Window* _window, std::string_view _id = "");
  virtual ~Layer();

  bool assertInterfaces() const;

  // Event handlers
  virtual void onMouseDown(int x, int y, int button);
  virtual void onMouseUp(int x, int y, int button);
  virtual void onMouseHover(int x, int y);
  virtual void onMouseWheel(int x, int y, int dir);
  virtual void onKeyDown(std::string_view key, int keyCode);
  virtual void onKeyUp(std::string_view key, int keyCode);
  virtual void onActivate();
  virtual void onSuspend();
  virtual void onDeactivate();

  // State management
  void turnOn();
  void turnOff();
  void suspend();
  void remove();
  bool shouldRemove() const;
  LayerState getState() const;
  const bmin::String& getId() const;
  void setId(std::string_view id);

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

  // Getters
  sdl2w::Window* getWindow() const { return window; }

  // Update and draw methods
  virtual void update(int deltaTime);
  virtual void render(int deltaTime);
};

} // namespace layers
