module;
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <functional>
#include <optional>
#include <string_view>
#include <typeinfo>
#include <cxxabi.h>
#include <typeindex>
#include <array>

export module carcer.state;
export import bmin.containers;
export import carcer.db;
export import carcer.model;
export import carcer.data;
import sdl2w;
#include "macros.h"

export {

namespace state {

struct State;

enum class LayerId {
  World,
  Inventory,
  InventoryContext,
  Magic,
  SpellCast,
  SpellInfo,
  EquipRunes,
  PickUp,
  PickUpContext,
  DropConfirm,
  GiveContext,
  PopupText,
  SpecialEvent,
};

inline std::string_view layerIdString(LayerId id) {
  switch (id) {
  case LayerId::World:
    return "layer_world";
  case LayerId::Inventory:
    return "layer_inventory";
  case LayerId::InventoryContext:
    return "layer_inventory_context";
  case LayerId::Magic:
    return "layer_magic";
  case LayerId::SpellCast:
    return "layer_spell_cast";
  case LayerId::SpellInfo:
    return "layer_spell_info";
  case LayerId::EquipRunes:
    return "layer_equip_runes";
  case LayerId::PickUp:
    return "layer_pick_up";
  case LayerId::DropConfirm:
    return "layer_drop_confirm";
  case LayerId::GiveContext:
    return "layer_give_context";
  case LayerId::PopupText:
    return "layer_popup_text";
  case LayerId::PickUpContext:
    return "layer_pick_up_context";
  case LayerId::SpecialEvent:
    return "layer_special_event";
  }
  return {};
}

inline std::optional<LayerId> layerIdFromString(std::string_view s) {
  if (s == "layer_world") return LayerId::World;
  if (s == "layer_inventory") return LayerId::Inventory;
  if (s == "layer_inventory_context") return LayerId::InventoryContext;
  if (s == "layer_magic") return LayerId::Magic;
  if (s == "layer_spell_cast") return LayerId::SpellCast;
  if (s == "layer_spell_info") return LayerId::SpellInfo;
  if (s == "layer_equip_runes") return LayerId::EquipRunes;
  if (s == "layer_pick_up") return LayerId::PickUp;
  if (s == "layer_pick_up_context") return LayerId::PickUpContext;
  if (s == "layer_drop_confirm") return LayerId::DropConfirm;
  if (s == "layer_give_context") return LayerId::GiveContext;
  if (s == "layer_popup_text") return LayerId::PopupText;
  if (s == "layer_special_event") return LayerId::SpecialEvent;
  return std::nullopt;
}

/**
 * A UI-requested overlay layer, queued on State so actions never need to
 * import carcer.layers -- actions touch only State; LayerManager (which
 * already sees carcer.state) is the only thing that names concrete Layer*
 * types.
 *
 * LayerManager::update() is meant to reconcile this list against its live
 * Layer* objects each frame (construct/destroy to match) -- reconciliation
 * itself is not yet implemented; that's real feature work (DB lookups for
 * e.g. SpecialEvent's GameEvent, re-resolving PickUpContext's ItemInstance by
 * id) left for when the main loop is wired up (see MODULES.md, "Phase 3a").
 * Field meaning is per-LayerId; unused fields stay default:
 *   InventoryContext: a=itemId, b=itemName
 *   SpellCast:        a=chId
 *   SpellInfo:        a=spellName
 *   EquipRunes:       a=characterPlayerId
 *   PickUp:           x,y=container tile (both 0 => floor pickup, no container)
 *   PickUpContext:    a=itemId (reconciler re-resolves the ItemInstance)
 *   DropConfirm:      a=characterPlayerId, b=itemId
 *   GiveContext:      a=fromCharacterPlayerId, b=itemId
 *   PopupText:        a=title, b=text
 *   SpecialEvent:     a=eventId
 *   World, Inventory, Magic: no payload
 */
struct LayerRequest {
  LayerId id;
  bmin::String a;
  bmin::String b;
  int x = 0;
  int y = 0;
};

/** Just the LayerManager pointer seam -- how actions/layers reach the live
    LayerManager without importing carcer.layers. Opening/closing layers goes
    through State::uiState.layerStack (LayerRequest) instead of a callback
    vtable; LayerManager::update() is meant to reconcile that list each frame
    (not yet implemented). */
class LayerManagerInterface {
private:
  static void* layerManager;

public:
  virtual ~LayerManagerInterface() = default;

  static void setLayerManager(void* _layerManager);
  static void* getLayerManager();
};

class DatabaseInterface {
private:
  static db::Database* database;

protected:
  static db::Database* getDatabase();
  bool hasDatabase() const { return database != nullptr; }

public:
  static void setDatabase(db::Database* _database);
};

struct Triggers {
  std::optional<bmin::String> pendingSpecialEventId;
  std::optional<model::TravelTrigger> pendingTravel;
  bool mapChangedThisTick = false;
};

struct UserSettings {
  int fontScale = 0;
  int floatingNotificationDurationMs = 3000;
};

enum class UiFloatingNotificationType {
  INFO,
  WARNING,
  ERROR,
};

struct UiFloatingNotification {
  bmin::String id;
  bmin::String message;
  UiFloatingNotificationType type;
  model::TimerStruct timer;
};

struct HeldMove {
  bool isActive = false;
  bmin::String key;
  int dx = 0;
  int dy = 0;
  model::TimerStruct initialDelay = model::TimerStruct(300);
  model::TimerStruct moveDelay = model::TimerStruct(50);
};

struct UiState {
  bmin::DynArray<UiFloatingNotification> floatingNotifications;
  HeldMove heldMove;
  /** HUD / inventory UI selection only — does not drive map movement. */
  bmin::String selectedPartyMemberId;
  /** Overlay layers the UI wants open, in stack order. The base LayerWorld is
      added directly at startup (see setupTestUi-style bootstrap), not through
      here -- this is for user-triggered overlays only. See LayerRequest. */
  bmin::DynArray<LayerRequest> layerStack;
};

struct State {
  UiState uiState;
  UserSettings settings;
  model::Player player;
  model::World world;
  Triggers triggers;
  bmin::Map<bmin::String, model::MapInstance> mapInstances;
  bmin::Map<bmin::String, bmin::String> specialEventStorage;

  model::TurnMode turnMode = model::TurnMode::TURN_TOWN;

  int playerMovementCount = 0;

  bmin::DynArray<bmin::String> soundsToPlay;
};

/** Request that a layer be open, replacing any existing request for the same
    id (re-opening with new params rather than stacking duplicates). */
inline void pushLayerRequest(State& state, LayerRequest request) {
  for (auto& existing : state.uiState.layerStack) {
    if (existing.id == request.id) {
      existing = std::move(request);
      return;
    }
  }
  state.uiState.layerStack.pushBack(std::move(request));
}

/** Request that a layer be closed. No-op if it wasn't open. */
inline void removeLayerRequest(State& state, LayerId id) {
  state.uiState.layerStack.eraseIf(
      [id](const LayerRequest& r) { return r.id == id; });
}

class StateManager;

class StateManagerInterface {
private:
  static state::StateManager* stateManager;

protected:
  static state::StateManager* getStateManager(bool throwIfNotSet = false);
  bool hasStateManager() const { return stateManager != nullptr; }

public:
  static void setStateManager(state::StateManager* _stateManager);
};

class AbstractAction : public state::DatabaseInterface,
                       public state::LayerManagerInterface,
                       public state::StateManagerInterface {
protected:
  State* state = nullptr;

  virtual void act() {
    /* noop */
  };

public:
  virtual bmin::String getName() const {
#ifdef __GNUG__
    int status;
    char* realname = abi::__cxa_demangle(typeid(*this).name(), 0, 0, &status);
    bmin::String name = (status == 0) ? realname : typeid(*this).name();
    free(realname);
    return name;
#else
    return typeid(*this).name();
#endif
  }

  void setState(State* state) { this->state = state; }

  void execute(State* state) {
    this->state = state;
    act();
  }

  void insertAction(AbstractAction* action, int ms = 0);
  void enqueueAction(AbstractAction* action, int ms = 0);

  virtual ~AbstractAction() = default;
};

namespace actions {

// UiManager constructs this action from inside carcer.state. Keeping the
// small state-only action here avoids a state -> actions -> state module cycle
// while preserving the ActionBus notification consumed by the UI.
class UiRemoveFloatingNotification : public AbstractAction {
  bmin::String notificationId;

  void act() override {
    state->uiState.floatingNotifications.eraseIf(
        [&](const UiFloatingNotification& notification) {
          return notification.id == notificationId;
        });
  }

public:
  explicit UiRemoveFloatingNotification(bmin::String id)
      : notificationId(std::move(id)) {}
};

} // namespace actions

class ActionBus {
  struct Entry {
    void* owner = nullptr;
    std::type_index actionType{typeid(void)};
    std::function<void(AbstractAction&, State&)> handler;
  };

  bmin::DynArray<Entry> entries;

public:
  void subscribe(void* owner,
                 std::type_index actionType,
                 std::function<void(AbstractAction&, State&)> handler);

  void unsubscribe(void* owner);
  void notify(AbstractAction& action, State& state);
};

class UiManager {
public:
  void update(int dt, State& state, StateManager& stateManager);
};

struct AsyncAction {
  bmin::UniquePtr<state::AbstractAction> action;
  model::TimerStruct timer;
};

struct ActionData {
  bmin::List<bmin::UniquePtr<AsyncAction>> sequentialActions;
  bmin::List<bmin::UniquePtr<AsyncAction>> sequentialActionsNext;
  bmin::List<bmin::UniquePtr<AsyncAction>> insertActions;
  bmin::DynArray<bmin::UniquePtr<AsyncAction>> parallelActions;
};

class StateManager : public state::DatabaseInterface {
private:
  state::State state;
  ActionData actionData;
  ActionBus actionBus;
  UiManager uiManager;

public:
  StateManager();
  ~StateManager() = default;

  state::State& getState();
  ActionData& getActionData();
  ActionBus& getActionBus();
  const ActionBus& getActionBus() const;

  void enqueueAction(ActionData& actions, AbstractAction* action, int ms);
  void insertAction(ActionData& actions, AbstractAction* action, int ms);
  void pllAction(ActionData& actions, AbstractAction* action, int ms);
  void moveSequentialActions(ActionData& actions);
  void moveInsertActions(ActionData& actions);

  void update(int dt);
};

inline void AbstractAction::insertAction(AbstractAction* action, int ms) {
  auto* sm = getStateManager();
  if (sm == nullptr) {
    return;
  }
  sm->insertAction(sm->getActionData(), action, ms);
}

inline void AbstractAction::enqueueAction(AbstractAction* action, int ms) {
  auto* sm = getStateManager();
  if (sm == nullptr) {
    return;
  }
  sm->enqueueAction(sm->getActionData(), action, ms);
}

} // namespace state

// --- from state/WorldActions.h ---
namespace state {

// World action types enum
enum class WorldActionType {
  NONE,
  JUMP,
  ABILITY,
  STATUS,
  TALK,
  END_FIGHT,
  GET,
  MAP,
  MAP_OUTDOOR,
  SNEAK,
  START_FIGHT,
  UNLOCK,
  EXAMINE,
  INVENTORY,
  SHOOT,
  DEFEND,
  INTERACT,
  REST,
  JOURNAL
};

struct WorldActionUiState {
  const std::array<WorldActionType, 13> townModeActionTypes = {
      WorldActionType::START_FIGHT,
      WorldActionType::EXAMINE,
      WorldActionType::TALK,
      WorldActionType::ABILITY,
      WorldActionType::SNEAK,
      WorldActionType::JUMP,
      WorldActionType::SHOOT,
      WorldActionType::INTERACT,
      WorldActionType::GET,
      WorldActionType::JOURNAL,
      WorldActionType::INVENTORY,
      WorldActionType::STATUS,
      WorldActionType::MAP,
  };
  const std::array<WorldActionType, 9> townModeFightActionTypes = {
      WorldActionType::END_FIGHT,
      WorldActionType::EXAMINE,
      WorldActionType::ABILITY,
      WorldActionType::JUMP,
      WorldActionType::SHOOT,
      WorldActionType::DEFEND,
      WorldActionType::GET,
      WorldActionType::INVENTORY,
      WorldActionType::STATUS,
  };
  const std::array<WorldActionType, 7> outdoorModeActionTypes = {
      WorldActionType::EXAMINE,
      WorldActionType::ABILITY,
      WorldActionType::MAP_OUTDOOR,
      WorldActionType::REST,
      WorldActionType::INVENTORY,
      WorldActionType::STATUS,
      WorldActionType::JOURNAL,
  };
};

} // namespace state

} // export
