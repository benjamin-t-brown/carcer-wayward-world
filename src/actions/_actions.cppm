module;
#include <cstddef>

export module carcer.actions;

export import carcer.state;
import sdl2w;

export namespace state::actions {

/** Owning handle for one private action implementation. */
class Command {
  AbstractAction* action = nullptr;

public:
  explicit Command(AbstractAction* action);
  Command(Command&& other) noexcept;
  Command& operator=(Command&& other) noexcept;
  Command(const Command&) = delete;
  Command& operator=(const Command&) = delete;
  ~Command();

  void execute(State* state);
  AbstractAction* release();
  operator AbstractAction*() &&;
};

struct CombatActionContext {
  bmin::String targetChId;
  bmin::String abilityId;
  model::TileXY targetLoc{};
};

struct WorldSetActionModeCtx {
  bmin::String spellId;
  bmin::String chId;
};

Command doCPUCombatTurn();
Command doCombatAction(const bmin::String& characterId,
                       model::CombatActionType actionType,
                       const CombatActionContext& context = {});
Command endCombat();
Command goNextCombatTurn();
Command modifyAP(bmin::String characterId, int delta);
Command modifyHP(bmin::String characterId, int delta);
Command modifyPartyMemberHp(bmin::String characterId, int delta);
Command playSound(bmin::String soundName);
Command setActiveCombatCharacter(bmin::String characterId = {});
Command startCombat();

Command adjustEquippedRune(const bmin::String& characterId,
                           model::RuneType runeType,
                           int delta);
Command cancelEquipRunes();
Command commitEquipRunes();
Command continueSpecialEvent();
Command dropInventoryItem(bmin::String characterId, bmin::String itemId);
Command giveInventoryItem(bmin::String fromCharacterId,
                          bmin::String toCharacterId,
                          bmin::String itemId,
                          int quantity);
Command pickUpItem(bmin::String itemId);
Command pushFloatingNotification(bmin::String message, UiFloatingNotificationType type);
Command removeFloatingNotification(bmin::String notificationId);
Command removeLayer(const bmin::String& layerId);
Command removeLayer(LayerId layerId);
Command reorderInventoryItem(const bmin::String& characterId,
                             int inventoryIndex,
                             int direction);
Command selectSpecialEventChoice(int choiceIndex);
Command selectSpellCast(const bmin::String& spellId, const bmin::String& characterId);
Command setCurrentPartyMember(int index);
Command setCurrentPartyMemberInventory(int index);
Command setCurrentPartyMemberMagic(int index);
Command setSelectedPartyMemberId(bmin::String characterId);
Command setSpellReady(const bmin::String& characterId,
                      const bmin::String& spellName,
                      bool ready);
Command showLayerDropContext(sdl2w::Window* window,
                             bmin::String characterId,
                             bmin::String itemId);
Command showLayerEquipRunes(sdl2w::Window* window, const bmin::String& characterId);
Command showLayerGiveContext(sdl2w::Window* window,
                             bmin::String characterId,
                             bmin::String itemId);
Command showLayerInventory(sdl2w::Window* window);
Command showLayerInventoryContext(sdl2w::Window* window,
                                  bmin::String itemName,
                                  bmin::String itemId);
Command showLayerMagic(sdl2w::Window* window);
Command showLayerPickUp(sdl2w::Window* window);
Command showLayerPickUp(sdl2w::Window* window, int containerX, int containerY);
Command showLayerPickupContext(sdl2w::Window* window, const model::ItemInstance& item);
Command showLayerPopupText(sdl2w::Window* window, bmin::String title, bmin::String text);
Command showLayerSpecialEvent(sdl2w::Window* window, bmin::String eventId);
Command showLayerSpellCast(sdl2w::Window* window, const bmin::String& characterId);
Command showLayerSpellInfo(sdl2w::Window* window, const bmin::String& spellName);
Command toggleEquipInventoryItem(const bmin::String& characterId,
                                 const bmin::String& itemId);
Command toggleManaSlotRune(const bmin::String& characterId, size_t slotIndex);
Command updateHeldMove(HeldMove heldMove);

Command examineAt(int x, int y);
Command examineAt(sdl2w::Window* window, int x, int y);
Command interactAt();
Command loadActiveMap(const bmin::String& gridId);
Command moveActionAim(int dx, int dy);
Command movePlayer(int dx, int dy);
Command setActionAim(int x, int y);
Command setActionMode(model::WorldActionMode mode,
                      const WorldSetActionModeCtx& context = {});
Command spawnPlayer(const bmin::String& mapName, const bmin::String& markerName);
Command spawnPlayerAtMarker(bmin::String markerName = "MarkerPlayer");
Command spawnPlayerAtXY(int x, int y);
Command talkAt(int x, int y);
Command travel(model::TravelTrigger trigger);

} // namespace state::actions

export namespace state {

void worldUpdate(sdl2w::Window* window, StateManager& stateManager, int dt);
inline void worldUpdate(StateManager& stateManager, int dt) {
  worldUpdate(nullptr, stateManager, dt);
}
void worldProcessPendingTriggers(sdl2w::Window* window, StateManager& stateManager);

} // namespace state
