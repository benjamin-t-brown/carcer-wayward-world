#pragma once

#include "lib/sdl2w/Window.h"
#include "state/DatabaseInterface.h"
#include "state/StateManagerInterface.h"
#include <memory>
#include <vector>

namespace ui {
class UiElement;
}
namespace db {
class Database;
} // namespace db

namespace layers {

enum class LayerState { ON, OFF, SUSPENDED };

class Layer : public state::StateManagerInterface, public state::DatabaseInterface {
protected:
  sdl2w::Window* window;
  LayerState state = LayerState::ON;
  std::vector<std::unique_ptr<ui::UiElement>> uiElements;
  bool removeFlag = false;

public:
  explicit Layer(sdl2w::Window* _window);
  virtual ~Layer() = default;

  // Event handlers
  virtual void onMouseDown(int x, int y, int button);
  virtual void onMouseUp(int x, int y, int button);
  virtual void onMouseHover(int x, int y);
  virtual void onKeyDown(const std::string& key, int keyCode);
  virtual void onKeyUp(const std::string& key, int keyCode);

  // State management
  void turnOn();
  void turnOff();
  void suspend();
  void remove();
  bool shouldRemove() const;
  LayerState getState() const;

  void addUiElement(std::unique_ptr<ui::UiElement> element);

  // Getters
  sdl2w::Window* getWindow() const { return window; }

  // Update and draw methods
  virtual void update(int deltaTime);
  virtual void render(int deltaTime);
};

} // namespace layers
