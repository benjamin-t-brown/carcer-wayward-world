module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <string_view>
#include <string>
#include <optional>
#if __has_include(<SDL.h>)
#include <SDL.h>
#include <SDL_pixels.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#endif

export module carcer.ui.UiElement;
export import bmin.containers;
import bmin.string_interop;
export import carcer.state;
export import carcer.ui.SdlPixels;
export import carcer.ui.TextStyle;
import sdl2w;
#include "macros.h"

export {

// --- from ui/UiElement.h ---
// IWYU pragma: keep
 // IWYU pragma: keep

// prevents circular dependency
 // IWYU pragma: keep

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

} // export
