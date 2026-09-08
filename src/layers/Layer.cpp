#include "Layer.h"
#include "lib/StringUtil.hpp"
#include "state/StateManager.h"

namespace layers {

Layer::Layer(sdl2w::Window* _window, std::string_view _id)
    : window(_window), id(strutil::fromStringView(_id)) {}

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

const bmin::String& Layer::getId() const { return id; }

void Layer::onMouseDown(int, int, int) {}
void Layer::onMouseUp(int, int, int) {}
void Layer::onMouseWheel(int, int, int) {}
void Layer::onMouseHover(int, int) {}
void Layer::onKeyDown(std::string_view, int) {}
void Layer::onKeyUp(std::string_view, int) {}

void Layer::onActivate() {}
void Layer::onSuspend() {}
void Layer::onDeactivate() {}

void Layer::turnOn() {
  state = LayerState::ON;
  onActivate();
}

void Layer::turnOff() {
  state = LayerState::OFF;
  onDeactivate();
}

void Layer::suspend() {
  state = LayerState::SUSPENDED;
  onSuspend();
}

void Layer::remove() { removeFlag = true; }

bool Layer::shouldRemove() const { return removeFlag; }

LayerState Layer::getState() const { return state; }

void Layer::update(int) {}
void Layer::render(int) {}

} // namespace layers
