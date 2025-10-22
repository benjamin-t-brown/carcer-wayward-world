#include "UiElement.h"

namespace ui {

UiElement::UiElement(sdl2w::Window* _window, UiElement* _parent)
    : window(_window), parent(_parent), stateInterface(std::nullopt) {}

void UiElement::setStateInterface(StateInterface* _stateInterface) {
  stateInterface = _stateInterface;
}

void UiElement::dispatchAction(const std::string& action, void* payload) {
  if (stateInterface.has_value()) {
    (*stateInterface)->dispatchAction(action, payload);
  } else if (parent != nullptr) {
    parent->dispatchAction(action, payload);
  }
}

void UiElement::dispatchUiAction(const std::string& action, void* payload) {
  // UI actions are handled internally, can be overridden by specific elements
  dispatchAction(action, payload);
}

UiElement* UiElement::getEntityById(const std::string& searchId) {
  if (id == searchId) {
    return this;
  }

  for (auto& child : children) {
    auto found = child->getEntityById(searchId);
    if (found != nullptr) {
      return found;
    }
  }

  return nullptr;
}

UiElement* UiElement::getTemplateById(const std::string& searchId) {
  // Can be overridden by specific elements for template lookup
  return getEntityById(searchId);
}

void UiElement::setStyle(const BaseStyle& _style) {
  style = _style;
  build();
}

BaseStyle& UiElement::getStyle() { return style; }

const BaseStyle& UiElement::getStyle() const { return style; }

const std::pair<int, int> UiElement::getDims() const {
  return {style.width, style.height};
}

void UiElement::setId(const std::string& _id) { id = _id; }

const std::string& UiElement::getId() const { return id; }

std::vector<std::unique_ptr<UiElement>>& UiElement::getChildren() { return children; }

const std::vector<std::unique_ptr<UiElement>>& UiElement::getChildren() const {
  return children;
}

void UiElement::removeChildAtIndex(size_t index) {
  if (index < children.size()) {
    children.erase(children.begin() + index);
    build();
  }
}

bool UiElement::checkClickEvent(int mouseX, int mouseY) {
  // Check if click is within bounds
  auto isInBounds = mouseX >= style.x && mouseX < style.x + style.width &&
                    mouseY >= style.y && mouseY < style.y + style.height;

  if (isInBounds) {
    // Check children first (front to back)
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      if ((*it)->checkClickEvent(mouseX, mouseY)) {
        return true;
      }
    }
    return true;
  }

  return false;
}

bool UiElement::checkHoverEvent(int mouseX, int mouseY) {
  // Check if hover is within bounds
  auto isInBounds = mouseX >= style.x && mouseX < style.x + style.width &&
                    mouseY >= style.y && mouseY < style.y + style.height;

  if (isInBounds) {
    // Check children first
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      if ((*it)->checkHoverEvent(mouseX, mouseY)) {
        return true;
      }
    }
    return true;
  }

  return false;
}

void UiElement::checkResizeEvent(int width, int height) {
  // Propagate resize event to children
  for (auto& child : children) {
    child->checkResizeEvent(width, height);
  }
}

void UiElement::build() {
  // Default implementation - can be overridden
  // This is called whenever style or children change
}

void UiElement::render() {
  // Default implementation - render all children
  for (auto& child : children) {
    child->render();
  }
}

} // namespace ui
