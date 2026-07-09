#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "bmin/UniquePtr.h"
#include "sdl2w/Window.h"
#include "state/AbstractAction.h"
#include "state/DatabaseInterface.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "ui/UiElement.h"
#include <string_view>

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
  virtual ~Layer();

  bool assertInterfaces() const;

  // Event handlers
  virtual void onMouseDown(int x, int y, int button);
  virtual void onMouseUp(int x, int y, int button);
  virtual void onMouseHover(int x, int y);
  virtual void onMouseWheel(int x, int y, int dir);
  virtual void onKeyDown(std::string_view key, int keyCode);
  virtual void onKeyUp(std::string_view key, int keyCode);

  // State management
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

  // Getters
  sdl2w::Window* getWindow() const { return window; }

  // Update and draw methods
  virtual void update(int deltaTime);
  virtual void render(int deltaTime);
};

} // namespace layers
