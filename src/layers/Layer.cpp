module;
#include <string_view>

module carcer.ui.layers;
import carcer.lib.StringUtil;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

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
  for (auto it = uiElements.rbegin(); it != uiElements.rend(); ++it) {
    (*it)->checkMouseDownEvent(x, y, button);
  }
}

void Layer::onMouseUp(int x, int y, int button) {
  if (state != LayerState::ON) {
    return;
  }
  for (auto it = uiElements.rbegin(); it != uiElements.rend(); ++it) {
    (*it)->checkMouseUpEvent(x, y, button);
  }
}

void Layer::onMouseWheel(int x, int y, int dir) {
  if (state != LayerState::ON) {
    return;
  }
  for (auto it = uiElements.rbegin(); it != uiElements.rend(); ++it) {
    (*it)->checkMouseWheelEvent(x, y, dir);
  }
}

void Layer::onMouseHover(int x, int y) {
  if (state != LayerState::ON) {
    return;
  }
  for (auto it = uiElements.rbegin(); it != uiElements.rend(); ++it) {
    (*it)->checkHoverEvent(x, y);
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
  for (auto& elem : uiElements) {
    if (elem) {
      elem->checkHoverEvent(mouseX, mouseY);
    }
  }
}

void Layer::render(int deltaTime) {
  for (auto& child : uiElements) {
    child->render(deltaTime);
  }
}

} // namespace layers
