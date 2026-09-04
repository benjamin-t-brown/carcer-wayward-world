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
export import carcer.model.instances;
export import carcer.model.templates;
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
  case LayerId::SpecialEvent:
    return "layer_special_event";
  }
  return {};
}

struct LayerManagerOps {
  void (*closeById)(void* layerManager, LayerId id) = nullptr;
  void (*closeByString)(void* layerManager, std::string_view id) = nullptr;
  void (*showInventory)(void* layerManager, void* window) = nullptr;
  void (*showMagic)(void* layerManager, void* window) = nullptr;
  void (*showSpellCast)(void* layerManager, void* window, const bmin::String* chId) = nullptr;
  void (*showSpellInfo)(void* layerManager,
                        void* window,
                        const bmin::String* spellName) = nullptr;
  void (*showEquipRunes)(void* layerManager,
                         void* window,
                         const bmin::String* characterPlayerId) = nullptr;
  void (*showPickUp)(void* layerManager,
                     void* window,
                     const int* containerX,
                     const int* containerY) = nullptr;
  void (*showPickupContext)(void* layerManager,
                            void* window,
                            const model::ItemInstance* item) = nullptr;
  void (*showInventoryContext)(void* layerManager,
                               void* window,
                               const bmin::String* itemId,
                               const bmin::String* itemName) = nullptr;
  void (*showDropConfirm)(void* layerManager,
                          void* window,
                          const bmin::String* characterPlayerId,
                          const bmin::String* itemId) = nullptr;
  void (*showGiveContext)(void* layerManager,
                          void* window,
                          const bmin::String* fromCharacterPlayerId,
                          const bmin::String* itemId) = nullptr;
  void (*showPopupText)(void* layerManager,
                        void* window,
                        const bmin::String* title,
                        const bmin::String* text) = nullptr;
  void (*showSpecialEvent)(void* layerManager,
                           void* window,
                           const bmin::String* eventId,
                           State* state) = nullptr;
  void (*cancelEquipRunes)(void* layerManager) = nullptr;
};

class LayerManagerInterface {
private:
  static void* layerManager;
  static LayerManagerOps ops;

public:
  virtual ~LayerManagerInterface() = default;

  static void setLayerManager(void* _layerManager);
  static void* getLayerManager();
  static void bindLayerOps(LayerManagerOps _ops);

  static void closeLayer(LayerId id);
  static void closeLayer(std::string_view id);
  static void showInventory(void* window);
  static void showMagic(void* window);
  static void showSpellCast(void* window, const bmin::String& chId);
  static void showSpellInfo(void* window, const bmin::String& spellName);
  static void showEquipRunes(void* window, const bmin::String& characterPlayerId);
  static void showPickUp(void* window);
  static void showPickUp(void* window, int containerX, int containerY);
  static void showPickupContext(void* window, const model::ItemInstance& item);
  static void showInventoryContext(void* window,
                                   const bmin::String& itemId,
                                   const bmin::String& itemName);
  static void showDropConfirm(void* window,
                              const bmin::String& characterPlayerId,
                              const bmin::String& itemId);
  static void showGiveContext(void* window,
                              const bmin::String& fromCharacterPlayerId,
                              const bmin::String& itemId);
  static void showPopupText(void* window,
                            const bmin::String& title,
                            const bmin::String& text);
  static void showSpecialEvent(void* window, const bmin::String& eventId, State& state);
  static void cancelEquipRunes();
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

namespace state {

void worldUpdate(sdl2w::Window* window, StateManager& stateManager, int dt);
void worldProcessPendingTriggers(sdl2w::Window* window, StateManager& stateManager);

} // namespace state

} // export
