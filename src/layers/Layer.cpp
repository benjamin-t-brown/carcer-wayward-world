#include "Layer.h"
#include "state/StateManager.h"
#include "ui/UiElement.h"

namespace layers {

Layer::Layer(sdl2w::Window* _window, std::string_view _id) : window(_window), id(_id) {}

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

void Layer::setId(std::string_view _id) { id = _id; }

std::string Layer::getId() const { return id; }

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
  uiElements.push_back(std::unique_ptr<ui::UiElement>(element));
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
