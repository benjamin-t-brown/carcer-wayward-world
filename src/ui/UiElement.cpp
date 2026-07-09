#include "UiElement.h"
#include "bmin/StringInterop.h"
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

UiElement* UiElement::getChildById(std::string_view searchId) {
  if (id == searchId) {
    return this;
  }

  for (auto& child : children) {
    auto found = child->getChildById(searchId);
    if (found != nullptr) {
      return found;
    }
  }

  return nullptr;
}

void UiElement::removeChildById(std::string_view childId) {
  children.eraseIf([childId](const bmin::UniquePtr<UiElement>& child) {
    return child->getId() == childId;
  });
}

void UiElement::setStyle(const BaseStyle& _style) { style = _style; }

BaseStyle& UiElement::getStyle() { return style; }

const BaseStyle& UiElement::getStyle() const { return style; }

const std::pair<int, int> UiElement::getDims() const {
  return {round(style.width * style.scale), round(style.height * style.scale)};
}

void UiElement::setId(const bmin::String& _id) { id = _id; }

const bmin::String& UiElement::getId() const { return id; }

bmin::DynArray<bmin::UniquePtr<UiElement>>& UiElement::getChildren() { return children; }

const bmin::DynArray<bmin::UniquePtr<UiElement>>& UiElement::getChildren() const {
  return children;
}

void UiElement::removeChildAtIndex(size_t index) {
  if (index < children.size()) {
    children.erase(static_cast<size_t>(index));
  }
}

void UiElement::addChild(UiElement* child) {
  children.pushBack(bmin::UniquePtr<UiElement>(child));
}

bool UiElement::checkMouseDownEvent(int mouseX,
                                    int mouseY,
                                    int button,
                                    bmin::DynArray<UiElement*> additionalElements) {
  if (isInBoundsScaled(mouseX, mouseY, this)) {
    isClicked = true;
    // Check children first (front to back)
    if (shouldPropagateEventsToChildren) {
      for (auto it = children.rbegin(); it != children.rend(); ++it) {
        auto& elem = (*it);
        if (elem->checkMouseDownEvent(mouseX, mouseY, button)) {
          return true;
        }
      }
      for (auto& additionalElement : additionalElements) {
        if (additionalElement->checkMouseDownEvent(mouseX, mouseY, button)) {
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

bool UiElement::checkMouseUpEvent(int mouseX,
                                  int mouseY,
                                  int button,
                                  bmin::DynArray<UiElement*> additionalElements) {
  if (shouldPropagateEventsToChildren) {
    // Check children first (front to back)
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
      (*it)->checkMouseUpEvent(mouseX, mouseY, button);
    }
    for (auto& additionalElement : additionalElements) {
      additionalElement->checkMouseUpEvent(mouseX, mouseY, button);
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

bool UiElement::checkHoverEvent(int mouseX,
                                int mouseY,
                                bmin::DynArray<UiElement*> additionalElements) {
  if (shouldPropagateEventsToChildren) {
    for (auto& child : children) {
      child->checkHoverEvent(mouseX, mouseY);
    }
    for (auto& additionalElement : additionalElements) {
      additionalElement->checkHoverEvent(mouseX, mouseY);
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

bool UiElement::checkMouseWheelEvent(int mouseX,
                                     int mouseY,
                                     int delta,
                                     bmin::DynArray<UiElement*> additionalElements) {
  if (isInBoundsScaled(mouseX, mouseY, this)) {
    if (shouldPropagateEventsToChildren) {
      for (auto& child : children) {
        child->checkMouseWheelEvent(mouseX, mouseY, delta);
      }
      for (auto& additionalElement : additionalElements) {
        additionalElement->checkMouseWheelEvent(mouseX, mouseY, delta);
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

void UiElement::addEventObserver(UiEventObserver* observer) {
  eventObservers.pushBack(bmin::UniquePtr<UiEventObserver>(observer));
}

void UiElement::removeEventObserver(UiEventObserver* observer) {
  eventObservers.eraseIf([observer](const bmin::UniquePtr<UiEventObserver>& obs) {
    return obs.get() == observer;
  });
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
