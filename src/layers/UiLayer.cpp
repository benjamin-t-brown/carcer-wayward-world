#include "layers/UiLayer.h"

#include "sdl2w/Events.h"
#include "sdl2w/Window.h"

namespace layers {

void UiLayer::addUiElement(ui::UiElement* element) {
  uiElements.pushBack(bmin::UniquePtr<ui::UiElement>(element));
}

void UiLayer::onMouseDown(int x, int y, int button) {
  for (auto it = uiElements.rbegin(); it != uiElements.rend(); ++it) {
    (*it)->checkMouseDownEvent(x, y, button);
  }
}

void UiLayer::onMouseUp(int x, int y, int button) {
  for (auto it = uiElements.rbegin(); it != uiElements.rend(); ++it) {
    (*it)->checkMouseUpEvent(x, y, button);
  }
}

void UiLayer::onMouseHover(int x, int y) {
  for (auto it = uiElements.rbegin(); it != uiElements.rend(); ++it) {
    (*it)->checkHoverEvent(x, y);
  }
}

void UiLayer::onMouseWheel(int x, int y, int direction) {
  for (auto it = uiElements.rbegin(); it != uiElements.rend(); ++it) {
    (*it)->checkMouseWheelEvent(x, y, direction);
  }
}

void UiLayer::update(int) {
  if (!window) {
    return;
  }
  const auto& events = window->getEvents();
  onMouseHover(events.mouseX, events.mouseY);
}

void UiLayer::render(int deltaTime) {
  for (auto& element : uiElements) {
    element->render(deltaTime);
  }
}

} // namespace layers
