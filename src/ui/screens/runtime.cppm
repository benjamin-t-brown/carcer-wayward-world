module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <string_view>
#include <string>
#include <optional>
#include <functional>

export module carcer.ui.screens.runtime;
export import carcer.ui.core;
export import carcer.model;
export import carcer.state;
export import bmin.containers;
export import carcer.lib.StringUtil;
export import carcer.ui.widgets.composites;
import sdl2w;
import carcer.actions;
import bmin.string_interop;
#include "macros.h"

export {

// --- from ui/helpers/modalLayoutFit.h ---
namespace ui {


// How ModalStandard / ModalSmall map width/height props to the final rect.
// CappedCentered (default): treat width/height as window dims, then cap + center.
// FullBleed: use width/height as the modal size as-is (no cap/center).
enum class LayoutFit { CappedCentered, FullBleed };

// Landscape max size class. Portrait still uses near-full margins for both.
enum class ModalSizeClass { Standard, Small };

struct LayoutRect {
  int x = 0;
  int y = 0;
  int width = 0;
  int height = 0;
};

// Portrait: near-full with small margins.
// Landscape Standard: ~500×min(500, h−50). Small: ~400×min(420, h−50).
LayoutRect computeCappedCenteredRect(int windowW,
                                     int windowH,
                                     ModalSizeClass sizeClass = ModalSizeClass::Standard);

// After a CappedCentered modal->setProps, sync the host page/minipage style to the
// fitted rect so getPos()+getDims() (often child[0] size) align with drawn controls.
// Call while host style still holds window width/height. Pass the same sizeClass
// the modal used.
void syncHostStyleToCappedCentered(BaseStyle& style,
                                   ModalSizeClass sizeClass = ModalSizeClass::Standard);

} // namespace ui

// --- from ui/helpers/keyboardShortcuts.h ---
namespace ui {

std::optional<state::WorldActionType>
getWorldActionFromKeyboardShortcut(std::string_view key, model::TurnMode turnMode);

struct MoveDelta {
  int dx = 0;
  int dy = 0;
};

std::optional<MoveDelta> getMoveDeltaForKey(std::string_view key);

bool isCancelActionKey(std::string_view key);

bool isConfirmActionKey(std::string_view key);

bool isCombatWaitKey(std::string_view key);

/** `m` / `M` — open combat spell-cast list (combat-only at call site). */
bool isOpenSpellCastKey(std::string_view key);

/** `r` / `R` — open magic setup (LayerMagic); always, combat or not. */
bool isOpenMagicSetupKey(std::string_view key);

/** Keys "1"-"6" → party index 0-5. */
std::optional<int> getPartyMemberIndexFromKey(std::string_view key);

/** Keys "a"-"z" / "A"-"Z" → pick-up list index 0-25. */
std::optional<int> getPickUpItemIndexFromKey(std::string_view key);

} // namespace ui

// --- from ui/helpers/worldActions.h ---
namespace ui {

void setHeldMoveActive(state::StateManager& stateManager, bool isActive);
void cancelCurrentWorldActionMode(state::StateManager& stateManager);
void showMagicSetupLayer(state::StateManager& stateManager, sdl2w::Window* window);
void showSpellCastLayer(state::StateManager& stateManager, sdl2w::Window* window);
void activateWorldAction(state::StateManager& stateManager,
                         state::WorldActionType worldActionType,
                         sdl2w::Window* window = nullptr);

} // namespace ui

} // export

export {

// --- from ui/observers/ObserverRemoveLayer.hpp ---
namespace ui {
class ObserverRemoveLayer : public ui::UiEventObserver,
                            public state::StateManagerInterface {
  bmin::String layerId;

public:
  ObserverRemoveLayer(const bmin::String& _layerId) : layerId(_layerId) {}
  ObserverRemoveLayer(std::string_view _layerId) : layerId(strutil::fromStringView(_layerId)) {}
  explicit ObserverRemoveLayer(state::LayerId id)
      : layerId(strutil::fromStringView(state::layerIdString(id))) {}

  void onClick(int mouseX, int mouseY, int button) override {
    LOG(INFO) << "ObserverRemoveLayer::onClick " << layerId << LOG_ENDL;
    auto stateManager = getStateManager();
    if (!stateManager || layerId.empty()) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(), state::actions::removeLayer(layerId), 0);
  }
};
} // namespace ui

} // export

export {

// Choice / continue clicks in a special event go through the action bus, not
// a back-pointer into layers::LayerSpecialEvent — UI observers don't reach
// into a concrete Layer. LayerSpecialEvent subscribes to react.
namespace ui {

class ObserverSpecialEventChoice : public ui::UiEventObserver,
                                   public state::StateManagerInterface {
  int choiceIndex;

public:
  explicit ObserverSpecialEventChoice(int _choiceIndex) : choiceIndex(_choiceIndex) {}

  void onClick(int mouseX, int mouseY, int button) override {
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(),
        state::actions::selectSpecialEventChoice(choiceIndex),
        0);
  }
};

class ObserverSpecialEventContinue : public ui::UiEventObserver,
                                     public state::StateManagerInterface {
public:
  void onClick(int mouseX, int mouseY, int button) override {
    auto stateManager = getStateManager();
    if (!stateManager) {
      return;
    }
    stateManager->enqueueAction(
        stateManager->getActionData(), state::actions::continueSpecialEvent(), 0);
  }
};

} // namespace ui

} // export

export {

// --- from ui/KeyboardHeldScroll.h ---
namespace ui {

enum class HeldScrollDirection { Up, Down };

// Hold-to-repeat keyboard scrolling for any SectionScrollable (or custom action).
// Wire from a Layer/Page: onKeyDown / onKeyUp / update. Timing matches world movement
// (first step immediately, 300ms pause, then every 50ms).
class KeyboardHeldScroll {
private:
  struct Binding {
    bmin::String key;
    std::function<void()> action;
  };

  struct Held {
    bool isActive = false;
    std::function<void()> action;
    bmin::String key;
    int dx = 0;
    int dy = 0;
    model::TimerStruct initialDelay = model::TimerStruct(300);
    model::TimerStruct moveDelay = model::TimerStruct(50);
  };

  const Binding* findBinding(std::string_view key) const;
  void applyHeldAction();

  bmin::DynArray<Binding> bindings;
  Held held;

public:
  void clearBindings();
  void stopScroll();

  // Bind a key to an arbitrary scroll/step action.
  void bindKey(std::string_view key, std::function<void()> action);

  // Bind a key to scrollUp/scrollDown on a section resolved each step (safe across UI
  // rebuilds).
  void bindSectionKey(std::string_view key,
                      std::function<SectionScrollable*()> sectionGetter,
                      HeldScrollDirection direction);

  // Returns true if the key matched a binding and was handled.
  bool onKeyDown(std::string_view key);
  void onKeyUp(std::string_view key);
  void update(int deltaTime, sdl2w::Window* window);
  bool isHolding() const { return held.isActive; }
};

} // namespace ui

} // export

export {

namespace layers {

enum class LayerState { ON, OFF, SUSPENDED };

class Layer : public state::StateManagerInterface, public state::DatabaseInterface {
protected:
  sdl2w::Window* window;
  LayerState state = LayerState::ON;
  bmin::DynArray<bmin::UniquePtr<ui::UiElement>> uiElements;
  bool removeFlag = false;
  bmin::String id;

public:
  explicit Layer(sdl2w::Window* _window, std::string_view _id = "");
  explicit Layer(sdl2w::Window* _window, state::LayerId id);
  virtual ~Layer();

  bool assertInterfaces() const;

  virtual void onMouseDown(int x, int y, int button);
  virtual void onMouseUp(int x, int y, int button);
  virtual void onMouseHover(int x, int y);
  virtual void onMouseWheel(int x, int y, int dir);
  virtual void onKeyDown(std::string_view key, int keyCode);
  virtual void onKeyUp(std::string_view key, int keyCode);

  void turnOn();
  void turnOff();
  void suspend();
  void remove();
  bool shouldRemove() const;
  LayerState getState() const;
  bmin::String getId() const;
  void setId(std::string_view id);

  void addUiElement(ui::UiElement* element);

  template <typename T> T* getUiElement(std::string_view elementId) {
    for (auto& elem : uiElements) {
      if (elem->getId() == elementId) {
        return dynamic_cast<T*>(elem.get());
      }
    }
    return nullptr;
  }

  template <state::ActionEvent Event, typename Fn> void subscribeAction(Fn&& fn) {
    if (!hasStateManager()) {
      return;
    }
    getStateManager()->getActionBus().subscribe(
        this,
        Event,
        [fn = std::forward<Fn>(fn)](state::AbstractAction& action, state::State& state) {
          fn(action, state);
        });
  }

  sdl2w::Window* getWindow() const { return window; }

  virtual void update(int deltaTime);
  virtual void render(int deltaTime);
};

} // namespace layers

} // export

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
  uiElements.pushBack(bmin::UniquePtr<ui::UiElement>(element));
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

export {

namespace layers {

class LayerManager : public state::StateManagerInterface, public state::DatabaseInterface {
private:
  bmin::DynArray<Layer*> layers;
  sdl2w::Window* window;
  bmin::DynArray<Layer*> layerEventsStack;

  void removeLayer(const Layer* layer);
  void removeLayerAt(size_t index);
  void clearLayers();
  void scrubFromStack(const Layer* layer);
  bool isLiveLayer(const Layer* layer) const;
  void activateLayerNoPush(Layer* layer);
  void restoreFrontAfterClose();

public:
  explicit LayerManager(sdl2w::Window* _window);
  ~LayerManager();

  void addLayer(Layer* layer);
  void moveToFront(Layer* layer);
  void closeLayer(Layer* layer);

  void handleMouseDown(int x, int y, int button);
  void handleMouseUp(int x, int y, int button);
  void handleMouseWheel(int x, int y, int dir);
  void handleKeyDown(std::string_view key, int keyCode);
  void handleKeyUp(std::string_view key, int keyCode);

  bmin::DynArray<Layer*>& getLayers();
  const bmin::DynArray<Layer*>& getLayers() const;
  size_t getLayerCount() const;
  Layer* getLayerAt(size_t index);
  Layer* getLayerById(std::string_view id);
  Layer* getLayerById(state::LayerId id) {
    return getLayerById(state::layerIdString(id));
  }
  Layer* getLastActiveLayer();

  void update(int deltaTime);
  void render(int deltaTime);

  void showSpecialEvent(sdl2w::Window* window,
                        const bmin::String& eventId,
                        state::State& state);
};

} // namespace layers

} // export
