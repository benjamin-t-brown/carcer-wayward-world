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
export import carcer.ui.widgets;
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
