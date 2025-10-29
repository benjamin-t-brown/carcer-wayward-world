#include "UiElement.h"
#include "uiUtils.h"

namespace ui {

void UiEventObserver::onMouseDown(int x, int y, int button) {
  // noop
}
void UiEventObserver::onMouseUp(int x, int y, int button) {
  // noop
}
void UiEventObserver::onClick(int x, int y, int button) {
  // noop
}
void UiEventObserver::onMouseWheel(int x, int y, int delta) {
  // noop
}

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

bool UiElement::checkMouseDownEvent(int mouseX, int mouseY, int button) {
  if (isInBoundsScaled(mouseX, mouseY, this)) {
    isClicked = true;
    // Check children first (front to back)
    if (shouldPropagateEventsToChildren) {
      for (auto it = children.rbegin(); it != children.rend(); ++it) {
        if ((*it)->checkMouseDownEvent(mouseX, mouseY, button)) {
          return true;
        }
      }
    }

    for (auto& observer : eventObservers) {
      observer->onMouseDown(mouseX, mouseY, button);
    }

    return true;
  }

  return false;
}

bool UiElement::checkMouseUpEvent(int mouseX, int mouseY, int button) {
  if (shouldPropagateEventsToChildren) {
    // Check children first (front to back)
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      (*it)->checkMouseUpEvent(mouseX, mouseY, button);
    }
  }

  if (isInBoundsScaled(mouseX, mouseY, this)) {
    if (isClicked) {
      // click event happens when mouse up occurs inside this element
      // after a mouse down also occurred inside this element.
      for (auto& observer : eventObservers) {
        observer->onClick(mouseX, mouseY, button);
      }
    }
  }

  for (auto& observer : eventObservers) {
    observer->onMouseUp(mouseX, mouseY, button);
  }

  isClicked = false;

  return true;
}

bool UiElement::checkHoverEvent(int mouseX, int mouseY) {
  if (shouldPropagateEventsToChildren) {
    for (auto& child : children) {
      child->checkHoverEvent(mouseX, mouseY);
    }
  }

  if (isInBoundsScaled(mouseX, mouseY, this)) {
    isHovered = true;

    return true;
  } else {
    isHovered = false;
  }

  return false;
}

bool UiElement::checkMouseWheelEvent(int mouseX, int mouseY, int delta) {
  if (isInBoundsScaled(mouseX, mouseY, this)) {
    if (shouldPropagateEventsToChildren) {
      for (auto& child : children) {
        child->checkMouseWheelEvent(mouseX, mouseY, delta);
      }
    }

    for (auto& observer : eventObservers) {
      observer->onMouseWheel(mouseX, mouseY, delta);
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

void UiElement::addEventObserver(std::unique_ptr<UiEventObserver> observer) {
  eventObservers.push_back(std::move(observer));
}

void UiElement::removeEventObserver(UiEventObserver* observer) {
  eventObservers.erase(
      std::remove_if(eventObservers.begin(),
                     eventObservers.end(),
                     [observer](const std::unique_ptr<UiEventObserver>& obs) {
                       return obs.get() == observer;
                     }),
      eventObservers.end());
}

void UiElement::build() {
  // Default implementation - can be overridden
  // This is called whenever style or children change
}

void UiElement::render(int dt) {
  // Default implementation - render all children
  for (auto& child : children) {
    child->render(dt);
  }
}

} // namespace ui
