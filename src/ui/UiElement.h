#pragma once

#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "bmin/UniquePtr.h"
#include "sdl2w/Window.h"
#include "state/LayerManagerInterface.h"
#include "state/StateManagerInterface.h"
#include "ui/SdlPixels.h" // IWYU pragma: keep
#include "ui/TextStyle.h" // IWYU pragma: keep
#include <optional>
#include <string_view>
#include <utility>

// prevents circular dependency
#include "state/AbstractAction.h" // IWYU pragma: keep

namespace ui {

// Forward declaration
class UiElement;

// Base StateInterface that can dispatch actions
class StateInterface {
public:
  virtual ~StateInterface() = default;
  virtual void dispatchAction(const bmin::String& action, void* payload) = 0;
};

// Geometry-only style. Visual fields live on component props / private caches.
struct BaseStyle {
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
  float scale = 1.0f;
};

class UiEventObserver {
public:
  virtual ~UiEventObserver() = default;
  virtual void onMouseDown(int x, int y, int button);
  virtual void onMouseUp(int x, int y, int button);
  virtual void onClick(int x, int y, int button);
  virtual void onMouseWheel(int x, int y, int delta);
};

// Main UiElement base class
class UiElement : public state::StateManagerInterface,
                  public state::LayerManagerInterface {
protected:
  sdl2w::Window* window;
  UiElement* parent;
  bmin::DynArray<bmin::UniquePtr<UiElement>> children;
  std::optional<StateInterface*> stateInterface;
  BaseStyle style;
  bmin::String id;
  bmin::DynArray<bmin::UniquePtr<UiEventObserver>> eventObservers;
  bool shouldPropagateEventsToChildren = true;

public:
  bool isHovered = false;
  bool isClicked = false;
  // Constructor
  UiElement(sdl2w::Window* _window, UiElement* _parent = nullptr);
  virtual ~UiElement() = default;

  virtual UiElement* getChildById(std::string_view id);
  virtual void removeChildById(std::string_view id);

  // Layout API (style is private/protected; no public getStyle/setStyle)
  virtual void setPos(int x, int y);
  virtual void setScale(float scale);
  virtual std::pair<int, int> getPos() const;
  virtual const std::pair<int, int> getDims() const;

  // Id methods
  virtual void setId(const bmin::String& _id);
  virtual const bmin::String& getId() const;

  // Children methods
  virtual bmin::DynArray<bmin::UniquePtr<UiElement>>& getChildren();
  virtual const bmin::DynArray<bmin::UniquePtr<UiElement>>& getChildren() const;
  virtual void removeChildAtIndex(size_t index);
  virtual void addChild(UiElement* child);

  // Event handlers
  virtual bool checkMouseDownEvent(int mouseX,
                                   int mouseY,
                                   int button,
                                   bmin::DynArray<UiElement*> additionalElements = {});
  virtual bool checkMouseUpEvent(int mouseX,
                                 int mouseY,
                                 int button,
                                 bmin::DynArray<UiElement*> additionalElements = {});
  virtual bool checkHoverEvent(int mouseX,
                               int mouseY,
                               bmin::DynArray<UiElement*> additionalElements = {});
  virtual bool checkMouseWheelEvent(int mouseX,
                                    int mouseY,
                                    int delta,
                                    bmin::DynArray<UiElement*> additionalElements = {});
  virtual void checkResizeEvent(int width, int height);
  virtual void addEventObserver(UiEventObserver* observer);
  virtual void removeEventObserver(UiEventObserver* observer);

  // Build and render
  virtual void build();
  virtual void render(int dt);

  // Getters for window and parent
  sdl2w::Window* getWindow() const { return window; }
  UiElement* getParent() const { return parent; }
};

} // namespace ui
