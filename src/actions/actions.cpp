module;
#include "macros.h"
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <optional>
#include <utility>

module carcer.actions;

import bmin.string_interop;
import carcer.data;
import carcer.game.combat;
import carcer.game.inventory;
import carcer.game.map;
import carcer.lib.StringUtil;
import carcer.model;
import carcer.state;
import sdl2w;

// --- actions/combat/EndCombat.cppm ---

namespace state {

namespace actions {

class EndCombat : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::EndCombat; }
  void act() override {
    if (!state) {
      return;
    }

    auto& world = state->world;
    LOG(INFO) << "EndCombat: ending combat, returning to town mode" << LOG_ENDL;
    world.combat.active = false;
    world.combat.turnOrderIds.clear();
    world.combat.activeTurnIndex = 0;
    world.combat.activeCharacterId = bmin::String{};
    world.combat.isWaitingForAction = false;
    state->turnMode = model::TurnMode::TURN_TOWN;

    model::removeExtraPartyMembersFromMap(world, state->player);

    if (state->player.party.empty()) {
      return;
    }

    const auto& leader = state->player.party[0];
    world.camera.cameraFollowCharacterId = leader.instanceId;
    world.camera.cameraMode = model::CameraMode::Follow;

    if (auto* avatar = game::findPartyAvatarOnActiveMap(world.activeMap, state->player)) {
      auto* database = getDatabase();
      if (database != nullptr) {
        game::updateActiveMapVisibilityFromPlayer(
            world, state->mapInstances, avatar->x, avatar->y, *database);
      }
      if (world.camera.viewW > 0 && world.camera.viewH > 0) {
        const auto cam = game::computeCameraFollow(
            avatar->x, avatar->y, world.camera.viewW, world.camera.viewH);
        world.camera.camX = cam.camX;
        world.camera.camY = cam.camY;
      }
    }
  }

public:
  EndCombat() = default;
};

} // namespace actions

} // namespace state

// --- actions/combat/ModifyAP.cppm ---

namespace state {

namespace actions {

class ModifyAP : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::ModifyAP; }
  bmin::String characterId;
  int delta = 0;

  void act() override {
    if (!state) {
      return;
    }
    game::ActiveMapOrchestrator orch(
        state->world.activeMap, state->mapInstances, getDatabase());
    auto* character = orch.findCharacterById(characterId);
    if (character == nullptr) {
      return;
    }
    character->currentAp += delta;
  }

public:
  ModifyAP(bmin::String _characterId, int _delta)
      : characterId(std::move(_characterId)), delta(_delta) {}
};

} // namespace actions

} // namespace state
// --- actions/combat/ModifyHP.cppm ---

namespace state {

namespace actions {

class ModifyHP : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::ModifyHP; }
  bmin::String characterId;
  int delta = 0;

  void act() override {
    if (!state) {
      return;
    }
    game::ActiveMapOrchestrator orch(
        state->world.activeMap, state->mapInstances, getDatabase());
    auto* character = orch.findCharacterById(characterId);
    if (character == nullptr) {
      return;
    }
    const auto hp = model::getCharacterHp(state->player, *character) + delta;
    model::setCharacterHp(state->player, *character, hp);
  }

public:
  ModifyHP(bmin::String _characterId, int _delta)
      : characterId(std::move(_characterId)), delta(_delta) {}
};

} // namespace actions

} // namespace state
// --- actions/combat/MoveCharacter.cppm ---

namespace state {

namespace actions {

class MoveCharacter : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::MoveCharacter; }
  bmin::String characterId;
  int dx = 0;
  int dy = 0;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch(world.activeMap, state->mapInstances, database);
    orch.fetchMapGrid(world.activeMap.gridId);
    auto* character = orch.findCharacterById(characterId);
    if (character == nullptr) {
      return;
    }

    const auto destX = character->x + dx;
    const auto destY = character->y + dy;
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || destX < 0 || destY < 0 || destX >= total.x || destY >= total.y) {
      return;
    }
    auto* destMap = orch.getMapInstanceAt(destX, destY);
    const auto destLocal = orch.activeMapCoordToInstanceCoord(destX, destY);
    if (!destMap || !destLocal.valid) {
      return;
    }
    destMap->tileLayerNumber = world.activeMap.mapLayer;
    if (!game::isDestinationWalkable(*destMap, destLocal.x, destLocal.y, *database)) {
      return;
    }
    if (orch.findCharacterAt(destX, destY, characterId) != nullptr) {
      return;
    }

    character->x = destX;
    character->y = destY;
    model::updateCharacterFacingFromMove(*character, dx, dy);

    if (model::isPartyMember(state->player, character->id)) {
      game::updateActiveMapVisibilityFromParty(
          world, state->mapInstances, state->player, *database);
    }
  }

public:
  MoveCharacter(bmin::String _characterId, int _dx, int _dy)
      : characterId(std::move(_characterId)), dx(_dx), dy(_dy) {}
};

} // namespace actions

} // namespace state
// --- actions/combat/PerformMeleeAttack.cppm ---

namespace state {

namespace actions {

class PerformMeleeAttack : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::PerformMeleeAttack; }
  bmin::String attackerId;
  bmin::String victimId;

  // Deferred visual effects are private actions in this module.
  void act() override;

public:
  PerformMeleeAttack(bmin::String _attackerId, bmin::String _victimId)
      : attackerId(std::move(_attackerId)), victimId(std::move(_victimId)) {}
};

} // namespace actions

} // namespace state
// --- actions/combat/PerformSpellCast.cppm ---

namespace state {

namespace actions {

class PerformSpellCast : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::PerformSpellCast; }
  bmin::String casterId;
  bmin::String spellId;
  model::SpellTargetInfo spellTargetInfo;

  // Deferred visual effects are private actions in this module
  // and action-mode changes.
  void doZoneSpell(const model::AbilityTemplate& ability,
                   model::CharacterInstance& caster,
                   game::ActiveMapOrchestrator& orch);

  bool verifySpellCanBeCast(const model::CharacterInstance& caster,
                            const model::AbilityTemplate& ability) {
    if (ability.costType == model::AbilityCostType::ABILITY_COST_MANA) {
      if (caster.currentMp < ability.costValue) {
        return false;
      }
    }

    return true;
  }

  void act() override;

public:
  explicit PerformSpellCast(const bmin::String& casterId,
                            const bmin::String& spellId,
                            const model::SpellTargetInfo& spellTargetInfo)
      : casterId(casterId), spellId(spellId), spellTargetInfo(spellTargetInfo) {}
};

} // namespace actions

} // namespace state
// --- actions/combat/SetActiveCombatCharacter.cppm ---

namespace state {

namespace actions {

class SetActiveCombatCharacter : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::SetActiveCombatCharacter; }
  bmin::String characterId;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    auto& combat = world.combat;
    if (!combat.active) {
      return;
    }

    if (characterId.empty()) {
      if (combat.activeTurnIndex < 0 ||
          combat.activeTurnIndex >= static_cast<int>(combat.turnOrderIds.size())) {
        return;
      }
      characterId = combat.turnOrderIds[static_cast<size_t>(combat.activeTurnIndex)];
    }

    game::ActiveMapOrchestrator orch(
        state->world.activeMap, state->mapInstances, database);
    auto* character = orch.findCharacterById(characterId);
    if (character == nullptr) {
      return;
    }

    combat.activeCharacterId = characterId;
    combat.isWaitingForAction = true;

    if (model::isPartyMember(state->player, characterId)) {
      // Highlight the acting party member in the HUD only.
      state->uiState.selectedPartyMemberId = characterId;
    }

    world.camera.cameraFollowCharacterId = characterId;
    world.camera.cameraMode = model::CameraMode::Follow;
    if (world.camera.viewW > 0 && world.camera.viewH > 0) {
      const auto cam = game::computeCameraFollow(
          character->x, character->y, world.camera.viewW, world.camera.viewH);
      world.camera.camX = cam.camX;
      world.camera.camY = cam.camY;
    }
  }

public:
  explicit SetActiveCombatCharacter(bmin::String _characterId = bmin::String{})
      : characterId(std::move(_characterId)) {}
};

} // namespace actions

} // namespace state
// --- actions/general/_general.cppm ---

namespace state {

namespace actions {

class PlaySound : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::PlaySound; }
  bmin::String soundName;

  void act() override {
    if (!state) {
      return;
    }
    if (soundName.empty()) {
      return;
    }
    LOG(DEBUG) << "PlaySound: " << soundName << LOG_ENDL;
    if (state->soundsToPlay.contains(soundName)) {
      return;
    }
    state->soundsToPlay.pushBack(soundName);
  }

public:
  explicit PlaySound(bmin::String _soundName) : soundName(std::move(_soundName)) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/layers/UiCancelEquipRunes.cppm ---

namespace state {

namespace actions {

/** Close the Equip Runes layer without committing edits.
    TODO(reconciler): snapshot restore belongs to the Layer itself (it can
    revert on close) once LayerManager::update() reconciles layerStack --
    see LayerRequest in carcer.state. */
class UiCancelEquipRunes : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiCancelEquipRunes; }
  void act() override {
    if (!state) {
      return;
    }
    removeLayerRequest(*state, LayerId::EquipRunes);
  }
};

} // namespace actions

} // namespace state
// --- actions/ui/layers/UiCommitEquipRunes.cppm ---

namespace state {

namespace actions {

/** Keep live equipped-rune edits and close the Equip Runes layer. */
class UiCommitEquipRunes : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiCommitEquipRunes; }
  void act() override {
    if (!state) {
      return;
    }
    removeLayerRequest(*state, LayerId::EquipRunes);
  }
};

} // namespace actions

} // namespace state
// --- actions/ui/layers/UiRemoveLayer.cppm ---

namespace state {

namespace actions {

class UiRemoveLayer : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiRemoveLayer; }
  bmin::String layerId;
  void act() override {
    if (!state) {
      return;
    }
    if (auto id = layerIdFromString(bmin::toStringView(layerId))) {
      removeLayerRequest(*state, *id);
    }
  }

public:
  UiRemoveLayer(const bmin::String& _layerId) : layerId(_layerId) {}
  explicit UiRemoveLayer(LayerId id)
      : layerId(strutil::fromStringView(layerIdString(id))) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/layers/UiShowLayerDropContext.cppm ---

namespace state {

namespace actions {

class UiShowLayerDropContext : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerDropContext; }
  bmin::String characterPlayerId;
  bmin::String itemId;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(
        *state,
        LayerRequest{.id = LayerId::DropConfirm, .a = characterPlayerId, .b = itemId});
  }

public:
  UiShowLayerDropContext(sdl2w::Window* /*_window*/,
                         bmin::String _characterPlayerId,
                         bmin::String _itemId)
      : characterPlayerId(std::move(_characterPlayerId)), itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/layers/UiShowLayerEquipRunes.cppm ---

namespace state {

namespace actions {

class UiShowLayerEquipRunes : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerEquipRunes; }
  bmin::String characterPlayerId;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::EquipRunes, .a = characterPlayerId});
  }

public:
  UiShowLayerEquipRunes(sdl2w::Window* /*_window*/,
                        const bmin::String& _characterPlayerId)
      : characterPlayerId(_characterPlayerId) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/layers/UiShowLayerGiveContext.cppm ---

namespace state {

namespace actions {

class UiShowLayerGiveContext : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerGiveContext; }
  bmin::String fromCharacterPlayerId;
  bmin::String itemId;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::GiveContext,
                                  .a = fromCharacterPlayerId,
                                  .b = itemId});
  }

public:
  UiShowLayerGiveContext(sdl2w::Window* /*_window*/,
                         bmin::String _fromCharacterPlayerId,
                         bmin::String _itemId)
      : fromCharacterPlayerId(std::move(_fromCharacterPlayerId)),
        itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/layers/UiShowLayerInventory.cppm ---

namespace state {

namespace actions {

class UiShowLayerInventory : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerInventory; }
  void act() override {
    if (!state) {
      return;
    }
    auto& player = state->player;
    const int selectedIndex = model::playerFindPartyMemberIndexById(
        player, state->uiState.selectedPartyMemberId);
    player.currentPartyMemberInventoryIndex = selectedIndex >= 0 ? selectedIndex : 0;
    pushLayerRequest(*state, LayerRequest{.id = LayerId::Inventory});
  }

public:
  explicit UiShowLayerInventory(sdl2w::Window* /*_window*/) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/layers/UiShowLayerInventoryContext.cppm ---

namespace state {

namespace actions {

class UiShowLayerInventoryContext : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::UiShowLayerInventoryContext;
  }
  bmin::String itemName;
  bmin::String itemId;
  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(
        *state,
        LayerRequest{.id = LayerId::InventoryContext, .a = itemId, .b = itemName});
  }

public:
  UiShowLayerInventoryContext(sdl2w::Window* /*_window*/,
                              bmin::String itemName,
                              bmin::String itemId)
      : itemName(itemName), itemId(itemId) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/layers/UiShowLayerMagic.cppm ---

namespace state {

namespace actions {

class UiShowLayerMagic : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerMagic; }
  void act() override {
    if (!state) {
      return;
    }
    auto& player = state->player;
    const int selectedIndex = model::playerFindPartyMemberIndexById(
        player, state->uiState.selectedPartyMemberId);
    player.currentPartyMemberMagicIndex = selectedIndex >= 0 ? selectedIndex : 0;
    pushLayerRequest(*state, LayerRequest{.id = LayerId::Magic});
  }

public:
  explicit UiShowLayerMagic(sdl2w::Window* /*_window*/) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/layers/UiShowLayerPickUp.cppm ---

namespace state {

namespace actions {

class UiShowLayerPickUp : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerPickUp; }
  std::optional<std::pair<int, int>> containerTile;

  void act() override {
    if (!state) {
      return;
    }
    LayerRequest request{.id = LayerId::PickUp};
    if (containerTile) {
      request.x = containerTile->first;
      request.y = containerTile->second;
    }
    pushLayerRequest(*state, std::move(request));
  }

public:
  explicit UiShowLayerPickUp(sdl2w::Window* /*_window*/) {}

  UiShowLayerPickUp(sdl2w::Window* /*_window*/, int containerX, int containerY)
      : containerTile(std::make_pair(containerX, containerY)) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/layers/UiShowLayerPickupContext.cppm ---

namespace state {

namespace actions {

class UiShowLayerPickupContext : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerPickupContext; }
  bmin::String itemId;
  void act() override {
    if (!state) {
      return;
    }
    // Reconciler re-resolves the full ItemInstance from state by id.
    pushLayerRequest(*state, LayerRequest{.id = LayerId::PickUpContext, .a = itemId});
  }

public:
  UiShowLayerPickupContext(sdl2w::Window* /*_window*/, const model::ItemInstance& item)
      : itemId(item.id) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/layers/UiShowLayerPopupText.cppm ---

namespace state {

namespace actions {

class UiShowLayerPopupText : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerPopupText; }
  bmin::String title;
  bmin::String text;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::PopupText, .a = title, .b = text});
  }

public:
  UiShowLayerPopupText(sdl2w::Window* /*_window*/,
                       bmin::String _title,
                       bmin::String _text)
      : title(std::move(_title)), text(std::move(_text)) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/layers/UiShowLayerSpecialEvent.cppm ---

namespace state {

namespace actions {

class UiShowLayerSpecialEvent : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerSpecialEvent; }
  bmin::String eventId;

  void act() override {
    if (!state) {
      return;
    }
    // Reconciler looks up the GameEvent (+ related events, storage) from
    // eventId via the database.
    pushLayerRequest(*state, LayerRequest{.id = LayerId::SpecialEvent, .a = eventId});
  }

public:
  UiShowLayerSpecialEvent(sdl2w::Window* /*_window*/, bmin::String _eventId)
      : eventId(std::move(_eventId)) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/layers/UiShowLayerSpellCast.cppm ---

namespace state {

namespace actions {

class UiShowLayerSpellCast : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerSpellCast; }
  bmin::String chId;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state, LayerRequest{.id = LayerId::SpellCast, .a = chId});
  }

public:
  explicit UiShowLayerSpellCast(sdl2w::Window* /*_window*/, const bmin::String& chId)
      : chId(chId) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/layers/UiShowLayerSpellInfo.cppm ---

namespace state {

namespace actions {

class UiShowLayerSpellInfo : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiShowLayerSpellInfo; }
  bmin::String spellName;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state, LayerRequest{.id = LayerId::SpellInfo, .a = spellName});
  }

public:
  UiShowLayerSpellInfo(sdl2w::Window* /*_window*/, const bmin::String& _spellName)
      : spellName(_spellName) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiAdjustEquippedRune.cppm ---

namespace state {

namespace actions {

/** Live-adjust equipped runes while the Equip Runes modal is open (delta +1 / -1). */
class UiAdjustEquippedRune : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiAdjustEquippedRune; }
  bmin::String characterPlayerId;
  model::RuneType runeType = model::RuneType::HEAT;
  int delta = 0;

  void act() override {
    auto* characterPlayer =
        model::playerFindPartyMemberById(state->player, characterPlayerId);
    if (!characterPlayer) {
      LOG(WARN) << "UiAdjustEquippedRune::act: character not found " << characterPlayerId
                << LOG_ENDL;
      return;
    }

    if (delta > 0) {
      model::characterPlayerEquipRuneType(*characterPlayer, runeType);
    } else if (delta < 0) {
      model::characterPlayerUnequipOneRuneOfType(*characterPlayer, runeType);
    }
  }

public:
  UiAdjustEquippedRune(const bmin::String& _characterPlayerId,
                       model::RuneType _runeType,
                       int _delta)
      : characterPlayerId(_characterPlayerId), runeType(_runeType), delta(_delta) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiContinueSpecialEvent.cppm ---

namespace state {

namespace actions {

// Broadcast-only, see UiSelectSpecialEventChoice.
class UiContinueSpecialEvent : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiContinueSpecialEvent; }
};

} // namespace actions

} // namespace state
// --- actions/ui/UiDropInventoryItem.cppm ---

namespace state {

namespace actions {

class UiDropInventoryItem : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiDropInventoryItem; }
  bmin::String characterPlayerId;
  bmin::String itemId;

  void act() override {
    auto& localState = *state;
    auto* partyMember =
        model::playerFindPartyMemberById(localState.player, characterPlayerId);
    if (!partyMember) {
      LOG(WARN) << "UiDropInventoryItem::act: party member not found" << LOG_ENDL;
      return;
    }

    bmin::String itemTemplateName;
    int quantity = 0;
    for (const auto& item : partyMember->inventory) {
      if (item.id == itemId) {
        quantity = item.quantity;
        itemTemplateName = item.itemName;
        break;
      }
    }
    if (quantity < 1 || itemTemplateName.empty()) {
      LOG(WARN) << "UiDropInventoryItem::act: item not found in inventory" << LOG_ENDL;
      return;
    }

    auto* database = getDatabase();
    if (!database) {
      LOG(WARN) << "UiDropInventoryItem::act: database is nullptr" << LOG_ENDL;
      return;
    }

    const auto* dropCharacter = game::findDropCharacterOnActiveMap(
        localState.world.activeMap, localState.player, characterPlayerId);
    if (!dropCharacter) {
      LOG(WARN) << "UiDropInventoryItem::act: no character on map to drop at" << LOG_ENDL;
      return;
    }

    if (model::characterPlayerIsItemEquippedById(*partyMember, itemId)) {
      game::toggleEquippedInventoryItem(*partyMember, itemId, *database);
    }

    model::ItemInstance dropped;
    dropped.id = itemId;
    dropped.itemTemplateName = itemTemplateName;
    dropped.quantity = quantity;
    dropped.x = dropCharacter->x;
    dropped.y = dropCharacter->y;
    localState.world.activeMap.items.pushBack(std::move(dropped));

    model::characterPlayerRemoveItemFromInventoryById(*partyMember, itemId, quantity);

    removeLayerRequest(localState, LayerId::DropConfirm);
    removeLayerRequest(localState, LayerId::InventoryContext);
  }

public:
  UiDropInventoryItem(bmin::String _characterPlayerId, bmin::String _itemId)
      : characterPlayerId(std::move(_characterPlayerId)), itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiGiveInventoryItem.cppm ---

namespace state {

namespace actions {

class UiGiveInventoryItem : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiGiveInventoryItem; }
  bmin::String fromCharacterPlayerId;
  bmin::String toCharacterPlayerId;
  bmin::String itemId;
  int quantity;

  void act() override {
    auto& localState = *state;
    auto* fromMember =
        model::playerFindPartyMemberById(localState.player, fromCharacterPlayerId);
    auto* toMember =
        model::playerFindPartyMemberById(localState.player, toCharacterPlayerId);
    if (!fromMember || !toMember) {
      LOG(WARN) << "UiGiveInventoryItem::act: party member not found" << LOG_ENDL;
      return;
    }

    auto* database = getDatabase();
    if (!database) {
      LOG(WARN) << "UiGiveInventoryItem::act: database is nullptr" << LOG_ENDL;
      return;
    }

    const auto result =
        game::giveInventoryItem(*fromMember, *toMember, itemId, quantity, *database);

    switch (result) {
    case model::GiveItemResult::TOO_HEAVY: {
      UiFloatingNotification notification;
      notification.id = model::createRandomId();
      notification.message = TRANSLATE("Too heavy!");
      notification.type = UiFloatingNotificationType::WARNING;
      model::timerStructStart(notification.timer,
                              state->settings.floatingNotificationDurationMs);
      localState.uiState.floatingNotifications.pushBack(std::move(notification));

      removeLayerRequest(localState, LayerId::GiveContext);
      break;
    }
    case model::GiveItemResult::SUCCESS: {
      removeLayerRequest(localState, LayerId::GiveContext);
      removeLayerRequest(localState, LayerId::InventoryContext);
      break;
    }
    default:
      LOG(WARN) << "UiGiveInventoryItem::act: give failed" << LOG_ENDL;
      break;
    }
  }

public:
  UiGiveInventoryItem(bmin::String _fromCharacterPlayerId,
                      bmin::String _toCharacterPlayerId,
                      bmin::String _itemId,
                      int _quantity)
      : fromCharacterPlayerId(std::move(_fromCharacterPlayerId)),
        toCharacterPlayerId(std::move(_toCharacterPlayerId)),
        itemId(std::move(_itemId)),
        quantity(_quantity) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiPickUpItem.cppm ---

namespace state {

namespace actions {

class UiPickUpItem : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiPickUpItem; }
  bmin::String itemId;

  void act() override {
    if (!state) {
      return;
    }
    auto& localState = *state;
    auto* database = getDatabase();
    if (!database) {
      LOG(WARN) << "UiPickUpItem::act: database is nullptr" << LOG_ENDL;
      return;
    }

    auto* partyMember = model::playerFindPartyMemberByIndex(
        localState.player, localState.player.currentPartyMemberIndex);
    if (!partyMember) {
      LOG(WARN) << "UiPickUpItem::act: party member not found" << LOG_ENDL;
      return;
    }

    model::ItemInstance* mapItem = nullptr;
    size_t mapItemIndex = 0;
    for (size_t i = 0; i < localState.world.activeMap.items.size(); i++) {
      if (localState.world.activeMap.items[i].id == itemId) {
        mapItem = &localState.world.activeMap.items[i];
        mapItemIndex = i;
        break;
      }
    }
    if (!mapItem) {
      LOG(WARN) << "UiPickUpItem::act: item not found on map" << LOG_ENDL;
      return;
    }

    const model::ItemTemplate* itemTemplate = nullptr;
    try {
      itemTemplate =
          &database->getItemTemplate(bmin::toStringView(mapItem->itemTemplateName));
    } catch (...) {
      LOG(WARN) << "UiPickUpItem::act: item template not found "
                << mapItem->itemTemplateName << LOG_ENDL;
      return;
    }

    const int addedWeight = mapItem->quantity * itemTemplate->weight;
    if (game::inventoryWeight(*partyMember, *database) + addedWeight >
        model::characterGetWeightCapacity(*partyMember)) {
      UiFloatingNotification notification;
      notification.id = model::createRandomId();
      notification.message = TRANSLATE("Too heavy!");
      notification.type = UiFloatingNotificationType::WARNING;
      model::timerStructStart(notification.timer,
                              localState.settings.floatingNotificationDurationMs);
      localState.uiState.floatingNotifications.pushBack(std::move(notification));
      return;
    }

    model::characterPlayerAddItemToInventory(
        *partyMember, *itemTemplate, mapItem->quantity);
    localState.world.activeMap.items.erase(mapItemIndex);
  }

public:
  explicit UiPickUpItem(bmin::String _itemId) : itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiPushFloatingNotification.cppm ---

namespace state {

namespace actions {

class UiPushFloatingNotification : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::UiPushFloatingNotification;
  }
  bmin::String message;
  UiFloatingNotificationType type;

  void act() override {
    auto& localState = *state;
    UiFloatingNotification notification;
    notification.id = model::createRandomId();
    notification.message = message;
    notification.type = type;
    model::timerStructStart(notification.timer,
                            state->settings.floatingNotificationDurationMs);
    localState.uiState.floatingNotifications.pushBack(std::move(notification));
    ++localState.uiState.floatingNotificationRevision;
  }

public:
  UiPushFloatingNotification(bmin::String _message, UiFloatingNotificationType _type)
      : message(std::move(_message)), type(_type) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiRemoveFloatingNotification.cppm ---

namespace state::actions {

class UiRemoveFloatingNotification : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::UiRemoveFloatingNotification;
  }
  bmin::String notificationId;

  void act() override {
    const auto oldSize = state->uiState.floatingNotifications.size();
    state->uiState.floatingNotifications.eraseIf(
        [&](const UiFloatingNotification& notification) {
          return notification.id == notificationId;
        });
    if (state->uiState.floatingNotifications.size() != oldSize) {
      ++state->uiState.floatingNotificationRevision;
    }
  }

public:
  explicit UiRemoveFloatingNotification(bmin::String id)
      : notificationId(std::move(id)) {}
};

} // namespace state::actions
// --- actions/ui/UiReorderInventoryItem.cppm ---

namespace state {

namespace actions {

class UiReorderInventoryItem : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiReorderInventoryItem; }
  bmin::String characterPlayerId;
  int inventoryIndex = 0;
  int direction = 0;

  void act() override {
    auto& localState = *state;
    auto* characterPlayer =
        model::playerFindPartyMemberById(localState.player, characterPlayerId);
    if (!characterPlayer) {
      LOG(WARN) << "UiReorderInventoryItem::act: character not found "
                << characterPlayerId << LOG_ENDL;
      return;
    }
    if (!model::characterPlayerReorderInventoryItem(
            *characterPlayer, static_cast<size_t>(inventoryIndex), direction)) {
      LOG(WARN) << "UiReorderInventoryItem::act: reorder failed index=" << inventoryIndex
                << " direction=" << direction << LOG_ENDL;
    }
  }

public:
  UiReorderInventoryItem(const bmin::String& _characterPlayerId,
                         int _inventoryIndex,
                         int _direction)
      : characterPlayerId(_characterPlayerId),
        inventoryIndex(_inventoryIndex),
        direction(_direction) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiSelectSpecialEventChoice.cppm ---

namespace state {

namespace actions {

// Broadcast-only: carries the payload for LayerSpecialEvent's own
// subscribeAction<> to react to. UI never mutates a special event's talk
// history / runner state directly.
class UiSelectSpecialEventChoice : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::UiSelectSpecialEventChoice;
  }
  int getEventValue() const override { return choiceIndex; }

public:
  int choiceIndex = -1;
  explicit UiSelectSpecialEventChoice(int _choiceIndex) : choiceIndex(_choiceIndex) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiSetCurrentPartyMember.cppm ---

namespace state {

namespace actions {

class UiSetCurrentPartyMember : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiSetCurrentPartyMember; }
  int partyMemberIndex = 0;
  void act() override {
    auto& localState = *state;
    auto& player = localState.player;

    int nextPartyMemberIndex = partyMemberIndex;
    if (partyMemberIndex < 0 ||
        partyMemberIndex >= static_cast<int>(player.party.size())) {
      nextPartyMemberIndex = 0;
      LOG(WARN) << "UiSetCurrentPartyMember::act: partyMemberIndex is out of range " +
                       bmin::toString(partyMemberIndex) + " " +
                       bmin::toString(player.party.size())
                << LOG_ENDL;
    }
    player.currentPartyMemberIndex = nextPartyMemberIndex;
  }

public:
  UiSetCurrentPartyMember(int _partyMemberIndex) : partyMemberIndex(_partyMemberIndex) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiSetCurrentPartyMemberInventory.cppm ---

namespace state {

namespace actions {

class UiSetCurrentPartyMemberInventory : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::UiSetCurrentPartyMemberInventory;
  }
  int partyMemberInventoryIndex = 0;

  void act() override {
    auto& localState = *state;

    int nextIndex = partyMemberInventoryIndex;
    if (partyMemberInventoryIndex < 0 ||
        partyMemberInventoryIndex >= static_cast<int>(localState.player.party.size())) {
      nextIndex = 0;
      LOG(WARN) << "UiSetCurrentPartyMemberInventory::act: index out of range "
                << partyMemberInventoryIndex << " " << localState.player.party.size()
                << LOG_ENDL;
    }
    localState.player.currentPartyMemberInventoryIndex = nextIndex;
  }

public:
  explicit UiSetCurrentPartyMemberInventory(int _partyMemberInventoryIndex)
      : partyMemberInventoryIndex(_partyMemberInventoryIndex) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiSetCurrentPartyMemberMagic.cppm ---

namespace state {

namespace actions {

class UiSetCurrentPartyMemberMagic : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::UiSetCurrentPartyMemberMagic;
  }
  int partyMemberMagicIndex = 0;

  void act() override {
    auto& localState = *state;

    int nextIndex = partyMemberMagicIndex;
    if (partyMemberMagicIndex < 0 ||
        partyMemberMagicIndex >= static_cast<int>(localState.player.party.size())) {
      nextIndex = 0;
      LOG(WARN) << "UiSetCurrentPartyMemberMagic::act: index out of range "
                << partyMemberMagicIndex << " " << localState.player.party.size()
                << LOG_ENDL;
    }
    localState.player.currentPartyMemberMagicIndex = nextIndex;
  }

public:
  explicit UiSetCurrentPartyMemberMagic(int _partyMemberMagicIndex)
      : partyMemberMagicIndex(_partyMemberMagicIndex) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiSetSelectedPartyMemberId.cppm ---

namespace state {

namespace actions {

/** UI selection only — does not affect map movement / party avatar. */
class UiSetSelectedPartyMemberId : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::UiSetSelectedPartyMemberId;
  }
  bmin::String partyMemberId;

  void act() override {
    auto& localState = *state;
    if (model::playerFindPartyMemberIndexById(localState.player, partyMemberId) < 0) {
      LOG(WARN) << "UiSetSelectedPartyMemberId::act: party member id not found "
                << partyMemberId << LOG_ENDL;
      return;
    }

    localState.uiState.selectedPartyMemberId = partyMemberId;
  }

public:
  explicit UiSetSelectedPartyMemberId(bmin::String _partyMemberId)
      : partyMemberId(std::move(_partyMemberId)) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiSetSpellReady.cppm ---

namespace state {

namespace actions {

class UiSetSpellReady : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiSetSpellReady; }
  bmin::String characterPlayerId;
  bmin::String spellName;
  bool ready = true;

  void act() override {
    auto& localState = *state;
    auto* characterPlayer =
        model::playerFindPartyMemberById(localState.player, characterPlayerId);
    if (!characterPlayer) {
      LOG(WARN) << "UiSetSpellReady::act: character not found " << characterPlayerId
                << LOG_ENDL;
      return;
    }

    auto* database = getDatabase();
    if (!database) {
      LOG(WARN) << "UiSetSpellReady::act: database is nullptr" << LOG_ENDL;
      return;
    }

    // if (ready) {
    //   const auto result = model::characterPlayerPrepareSpell(
    //       *characterPlayer, bmin::toStringView(spellName), true, *database);
    //   if (result == model::SpellReadyResult::CANNOT_EQUIP) {
    //     UiFloatingNotification notification;
    //     notification.id = model::createRandomId();
    //     notification.message = TRANSLATE("That spell cannot be prepared.");
    //     notification.type = UiFloatingNotificationType::WARNING;
    //     model::timerStructStart(notification.timer,
    //                             state->settings.floatingNotificationDurationMs);
    //     localState.uiState.floatingNotifications.pushBack(std::move(notification));
    //   }
    //   return;
    // }

    // model::characterPlayerUnprepareSpell(*characterPlayer,
    // bmin::toStringView(spellName));
  }

public:
  UiSetSpellReady(const bmin::String& _characterPlayerId,
                  const bmin::String& _spellName,
                  bool _ready)
      : characterPlayerId(_characterPlayerId), spellName(_spellName), ready(_ready) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiToggleEquipInventoryItem.cppm ---

namespace state {

namespace actions {

class UiToggleEquipInventoryItem : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::UiToggleEquipInventoryItem;
  }
  bmin::String characterPlayerId;
  bmin::String itemId;

  void act() override {
    auto& localState = *state;
    auto* characterPlayer =
        model::playerFindPartyMemberById(localState.player, characterPlayerId);
    if (!characterPlayer) {
      LOG(WARN) << "UiToggleEquipInventoryItem::act: character not found "
                << characterPlayerId << LOG_ENDL;
      return;
    }

    auto* database = getDatabase();
    if (!database) {
      LOG(WARN) << "UiToggleEquipInventoryItem::act: database is nullptr" << LOG_ENDL;
      return;
    }

    const auto result =
        game::toggleEquippedInventoryItem(*characterPlayer, itemId, *database);

    switch (result) {
    case model::EquipItemResult::SLOT_OCCUPIED: {
      UiFloatingNotification notification;
      notification.id = model::createRandomId();
      notification.message = TRANSLATE("An item is already equipped.");
      notification.type = UiFloatingNotificationType::WARNING;
      model::timerStructStart(notification.timer,
                              state->settings.floatingNotificationDurationMs);
      localState.uiState.floatingNotifications.pushBack(std::move(notification));
      break;
    }
    case model::EquipItemResult::TWO_HANDED_OFF_HAND: {
      UiFloatingNotification notification;
      notification.id = model::createRandomId();
      notification.message =
          TRANSLATE("A two-handed weapon cannot be equipped in the off hand.");
      notification.type = UiFloatingNotificationType::WARNING;
      model::timerStructStart(notification.timer,
                              state->settings.floatingNotificationDurationMs);
      localState.uiState.floatingNotifications.pushBack(std::move(notification));
      break;
    }
    default:
      break;
    }
  }

public:
  UiToggleEquipInventoryItem(const bmin::String& _characterPlayerId,
                             const bmin::String& _itemId)
      : characterPlayerId(_characterPlayerId), itemId(_itemId) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiToggleManaSlotRune.cppm ---

namespace state {

namespace actions {

class UiToggleManaSlotRune : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiToggleManaSlotRune; }
  bmin::String characterPlayerId;
  size_t slotIndex = 0;

  void act() override {
    auto& localState = *state;
    auto* characterPlayer =
        model::playerFindPartyMemberById(localState.player, characterPlayerId);
    if (!characterPlayer) {
      LOG(WARN) << "UiToggleManaSlotRune::act: character not found " << characterPlayerId
                << LOG_ENDL;
      return;
    }

    const auto result =
        model::characterPlayerToggleManaSlotRune(*characterPlayer, slotIndex);

    switch (result) {
    case model::EquipRuneResult::SLOT_OCCUPIED: {
      UiFloatingNotification notification;
      notification.id = model::createRandomId();
      notification.message = TRANSLATE("A rune is already equipped in that slot.");
      notification.type = UiFloatingNotificationType::WARNING;
      model::timerStructStart(notification.timer,
                              state->settings.floatingNotificationDurationMs);
      localState.uiState.floatingNotifications.pushBack(std::move(notification));
      break;
    }
    case model::EquipRuneResult::NO_RUNE_AVAILABLE: {
      UiFloatingNotification notification;
      notification.id = model::createRandomId();
      notification.message = TRANSLATE("No rune available to equip.");
      notification.type = UiFloatingNotificationType::WARNING;
      model::timerStructStart(notification.timer,
                              state->settings.floatingNotificationDurationMs);
      localState.uiState.floatingNotifications.pushBack(std::move(notification));
      break;
    }
    case model::EquipRuneResult::NOT_A_RUNE: {
      UiFloatingNotification notification;
      notification.id = model::createRandomId();
      notification.message = TRANSLATE("That item is not a rune.");
      notification.type = UiFloatingNotificationType::WARNING;
      model::timerStructStart(notification.timer,
                              state->settings.floatingNotificationDurationMs);
      localState.uiState.floatingNotifications.pushBack(std::move(notification));
      break;
    }
    default:
      break;
    }
  }

public:
  UiToggleManaSlotRune(const bmin::String& _characterPlayerId, size_t _slotIndex)
      : characterPlayerId(_characterPlayerId), slotIndex(_slotIndex) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiUpdateHeldMove.cppm ---

namespace state {

namespace actions {

class UiUpdateHeldMove : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiUpdateHeldMove; }
  HeldMove nextHeldMove;

  void act() override {
    auto& localState = *state;
    localState.uiState.heldMove = nextHeldMove;
  }

public:
  explicit UiUpdateHeldMove(HeldMove _nextHeldMove)
      : nextHeldMove(std::move(_nextHeldMove)) {}
};

} // namespace actions

} // namespace state
// --- actions/world/CharacterSetSpriteIndexOffset.cppm ---

namespace state::actions {

class CharacterSetSpriteIndexOffset : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::CharacterSetSpriteIndexOffset;
  }
  bmin::String characterId;
  int offset = 0;

  void act() override {
    if (!state) {
      return;
    }
    game::ActiveMapOrchestrator orch(
        state->world.activeMap, state->mapInstances, getDatabase());
    auto* character = orch.findCharacterById(characterId);
    if (character == nullptr) {
      return;
    }
    character->spriteIndexOffset = offset;
  }

public:
  CharacterSetSpriteIndexOffset(bmin::String _characterId, int _offset)
      : characterId(std::move(_characterId)), offset(_offset) {}
};

} // namespace state::actions
// --- actions/world/ClearTownEnemyAiResolving.cppm ---

namespace state {

namespace actions {

class ClearTownEnemyAiResolving : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::ClearTownEnemyAiResolving; }
  void act() override {
    if (!state) {
      return;
    }
    state->world.resolvingTownEnemyAi = false;
  }
};

} // namespace actions

} // namespace state
// --- actions/world/ModifyPartyMemberHp.cppm ---

namespace state {

namespace actions {

// Applies town-mode party HP change (victim need not be on the active map).
class ModifyPartyMemberHp : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::ModifyPartyMemberHp; }
  bmin::String instanceId;
  int delta = 0;

  void act() override {
    if (!state) {
      return;
    }
    model::modifyPartyMemberHp(state->player, instanceId, delta);
  }

public:
  ModifyPartyMemberHp(bmin::String _instanceId, int _delta)
      : instanceId(std::move(_instanceId)), delta(_delta) {}
};

} // namespace actions

} // namespace state
// --- actions/world/WorldExamineAt.cppm ---

namespace state {

namespace actions {

class WorldExamineAt : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldExamineAt; }
  sdl2w::Window* window = nullptr;
  int x = 0;
  int y = 0;

  static bool isAdjacentOrSame(int ax, int ay, int bx, int by) {
    return std::abs(ax - bx) <= 1 && std::abs(ay - by) <= 1;
  }

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldExamineAt::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldExamineAt::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch(world.activeMap, state->mapInstances, getDatabase());
    orch.fetchMapGrid(world.activeMap.gridId);
    auto* map = orch.getMapInstanceAt(x, y);
    const auto local = orch.activeMapCoordToInstanceCoord(x, y);
    if (!map || !local.valid) {
      return;
    }
    map->tileLayerNumber = world.activeMap.mapLayer;

    if (!game::isTileCurrentlyVisible(*map, local.x, local.y)) {
      LOG(INFO) << "You can't see there." << LOG_ENDL;
      return;
    }

    world.actionMode = model::WorldActionMode::NONE;
    world.actionAimTile.reset();

    const auto* tile = game::tileAtCurrentLayer(*map, local.x, local.y);
    if (tile && tile->eventTrigger && tile->eventTrigger->requiresLook) {
      state->triggers.pendingSpecialEventId = tile->eventTrigger->eventId;
      return;
    }

    const bool isContainer =
        tile != nullptr && game::isTileEffectivelyContainer(*tile, *database);
    if (isContainer) {
      const auto* avatar =
          game::findPartyAvatarOnActiveMap(world.activeMap, state->player);
      const bool adjacent =
          avatar != nullptr && isAdjacentOrSame(avatar->x, avatar->y, x, y);
      if (adjacent && window) {
        const auto contents = game::collectItemsAtActiveMapTile(world.activeMap, x, y);
        if (contents.empty()) {
          LOG(INFO) << TRANSLATE("Nothing inside.") << LOG_ENDL;
          return;
        }
        pushLayerRequest(*state, LayerRequest{.id = LayerId::PickUp, .x = x, .y = y});
        return;
      }
      LOG(INFO) << game::formatExamineMessage(
                       *map, world.activeMap, x, y, local.x, local.y, *database)
                << LOG_ENDL;
      LOG(INFO) << TRANSLATE("You need to get closer to look inside.") << LOG_ENDL;
      return;
    }

    LOG(INFO) << game::formatExamineMessage(
                     *map, world.activeMap, x, y, local.x, local.y, *database)
              << LOG_ENDL;
  }

public:
  WorldExamineAt(int _x, int _y) : x(_x), y(_y) {}
  WorldExamineAt(sdl2w::Window* _window, int _x, int _y)
      : window(_window), x(_x), y(_y) {}
};

} // namespace actions

} // namespace state
// --- actions/world/WorldInteractAt.cppm ---

namespace state {

namespace actions {

class WorldInteractAt : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldInteractAt; }
  void act() override {
    if (!state) {
      return;
    }

    auto& world = state->world;
    const auto* avatar = game::findPartyAvatarOnActiveMap(world.activeMap, state->player);
    if (!avatar || world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch(world.activeMap, state->mapInstances, getDatabase());
    orch.fetchMapGrid(world.activeMap.gridId);
    auto* map = orch.getMapInstanceAt(avatar->x, avatar->y);
    const auto local = orch.activeMapCoordToInstanceCoord(avatar->x, avatar->y);
    if (!map || !local.valid) {
      return;
    }
    map->tileLayerNumber = world.activeMap.mapLayer;

    state->triggers.pendingTravel =
        game::resolveActionTravelAtStanding(*map, local.x, local.y);
  }
};

} // namespace actions

} // namespace state
// --- actions/world/WorldLoadActiveMap.cppm ---

namespace state {

namespace actions {

class WorldLoadActiveMap : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldLoadActiveMap; }
  bmin::String gridId;

  void saveCurrentMapToPersistentState() {
    auto& localState = *state;

    auto previousGridId = localState.world.activeMap.gridId;
    if (previousGridId.empty()) {
      return;
    }
    game::ActiveMapOrchestrator previousActiveMap(
        localState.world.activeMap, localState.mapInstances, getDatabase());
    previousActiveMap.fetchMapGrid(previousGridId);

    for (auto ch : localState.world.activeMap.characters) {
      if (model::isPartyMember(localState.player, ch.id)) {
        continue;
      }
      auto* map = previousActiveMap.getMapInstanceAt(ch.x, ch.y);
      if (!map) {
        map = previousActiveMap.getDefaultMapInstance();
      }
      if (!map) {
        continue;
      }
      const auto local = previousActiveMap.activeMapCoordToInstanceCoord(ch.x, ch.y);
      if (local.valid) {
        ch.x = local.x;
        ch.y = local.y;
      }
      map->persistentState.characters.pushBack(std::move(ch));
    }
    for (auto item : localState.world.activeMap.items) {
      auto* map = previousActiveMap.getMapInstanceAt(item.x, item.y);
      if (!map) {
        map = previousActiveMap.getDefaultMapInstance();
      }
      if (!map) {
        continue;
      }
      const auto local = previousActiveMap.activeMapCoordToInstanceCoord(item.x, item.y);
      if (local.valid) {
        item.x = local.x;
        item.y = local.y;
      }
      map->persistentState.items.pushBack(std::move(item));
    }
  }

  void act() override {
    auto& localState = *state;

    auto* database = getDatabase();
    if (!database) {
      return;
    }

    const auto resolvedGridId = game::resolveGridIdForMapOrGrid(*database, gridId);
    if (resolvedGridId.empty()) {
      return;
    }

    if (localState.mapInstances.empty()) {
      localState.mapInstances = game::createMapInstances(*database);
    }

    saveCurrentMapToPersistentState();

    localState.world.activeMap = {};
    localState.world.activeMap.gridId = resolvedGridId;
    localState.world.camera.camX = 0;
    localState.world.camera.camY = 0;
    localState.world.camera.cameraMode = model::CameraMode::Follow;
    localState.world.camera.cameraFollowCharacterId = bmin::String{};
    localState.world.actionMode = model::WorldActionMode::NONE;
    localState.world.actionAimTile.reset();
    localState.world.pendingSpellId = bmin::String{};

    game::ActiveMapOrchestrator activeMap(
        localState.world.activeMap, localState.mapInstances, database);
    activeMap.fetchMapGrid(resolvedGridId);
    auto& grid = activeMap.getMapGrid();
    for (int y = 0; y < grid.gridHeight; y++) {
      for (int x = 0; x < grid.gridWidth; x++) {
        const auto& mapName = grid.cells[static_cast<size_t>(y)][static_cast<size_t>(x)];
        if (mapName.empty()) {
          continue;
        }
        auto it = localState.mapInstances.find(mapName);
        if (it == localState.mapInstances.end()) {
          continue;
        }
        auto& map = it->value;
        auto& persistentState = map.persistentState;

        // Drop defeated characters before hoisting.
        for (size_t ci = 0; ci < persistentState.characters.size();) {
          const auto& character = persistentState.characters[ci];
          auto remove = false;
          for (const auto& record : persistentState.defeatedCharacters) {
            if (character.templateName != record.templateName) {
              continue;
            }
            const auto spawnX = character.spawnX >= 0 ? character.spawnX : character.x;
            const auto spawnY = character.spawnY >= 0 ? character.spawnY : character.y;
            if (spawnX == record.x && spawnY == record.y) {
              remove = true;
              break;
            }
          }
          if (remove) {
            persistentState.characters.erase(ci);
          } else {
            ++ci;
          }
        }

        auto* database = getDatabase();
        for (auto character : persistentState.characters) {
          const auto worldLoc =
              activeMap.instanceCoordToActiveMapCoord(mapName, character.x, character.y);
          if (worldLoc.valid) {
            character.x = worldLoc.x;
            character.y = worldLoc.y;
          }
          if (database) {
            game::applyCharacterTemplateFromDatabase(character, *database);
          }
          localState.world.activeMap.characters.pushBack(std::move(character));
        }
        for (auto item : persistentState.items) {
          const auto worldLoc =
              activeMap.instanceCoordToActiveMapCoord(mapName, item.x, item.y);
          if (worldLoc.valid) {
            item.x = worldLoc.x;
            item.y = worldLoc.y;
          }
          localState.world.activeMap.items.pushBack(std::move(item));
        }
        persistentState.characters.clear();
        persistentState.items.clear();
      }
    }
  }

public:
  WorldLoadActiveMap(const bmin::String& gridId) : gridId(gridId) {}
};

} // namespace actions

} // namespace state
// --- actions/world/WorldMoveActionAim.cppm ---

namespace state {

namespace actions {

class WorldMoveActionAim : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldMoveActionAim; }
  int dx = 0;
  int dy = 0;

  void act() override {
    if (!state) {
      return;
    }
    if (state->world.actionMode == model::WorldActionMode::NONE) {
      return;
    }
    if (!state->world.actionAimTile) {
      return;
    }
    if (state->world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch(
        state->world.activeMap, state->mapInstances, getDatabase());
    orch.fetchMapGrid(state->world.activeMap.gridId);
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || total.x <= 0 || total.y <= 0) {
      return;
    }

    auto& aim = *state->world.actionAimTile;
    auto nextX = aim.x + dx;
    auto nextY = aim.y + dy;
    if (nextX < 0) {
      nextX = 0;
    } else if (nextX >= total.x) {
      nextX = total.x - 1;
    }
    if (nextY < 0) {
      nextY = 0;
    } else if (nextY >= total.y) {
      nextY = total.y - 1;
    }
    aim.x = nextX;
    aim.y = nextY;
  }

public:
  WorldMoveActionAim(int _dx, int _dy) : dx(_dx), dy(_dy) {}
};

} // namespace actions

} // namespace state
// --- actions/world/WorldSetActionAim.cppm ---

namespace state {

namespace actions {

class WorldSetActionAim : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldSetActionAim; }
  int x = 0;
  int y = 0;

  void act() override {
    if (!state) {
      return;
    }
    if (state->world.actionMode == model::WorldActionMode::NONE) {
      return;
    }
    if (state->world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch(
        state->world.activeMap, state->mapInstances, getDatabase());
    orch.fetchMapGrid(state->world.activeMap.gridId);
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || total.x <= 0 || total.y <= 0) {
      return;
    }

    auto nextX = x;
    auto nextY = y;
    if (nextX < 0) {
      nextX = 0;
    } else if (nextX >= total.x) {
      nextX = total.x - 1;
    }
    if (nextY < 0) {
      nextY = 0;
    } else if (nextY >= total.y) {
      nextY = total.y - 1;
    }

    if (state->world.actionAimTile && state->world.actionAimTile->x == nextX &&
        state->world.actionAimTile->y == nextY) {
      return;
    }
    state->world.actionAimTile = model::TileXY{nextX, nextY};
  }

public:
  WorldSetActionAim(int _x, int _y) : x(_x), y(_y) {}
};

} // namespace actions

} // namespace state
// --- actions/world/WorldSetActionMode.cppm ---

namespace state::actions {

class WorldSetActionMode : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldSetActionMode; }
  model::WorldActionMode mode = model::WorldActionMode::NONE;
  WorldSetActionModeCtx ctx;

  void act() override {
    if (!state) {
      return;
    }
    game::resolveWorldActionMode(
        state->world, state->player, mode, ctx.spellId, ctx.chId);
  }

public:
  explicit WorldSetActionMode(model::WorldActionMode _mode,
                              const WorldSetActionModeCtx& _ctx = {})
      : mode(_mode), ctx(_ctx) {}
};

} // namespace state::actions
// --- actions/world/WorldSetCamera.cppm ---

namespace state {

namespace actions {

class WorldSetCamera : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldSetCamera; }
  int camX = 0;
  int camY = 0;

  void act() override {
    if (!state) {
      return;
    }
    state->world.camera.camX = camX;
    state->world.camera.camY = camY;
  }

public:
  WorldSetCamera(int _camX, int _camY) : camX(_camX), camY(_camY) {}
};

} // namespace actions

} // namespace state
// --- actions/world/WorldSetCameraMode.cppm ---

namespace state {

namespace actions {

class WorldSetCameraMode : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldSetCameraMode; }
  model::CameraMode cameraMode = model::CameraMode::Follow;

  void act() override {
    if (!state) {
      return;
    }
    state->world.camera.cameraMode = cameraMode;
  }

public:
  explicit WorldSetCameraMode(model::CameraMode _cameraMode) : cameraMode(_cameraMode) {}
};

} // namespace actions

} // namespace state
// --- actions/world/WorldSpawnDamageParticle.cppm ---

namespace state::actions {

class WorldSpawnDamageParticle : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldSpawnDamageParticle; }
  bmin::String animationName;
  bmin::String text;
  int tileX = 0;
  int tileY = 0;
  int lifetimeMs = 0;

  void act() override {
    if (!state) {
      return;
    }

    model::DamageParticle particle;
    particle.animationName = animationName;
    particle.tileX = tileX;
    particle.tileY = tileY;
    particle.text = text;
    model::timerStructStart(particle.lifetime, lifetimeMs);
    state->world.activeMap.damageParticles.pushBack(std::move(particle));
  }

public:
  WorldSpawnDamageParticle(const bmin::String& _animationName,
                           const bmin::String& _text,
                           int _tileX,
                           int _tileY,
                           int _lifetimeMs)
      : animationName(_animationName),
        text(_text),
        tileX(_tileX),
        tileY(_tileY),
        lifetimeMs(_lifetimeMs) {}
};

} // namespace state::actions
// --- actions/world/WorldSpawnPlayerAtMarker.cppm ---

namespace state {

namespace actions {

// Spawns (or re-spawns) one player avatar CharacterInstance at a named marker on
// the active map grid. Does not move the camera.
class WorldSpawnPlayerAtMarker : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldSpawnPlayerAtMarker; }
  bmin::String markerName;

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: no active map loaded" << LOG_ENDL;
      return;
    }

    game::ActiveMapOrchestrator orch(world.activeMap, state->mapInstances, database);
    orch.fetchMapGrid(world.activeMap.gridId);
    const auto& grid = orch.getMapGrid();

    game::ActiveMapMarker found{};
    for (int y = 0; y < grid.gridHeight && !found.valid; ++y) {
      for (int x = 0; x < grid.gridWidth && !found.valid; ++x) {
        const auto& mapName = grid.cells[static_cast<size_t>(y)][static_cast<size_t>(x)];
        if (mapName.empty()) {
          continue;
        }
        found = orch.findMarker(mapName, markerName);
      }
    }

    if (!found.valid) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: marker not found: " << markerName
                 << LOG_ENDL;
      return;
    }

    world.activeMap.mapLayer = found.layer;

    if (!game::placePartyAvatarAt(
            world.activeMap, state->player, found.x, found.y, database)) {
      LOG(ERROR) << "WorldSpawnPlayerAtMarker::act: party is empty" << LOG_ENDL;
      return;
    }

    game::updateActiveMapVisibilityFromPlayer(
        world, state->mapInstances, found.x, found.y, *database);
  }

public:
  explicit WorldSpawnPlayerAtMarker(bmin::String _markerName = "MarkerPlayer")
      : markerName(std::move(_markerName)) {}
};

} // namespace actions

} // namespace state
// --- actions/world/WorldSpawnPlayerAtXY.cppm ---

namespace state {

namespace actions {

// Places the current party avatar at world tile coordinates on the active map.
class WorldSpawnPlayerAtXY : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldSpawnPlayerAtXY; }
  int destX = 0;
  int destY = 0;

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: no active map loaded" << LOG_ENDL;
      return;
    }

    game::ActiveMapOrchestrator orch(world.activeMap, state->mapInstances, database);
    orch.fetchMapGrid(world.activeMap.gridId);
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || destX < 0 || destY < 0 || destX >= total.x || destY >= total.y) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: destination out of bounds" << LOG_ENDL;
      return;
    }

    if (!game::placePartyAvatarAt(
            world.activeMap, state->player, destX, destY, database)) {
      LOG(ERROR) << "WorldSpawnPlayerAtXY::act: party is empty" << LOG_ENDL;
      return;
    }

    game::updateActiveMapVisibilityFromPlayer(
        world, state->mapInstances, destX, destY, *database);
  }

public:
  WorldSpawnPlayerAtXY(int _destX, int _destY) : destX(_destX), destY(_destY) {}
};

} // namespace actions

} // namespace state
// --- actions/world/WorldSpawnProjectile.cppm ---

namespace state::actions {

class WorldSpawnProjectile : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldSpawnProjectile; }
  bmin::String animationName;
  float fromTileX = 0.f;
  float fromTileY = 0.f;
  float toTileX = 0.f;
  float toTileY = 0.f;
  int travelMs = 0;
  model::ProjectilePath projectilePath = model::ProjectilePath::PROJECTILE_PATH_NONE;

  void act() override {
    if (!state) {
      return;
    }

    model::WorldProjectile projectile;
    projectile.animationName = animationName;
    projectile.fromTileX = fromTileX;
    projectile.fromTileY = fromTileY;
    projectile.toTileX = toTileX;
    projectile.toTileY = toTileY;
    projectile.projectilePath = projectilePath;
    model::timerStructStart(projectile.travel, travelMs);
    state->world.activeMap.projectiles.pushBack(std::move(projectile));
  }

public:
  WorldSpawnProjectile(const bmin::String& _animationName,
                       float _fromTileX,
                       float _fromTileY,
                       float _toTileX,
                       float _toTileY,
                       int _travelMs,
                       model::ProjectilePath _projectilePath)
      : animationName(_animationName),
        fromTileX(_fromTileX),
        fromTileY(_fromTileY),
        toTileX(_toTileX),
        toTileY(_toTileY),
        travelMs(_travelMs),
        projectilePath(_projectilePath) {}
};

} // namespace state::actions
// --- actions/world/WorldTalkAt.cppm ---

namespace state {

namespace actions {

// Confirm Talk at an absolute map tile: start that character's talk special event
// (CharacterTemplate.talk.talkName) if present.
class WorldTalkAt : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldTalkAt; }
  int x = 0;
  int y = 0;

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldTalkAt::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldTalkAt::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      return;
    }

    game::ActiveMapOrchestrator orch(world.activeMap, state->mapInstances, getDatabase());
    orch.fetchMapGrid(world.activeMap.gridId);
    auto* map = orch.getMapInstanceAt(x, y);
    const auto local = orch.activeMapCoordToInstanceCoord(x, y);
    if (!map || !local.valid) {
      return;
    }
    map->tileLayerNumber = world.activeMap.mapLayer;

    if (!game::isTileCurrentlyVisible(*map, local.x, local.y)) {
      LOG(INFO) << TRANSLATE("You can't see there.") << LOG_ENDL;
      return;
    }

    world.actionMode = model::WorldActionMode::NONE;
    world.actionAimTile.reset();

    const model::CharacterInstance* target = nullptr;
    for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
      const auto& character = world.activeMap.characters[i];
      if (character.x != x || character.y != y) {
        continue;
      }
      if (!target) {
        target = &character;
      }
      try {
        const auto& characterTemplate =
            database->getCharacterTemplate(bmin::toStringView(character.templateName));
        if (!characterTemplate.talk.talkName.empty()) {
          target = &character;
          break;
        }
      } catch (...) {
      }
    }

    if (!target) {
      LOG(INFO) << TRANSLATE("Talk: there is no one there.") << LOG_ENDL;
      return;
    }

    try {
      const auto& characterTemplate =
          database->getCharacterTemplate(bmin::toStringView(target->templateName));
      const auto& talkName = characterTemplate.talk.talkName;
      if (talkName.empty()) {
        LOG(INFO) << TRANSLATE("Talk: they have nothing to say.") << LOG_ENDL;
        return;
      }
      if (!database->getGameEvents().contains(talkName)) {
        LOG(ERROR) << "WorldTalkAt: talk event not found: " << talkName << LOG_ENDL;
        LOG(INFO) << TRANSLATE("Talk: they have nothing to say.") << LOG_ENDL;
        return;
      }
      state->triggers.pendingSpecialEventId = talkName;
    } catch (...) {
      LOG(INFO) << TRANSLATE("Talk: they have nothing to say.") << LOG_ENDL;
    }
  }

public:
  WorldTalkAt(int _x, int _y) : x(_x), y(_y) {}
};

} // namespace actions

} // namespace state
// --- actions/combat/RemoveCharacterFromMap.cppm ---

namespace state {

namespace actions {

class RemoveCharacterFromMap : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::RemoveCharacterFromMap; }
  bmin::String characterId;

  void act() override {
    if (!state) {
      return;
    }
    auto& characters = state->world.activeMap.characters;
    for (size_t i = 0; i < characters.size();) {
      if (characters[i].id == characterId) {
        if (model::isCharacterEnemy(characters[i])) {
          if (auto* database = getDatabase()) {
            game::markMapCharacterDefeated(
                state->world.activeMap, state->mapInstances, characters[i], *database);
          }
        }
        characters.erase(i);
        if (state->world.combat.active) {
          model::removeCharacterFromCombatTurnOrder(state->world.combat, characterId);
        }
        return;
      }
      i++;
    }
  }

public:
  explicit RemoveCharacterFromMap(bmin::String _characterId)
      : characterId(std::move(_characterId)) {}
};

} // namespace actions

} // namespace state
// --- actions/combat/GoNextCombatTurn.cppm ---

namespace state {

namespace actions {

class GoNextCombatTurn : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::GoNextCombatTurn; }
  void startNewCombatRound() {
    LOG(INFO) << "GoNextCombatTurn: new combat round, resetting AP" << LOG_ENDL;
    state->world.combat.activeTurnIndex = 0;
    model::resetAllCombatAp(state->world, model::COMBAT_STARTING_AP);
    state->playerMovementCount += game::TILE_FIELD_MOVES_PER_COMBAT_ROUND;
    game::ageMapInstances(state->mapInstances, game::TILE_FIELD_MOVES_PER_COMBAT_ROUND);
  }

  void act() override {
    if (!state) {
      return;
    }

    auto& combat = state->world.combat;
    if (!combat.active || combat.turnOrderIds.empty()) {
      return;
    }

    LOG(INFO) << "GoNextCombatTurn: advancing from turn index " << combat.activeTurnIndex
              << LOG_ENDL;

    combat.activeTurnIndex += 1;
    if (combat.activeTurnIndex >= static_cast<int>(combat.turnOrderIds.size())) {
      startNewCombatRound();
    }

    game::ActiveMapOrchestrator orch(
        state->world.activeMap, state->mapInstances, getDatabase());
    const auto turnCount = static_cast<int>(combat.turnOrderIds.size());
    for (int attempt = 0; attempt < turnCount; attempt++) {
      const auto index = combat.activeTurnIndex;
      if (index < 0 || index >= turnCount) {
        break;
      }
      const auto& nextId = combat.turnOrderIds[static_cast<size_t>(index)];
      auto* nextCharacter = orch.findCharacterById(nextId);
      if (nextCharacter == nullptr) {
        combat.activeTurnIndex += 1;
        if (combat.activeTurnIndex >= turnCount) {
          startNewCombatRound();
        }
        continue;
      }
      if (model::isCharacterDefeated(state->player, *nextCharacter)) {
        combat.activeTurnIndex += 1;
        if (combat.activeTurnIndex >= turnCount) {
          startNewCombatRound();
        }
        continue;
      }
      insertAction(new SetActiveCombatCharacter(nextId), 0);
      LOG(INFO) << "GoNextCombatTurn: next actor is "
                << model::formatCharacterLogLabel(state->world.activeMap, nextId)
                << LOG_ENDL;
      return;
    }
  }

public:
  GoNextCombatTurn() = default;
};

} // namespace actions

} // namespace state
// --- actions/combat/StartCombat.cppm ---

namespace state {

namespace actions {

class StartCombat : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::StartCombat; }
  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    LOG(INFO) << "StartCombat: starting combat on grid " << world.activeMap.gridId
              << LOG_ENDL;
    state->turnMode = model::TurnMode::TURN_COMBAT;
    game::addPartyMembersToCombatMap(world, state->player, *database);
    game::updateActiveMapVisibilityFromParty(
        world, state->mapInstances, state->player, *database);
    world.combat = model::createCombatFromWorld(world, state->player);
    model::resetAllCombatAp(world, model::COMBAT_STARTING_AP);

    if (world.combat.turnOrderIds.empty()) {
      world.combat.active = false;
      LOG(WARN) << "StartCombat: no combatants found, aborting" << LOG_ENDL;
      return;
    }

    world.combat.activeTurnIndex = 0;
    LOG(INFO) << "StartCombat: turn order has " << world.combat.turnOrderIds.size()
              << " characters" << LOG_ENDL;
    insertAction(new SetActiveCombatCharacter(), 0);
  }

public:
  StartCombat() = default;
};

} // namespace actions

} // namespace state
// --- actions/world/PerformTownMeleeAttack.cppm ---

namespace state {

namespace actions {

// Town melee with the same swing / particle / reset timing as combat melee.
// Damages a random living party member; FX play on the party avatar tile.
class PerformTownMeleeAttack : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::PerformTownMeleeAttack; }
  bmin::String attackerId;

  static model::CharacterPlayer* pickRandomLivingPartyMember(model::Player& player) {
    bmin::DynArray<model::CharacterPlayer*> living;
    for (size_t i = 0; i < player.party.size(); i++) {
      if (player.party[i].currentHp > 0) {
        living.pushBack(&player.party[i]);
      }
    }
    if (living.empty()) {
      return nullptr;
    }
    const auto index = static_cast<size_t>(std::rand() % static_cast<int>(living.size()));
    return living[index];
  }

  void act() override {
    if (!state) {
      return;
    }

    game::ActiveMapOrchestrator orch(
        state->world.activeMap, state->mapInstances, getDatabase());
    auto* attacker = orch.findCharacterById(attackerId);
    auto* avatar =
        game::findPartyAvatarOnActiveMap(state->world.activeMap, state->player);
    if (attacker == nullptr || avatar == nullptr) {
      return;
    }

    auto* victim = pickRandomLivingPartyMember(state->player);
    if (victim == nullptr) {
      return;
    }

    model::updateCharacterFacingToward(*attacker, avatar->x, avatar->y);

    insertAction(new CharacterSetSpriteIndexOffset(attackerId, 1), 0);

    const auto hit = (std::rand() % 100) < model::COMBAT_HIT_CHANCE_PERCENT;
    if (hit) {
      insertAction(new PlaySound("punch1"), 0);
      insertAction(nullptr, 75);
      insertAction(
          new ModifyPartyMemberHp(victim->instanceId, -model::COMBAT_MELEE_DAMAGE), 0);
      insertAction(
          new WorldSpawnDamageParticle("splash_attack",
                                       bmin::toString(model::COMBAT_MELEE_DAMAGE),
                                       avatar->x,
                                       avatar->y,
                                       500),
          0);
      insertAction(nullptr, 500);
      LOG(INFO) << "TownMeleeAttack: " << attackerId << " hit " << victim->instanceId
                << " for " << model::COMBAT_MELEE_DAMAGE << LOG_ENDL;
    } else {
      insertAction(new PlaySound("whip"), 0);
      insertAction(nullptr, 300);
      LOG(DEBUG) << "TownMeleeAttack: miss by " << attackerId << " vs party member "
                 << victim->instanceId << LOG_ENDL;
    }
    insertAction(new CharacterSetSpriteIndexOffset(attackerId, 0), 0);
  }

public:
  explicit PerformTownMeleeAttack(bmin::String _attackerId)
      : attackerId(std::move(_attackerId)) {}
};

} // namespace actions

} // namespace state
// --- actions/world/WorldSpawnPlayer.cppm ---

namespace state {

namespace actions {

class WorldSpawnPlayer : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldSpawnPlayer; }
  bmin::String mapName;
  bmin::String markerName;
  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldSpawnPlayer::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldSpawnPlayer::act: state is nullptr" << LOG_ENDL;
      return;
    }

    const auto gridId = game::resolveGridIdForMapOrGrid(*database, mapName);
    if (gridId.empty()) {
      LOG(ERROR) << "WorldSpawnPlayer::act: could not resolve grid for " << mapName
                 << LOG_ENDL;
      return;
    }

    WorldLoadActiveMap(gridId).execute(state);
    WorldSpawnPlayerAtMarker(markerName).execute(state);
  }

public:
  explicit WorldSpawnPlayer(const bmin::String& _mapName, const bmin::String& _markerName)
      : mapName(_mapName), markerName(_markerName) {}
};

} // namespace actions

} // namespace state
// --- actions/world/WorldTravel.cppm ---

namespace state {

namespace actions {

class WorldTravel : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldTravel; }
  model::TravelTrigger travel;

  void act() override {
    if (!state) {
      return;
    }
    if (travel.destinationMapName.empty()) {
      return;
    }

    auto* database = getDatabase();
    if (!database) {
      return;
    }

    const auto gridId =
        game::resolveGridIdForMapOrGrid(*database, travel.destinationMapName);
    if (gridId.empty()) {
      LOG(ERROR) << "WorldTravel::act: could not resolve grid for "
                 << travel.destinationMapName << LOG_ENDL;
      return;
    }

    // Same grid: teleport only. Different grid: unload/reload active map entities.
    if (state->world.activeMap.gridId != gridId) {
      WorldLoadActiveMap(gridId).execute(state);
    }

    auto usedMarker = false;
    if (!travel.destinationMarkerName.empty()) {
      game::ActiveMapOrchestrator orch(
          state->world.activeMap, state->mapInstances, getDatabase());
      orch.fetchMapGrid(gridId);
      const auto marker =
          orch.findMarker(travel.destinationMapName, travel.destinationMarkerName);
      if (marker.valid) {
        state->world.activeMap.mapLayer = marker.layer;
        WorldSpawnPlayerAtXY(marker.x, marker.y).execute(state);
        usedMarker = true;
      } else {
        LOG(WARN) << "WorldTravel::act: marker not found on destination map, "
                     "falling back to XY: "
                  << travel.destinationMarkerName << LOG_ENDL;
      }
    }

    if (!usedMarker) {
      state->world.activeMap.mapLayer = travel.destinationLayer;
      // destinationX/Y are local to destinationMapName — convert to world.
      game::ActiveMapOrchestrator orch(
          state->world.activeMap, state->mapInstances, getDatabase());
      orch.fetchMapGrid(gridId);
      const auto worldLoc = orch.instanceCoordToActiveMapCoord(
          travel.destinationMapName, travel.destinationX, travel.destinationY);
      if (worldLoc.valid) {
        WorldSpawnPlayerAtXY(worldLoc.x, worldLoc.y).execute(state);
      } else {
        WorldSpawnPlayerAtXY(travel.destinationX, travel.destinationY).execute(state);
      }
    }
  }

public:
  explicit WorldTravel(model::TravelTrigger _travel) : travel(std::move(_travel)) {}
};

} // namespace actions

} // namespace state
// --- actions/combat/PerformCharacterDefeated.cppm ---

namespace state {

namespace actions {

class PerformCharacterDefeated : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::PerformCharacterDefeated; }
  bmin::String characterId;

  void act() override {
    if (state) {
      game::ActiveMapOrchestrator orch(
          state->world.activeMap, state->mapInstances, getDatabase());
      if (!state->world.activeMap.gridId.empty()) {
        orch.fetchMapGrid(state->world.activeMap.gridId);
      }
      if (auto* character = orch.findCharacterById(characterId)) {
        auto* map = orch.getMapInstanceAt(character->x, character->y);
        const auto local = orch.activeMapCoordToInstanceCoord(character->x, character->y);
        if (map && local.valid) {
          map->tileLayerNumber = state->world.activeMap.mapLayer;
          game::addTileFieldAt(*map, local.x, local.y, game::TileFieldType::BLOOD);
        }
      }
    }
    insertAction(new PlaySound("yell1"), 0);
    insertAction(new RemoveCharacterFromMap(characterId), 300);
  }

public:
  explicit PerformCharacterDefeated(bmin::String _characterId)
      : characterId(std::move(_characterId)) {}
};

} // namespace actions

} // namespace state
// --- actions/ui/UiSelectSpellCast.cppm ---

namespace state {

namespace actions {

/** Validate a known spell for combat cast; on success close cast list and enter SPELL
 * aim. */
class UiSelectSpellCast : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiSelectSpellCast; }
  bmin::String spellId;
  bmin::String chId;

  void pushWarning(bmin::String message) {
    UiFloatingNotification notification;
    notification.id = model::createRandomId();
    notification.message = std::move(message);
    notification.type = UiFloatingNotificationType::WARNING;
    model::timerStructStart(notification.timer,
                            state->settings.floatingNotificationDurationMs);
    state->uiState.floatingNotifications.pushBack(std::move(notification));
  }

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (!database) {
      LOG(WARN) << "UiSelectSpellCast::act: database is nullptr" << LOG_ENDL;
      return;
    }

    auto* character = model::playerFindPartyMemberById(
        state->player, state->world.combat.activeCharacterId);
    if (!character) {
      LOG(WARN) << "UiSelectSpellCast::act: active combat party member not found"
                << LOG_ENDL;
      pushWarning(TRANSLATE("Cannot cast that spell."));
      return;
    }

    const auto* spell = database->findSpellTemplate(bmin::toStringView(spellId));
    if (spell == nullptr) {
      pushWarning(TRANSLATE("Cannot cast that spell."));
      return;
    }

    UiRemoveLayer(LayerId::SpellCast).execute(state);
    game::resolveWorldActionMode(
        state->world, state->player, model::WorldActionMode::SPELL, spellId, chId);
  }

public:
  explicit UiSelectSpellCast(const bmin::String& _spellId, const bmin::String& _chId)
      : spellId((_spellId)), chId((_chId)) {}
};

} // namespace actions

} // namespace state
// --- actions/world/TownEnemySeekAndMelee.cppm ---

namespace state {

namespace actions {

// One agitated enemy: optional seek step, then town melee if adjacent.
class TownEnemySeekAndMelee : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::TownEnemySeekAndMelee; }
  bmin::String enemyId;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    game::ActiveMapOrchestrator orch(
        state->world.activeMap, state->mapInstances, database);
    auto* enemy = orch.findCharacterById(enemyId);
    auto* avatar =
        game::findPartyAvatarOnActiveMap(state->world.activeMap, state->player);
    if (enemy == nullptr || avatar == nullptr) {
      return;
    }

    if (game::isChebyshevAdjacent(enemy->x, enemy->y, avatar->x, avatar->y)) {
      insertAction(new PerformTownMeleeAttack(enemyId), 0);
      return;
    }

    auto dx = 0;
    auto dy = 0;
    if (!game::chooseSeekStepToward(state->world.activeMap,
                                    state->mapInstances,
                                    *enemy,
                                    avatar->x,
                                    avatar->y,
                                    *database,
                                    dx,
                                    dy)) {
      return;
    }

    model::updateCharacterFacingFromMove(*enemy, dx, dy);
    enemy->x += dx;
    enemy->y += dy;
    LOG(DEBUG) << "TownEnemyAi: " << enemyId << " stepped (" << dx << ", " << dy << ")"
               << LOG_ENDL;

    if (game::isChebyshevAdjacent(enemy->x, enemy->y, avatar->x, avatar->y)) {
      insertAction(new PerformTownMeleeAttack(enemyId), 0);
    }
  }

public:
  explicit TownEnemySeekAndMelee(bmin::String _enemyId) : enemyId(std::move(_enemyId)) {}
};

} // namespace actions

} // namespace state
// --- actions/combat/DoCombatActionCompletion.cppm ---

namespace state {

namespace actions {

class DoCombatActionCompletion : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::DoCombatActionCompletion; }
  void act() override {
    if (!state) {
      return;
    }

    auto& world = state->world;
    auto& combat = world.combat;

    LOG(INFO) << "DoCombatActionCompletion: checking results for "
              << model::formatCharacterLogLabel(world.activeMap, combat.activeCharacterId)
              << LOG_ENDL;

    bmin::DynArray<bmin::String> defeatedIds;
    for (const auto& character : world.activeMap.characters) {
      if (model::isCharacterDefeated(state->player, character)) {
        defeatedIds.pushBack(character.id);
      }
    }
    for (const auto& id : defeatedIds) {
      insertAction(new PerformCharacterDefeated(id), 0);
    }

    game::ActiveMapOrchestrator orch(
        state->world.activeMap, state->mapInstances, getDatabase());
    auto* activeCharacter = orch.findCharacterById(combat.activeCharacterId);
    const auto apRemaining = activeCharacter != nullptr ? activeCharacter->currentAp : 0;
    const auto turnEnded = apRemaining <= 0;

    if (turnEnded) {
      LOG(INFO) << "DoCombatActionCompletion: turn ended, advancing to next character"
                << LOG_ENDL;
      insertAction(new GoNextCombatTurn(), 0);
    } else if (activeCharacter != nullptr) {
      LOG(INFO) << "DoCombatActionCompletion: "
                << model::formatCharacterLogLabel(world.activeMap,
                                                  combat.activeCharacterId)
                << " has " << apRemaining << " AP remaining, waiting for next action"
                << LOG_ENDL;
      insertAction(new SetActiveCombatCharacter(combat.activeCharacterId), 0);
    }
  }

public:
  DoCombatActionCompletion() = default;
};

} // namespace actions

} // namespace state
// --- actions/world/TownEnemyAiAfterPlayerMove.cppm ---

namespace state {

namespace actions {

// Spotting + one town action per agitated enemy (queued with combat-style delays).
class TownEnemyAiAfterPlayerMove : public AbstractAction {
  ActionEvent getEvent() const override {
    return ActionEvent::TownEnemyAiAfterPlayerMove;
  }
  void act() override {
    if (!state) {
      return;
    }
    auto& world = state->world;
    if (world.combat.active) {
      world.resolvingTownEnemyAi = false;
      return;
    }

    world.resolvingTownEnemyAi = true;
    // Held-move stays active; LayerWorld pauses repeats while this flag is set.

    auto* database = getDatabase();
    if (database == nullptr) {
      world.resolvingTownEnemyAi = false;
      return;
    }
    game::updateEnemySpotting(world, state->mapInstances, state->player, *database);

    for (size_t i = 0; i < world.activeMap.characters.size(); i++) {
      const auto& character = world.activeMap.characters[i];
      if (!character.agitated) {
        continue;
      }
      if (!model::characterInstanceIsEnemy(character)) {
        continue;
      }
      if (character.behaviorName != "IMMOBILE_UNTIL_ENEMY_SPOTTED") {
        continue;
      }
      if (character.combatBehaviorTown != model::CombatBehaviorName::SEEK_AND_MELEE) {
        continue;
      }
      insertAction(new TownEnemySeekAndMelee(character.id), 0);
    }

    insertAction(new ClearTownEnemyAiResolving(), 0);
  }
};

} // namespace actions

} // namespace state
// --- actions/combat/DoCombatAction.cppm ---

namespace state {

namespace actions {

class DoCombatAction : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::DoCombatAction; }
  bmin::String chId;
  model::CombatActionType actionType = model::CombatActionType::WAIT;
  CombatActionContext ctx;

  void handleMove() {
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    const auto& actorId = world.combat.activeCharacterId;
    game::ActiveMapOrchestrator orch(world.activeMap, state->mapInstances, getDatabase());
    if (!world.activeMap.gridId.empty()) {
      orch.fetchMapGrid(world.activeMap.gridId);
    }
    auto* actor = orch.findCharacterById(actorId);
    if (actor == nullptr) {
      return;
    }
    auto dx = ctx.targetLoc.x;
    auto dy = ctx.targetLoc.y;

    model::updateCharacterFacingFromMove(*actor, dx, dy);

    const auto destX = actor->x + dx;
    const auto destY = actor->y + dy;
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || destX < 0 || destY < 0 || destX >= total.x || destY >= total.y) {
      return;
    }

    if (auto* occupant = orch.findCharacterAt(destX, destY, actorId)) {
      const auto actorIsEnemy = model::isCharacterEnemy(*actor);
      const auto occupantIsEnemy = model::isCharacterEnemy(*occupant);
      if (actorIsEnemy != occupantIsEnemy) {
        insertAction(new PerformMeleeAttack(actorId, occupant->id), 0);
        insertAction(new ModifyAP(actorId, -model::COMBAT_ATTACK_COST), 0);
        return;
      }
      return;
    }

    auto* destMap = orch.getMapInstanceAt(destX, destY);
    const auto destLocal = orch.activeMapCoordToInstanceCoord(destX, destY);
    if (!destMap || !destLocal.valid) {
      return;
    }
    destMap->tileLayerNumber = world.activeMap.mapLayer;
    if (!game::isDestinationWalkable(*destMap, destLocal.x, destLocal.y, *database)) {
      return;
    }

    insertAction(new MoveCharacter(actorId, ctx.targetLoc.x, ctx.targetLoc.y), 0);
    insertAction(new ModifyAP(actorId, -model::COMBAT_MOVE_COST), 0);
  }

  void handleSpell() {
    model::SpellTargetInfo spellTargetInfo;
    spellTargetInfo.targetCharacterId = ctx.targetChId;
    spellTargetInfo.tileX = ctx.targetLoc.x;
    spellTargetInfo.tileY = ctx.targetLoc.y;
    if (ctx.abilityId.empty()) {
      LOG(INFO) << "DoCombatAction: SPELL with empty spellId" << LOG_ENDL;
      return;
    }
    insertAction(new PerformSpellCast(chId, ctx.abilityId, spellTargetInfo), 0);
    insertAction(nullptr, 150);
  }

  void act() override {
    if (!state || !state->world.combat.active) {
      return;
    }

    const char* actionLabel = "?";
    switch (actionType) {
    case model::CombatActionType::MOVE:
      actionLabel = "MOVE";
      break;
    case model::CombatActionType::SHOOT:
      actionLabel = "SHOOT";
      break;
    case model::CombatActionType::SPELL:
      actionLabel = "SPELL";
      break;
    case model::CombatActionType::WAIT:
      actionLabel = "WAIT";
      break;
    }
    if (actionType == model::CombatActionType::MOVE) {
      LOG(INFO) << "DoCombatAction: " << actionLabel << " for "
                << model::formatCharacterLogLabel(state->world.activeMap,
                                                  state->world.combat.activeCharacterId)
                << " (" << ctx.targetLoc.x << ", " << ctx.targetLoc.y << ")" << LOG_ENDL;
    } else {
      LOG(INFO) << "DoCombatAction: " << actionLabel << " for "
                << model::formatCharacterLogLabel(state->world.activeMap,
                                                  state->world.combat.activeCharacterId)
                << LOG_ENDL;
    }

    state->world.combat.isWaitingForAction = false;

    switch (actionType) {
    case model::CombatActionType::MOVE:
      handleMove();
      break;
    case model::CombatActionType::SPELL:
      handleSpell();
      break;
    case model::CombatActionType::SHOOT:
      insertAction(new DoCombatActionCompletion(), 0);
      break;
    case model::CombatActionType::WAIT: {
      game::ActiveMapOrchestrator orch(
          state->world.activeMap, state->mapInstances, getDatabase());
      orch.fetchMapGrid(state->world.activeMap.gridId);
      auto* character = orch.findCharacterById(chId);
      if (character != nullptr) {
        insertAction(new ModifyAP(chId, -character->currentAp), 0);
      }
      break;
    }
    }
    insertAction(new DoCombatActionCompletion(), 0);
  }

public:
  explicit DoCombatAction(const bmin::String& chId, model::CombatActionType _actionType)
      : chId(chId), actionType(_actionType) {}

  DoCombatAction(const bmin::String& chId,
                 model::CombatActionType _actionType,
                 const CombatActionContext& ctx)
      : chId(chId), actionType(_actionType), ctx(ctx) {}
};

} // namespace actions

} // namespace state
// --- actions/world/WorldMovePlayer.cppm ---

namespace state {

namespace actions {

// Moves the current party avatar by (dx, dy) tiles, or opens a closed door on bump.
class WorldMovePlayer : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::WorldMovePlayer; }
  int dx = 0;
  int dy = 0;

  const char* moveDirectionLabel(int dx, int dy) {
    if (dx < 0 && dy < 0) {
      return "nw";
    }
    if (dx == 0 && dy < 0) {
      return "n";
    }
    if (dx > 0 && dy < 0) {
      return "ne";
    }
    if (dx < 0 && dy == 0) {
      return "w";
    }
    if (dx > 0 && dy == 0) {
      return "e";
    }
    if (dx < 0 && dy > 0) {
      return "sw";
    }
    if (dx == 0 && dy > 0) {
      return "s";
    }
    if (dx > 0 && dy > 0) {
      return "se";
    }
    return "?";
  }

  void act() override {
    auto* database = getDatabase();
    if (!database) {
      LOG(ERROR) << "WorldMovePlayer::act: database is nullptr" << LOG_ENDL;
      return;
    }
    if (!state) {
      LOG(ERROR) << "WorldMovePlayer::act: state is nullptr" << LOG_ENDL;
      return;
    }

    auto& world = state->world;
    if (world.activeMap.gridId.empty()) {
      return;
    }
    if (!world.combat.active && world.resolvingTownEnemyAi) {
      return;
    }

    game::ActiveMapOrchestrator orch(world.activeMap, state->mapInstances, database);
    orch.fetchMapGrid(world.activeMap.gridId);
    const auto total = orch.getTotalMapTilesSize();
    if (!total.valid || total.x <= 0 || total.y <= 0) {
      return;
    }

    auto& player = state->player;
    if (player.party.empty()) {
      LOG(ERROR) << "WorldMovePlayer::act: party is empty" << LOG_ENDL;
      return;
    }

    auto* avatar = game::findPartyAvatarOnActiveMap(world.activeMap, player);
    if (!avatar) {
      LOG(ERROR) << "WorldMovePlayer::act: party avatar not found on map" << LOG_ENDL;
      return;
    }

    model::updateCharacterFacingFromMove(*avatar, dx, dy);

    const auto destX = avatar->x + dx;
    const auto destY = avatar->y + dy;
    LOG(DEBUG) << "WorldMovePlayer: move " << moveDirectionLabel(dx, dy) << LOG_ENDL;

    if (destX < 0 || destY < 0 || destX >= total.x || destY >= total.y) {
      LOG(DEBUG) << " blocked!" << LOG_ENDL;
      return;
    }

    auto* destMap = orch.getMapInstanceAt(destX, destY);
    const auto destLocal = orch.activeMapCoordToInstanceCoord(destX, destY);
    if (!destMap || !destLocal.valid) {
      LOG(DEBUG) << " blocked!" << LOG_ENDL;
      return;
    }
    destMap->tileLayerNumber = world.activeMap.mapLayer;

    if (auto* door =
            game::findClosedDoorAt(*destMap, destLocal.x, destLocal.y, *database)) {
      door->tileId = door->tileId + 1;
      game::updateActiveMapVisibilityFromPlayer(
          world, state->mapInstances, avatar->x, avatar->y, *database);
      return;
    }

    if (!game::isDestinationWalkable(*destMap, destLocal.x, destLocal.y, *database)) {
      LOG(DEBUG) << " blocked!" << LOG_ENDL;
      return;
    }

    // Town/outdoor: characters occupy tiles. Combat handles collide-to-attack separately.
    if (orch.findCharacterAt(destX, destY, avatar->id) != nullptr) {
      LOG(DEBUG) << " blocked!" << LOG_ENDL;
      return;
    }

    avatar->x = destX;
    avatar->y = destY;
    auto triggerResult = game::resolveStepTriggersAt(*destMap, destLocal.x, destLocal.y);
    state->triggers.pendingSpecialEventId = std::move(triggerResult.specialEventId);
    state->triggers.pendingTravel = std::move(triggerResult.travel);
    game::updateActiveMapVisibilityFromPlayer(
        world, state->mapInstances, destX, destY, *database);
    if (!world.combat.active) {
      state->playerMovementCount += 1;
      game::ageMapInstances(state->mapInstances, 1);
      world.resolvingTownEnemyAi = true;
      insertAction(new TownEnemyAiAfterPlayerMove(), 0);
    }
  }

public:
  WorldMovePlayer(int _dx, int _dy) : dx(_dx), dy(_dy) {}
};

} // namespace actions

} // namespace state
// --- actions/combat/DoCPUCombatTurn.cppm ---

namespace state {

namespace actions {

class DoCPUCombatTurn : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::DoCPUCombatTurn; }
  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    auto& world = state->world;
    const auto& actorId = world.combat.activeCharacterId;

    // Stale CPU turns can land after GoNextCombatTurn advances to a party member.
    // Re-arm waiting so the player can act; do not auto-WAIT away their turn.
    if (model::isPartyMember(state->player, actorId)) {
      world.combat.isWaitingForAction = true;
      return;
    }

    LOG(INFO) << "DoCPUCombatTurn: choosing action for "
              << model::formatCharacterLogLabel(world.activeMap, actorId) << LOG_ENDL;

    game::ActiveMapOrchestrator orch(world.activeMap, state->mapInstances, getDatabase());
    if (!world.activeMap.gridId.empty()) {
      orch.fetchMapGrid(world.activeMap.gridId);
    }
    auto* actor = orch.findCharacterById(actorId);
    if (actor == nullptr) {
      insertAction(nullptr, 300);
      insertAction(new DoCombatAction(actorId, model::CombatActionType::WAIT), 0);
      return;
    }

    if (actor->combatBehaviorCombat == model::CombatBehaviorName::SEEK_AND_MELEE) {
      auto dx = 0;
      auto dy = 0;
      if (game::chooseSeekAndMeleeCombatAction(
              world, state->mapInstances, state->player, *actor, *database, dx, dy)) {
        insertAction(nullptr, 300);
        insertAction(new DoCombatAction(
                         actorId, model::CombatActionType::MOVE, {.targetLoc = {dx, dy}}),
                     0);
        return;
      }
    }

    insertAction(nullptr, 300);
    insertAction(new DoCombatAction(actorId, model::CombatActionType::WAIT), 0);
  }

public:
  DoCPUCombatTurn() = default;
};

} // namespace actions

} // namespace state
namespace state {

namespace actions {

void PerformMeleeAttack::act() {
  if (!state) {
    return;
  }
  auto* database = getDatabase();
  if (database == nullptr) {
    return;
  }

  game::ActiveMapOrchestrator orch(state->world.activeMap, state->mapInstances, database);
  auto* attacker = orch.findCharacterById(attackerId);
  auto* victim = orch.findCharacterById(victimId);
  if (attacker == nullptr || victim == nullptr) {
    return;
  }

  model::updateCharacterFacingToward(*attacker, victim->x, victim->y);

  insertAction(new CharacterSetSpriteIndexOffset(attackerId, 1), 0);

  const auto hit = (std::rand() % 100) < model::COMBAT_HIT_CHANCE_PERCENT;
  if (hit) {
    insertAction(new PlaySound("hit_punch1"), 0);
    insertAction(nullptr, 75);
    insertAction(new ModifyHP(victimId, -model::COMBAT_MELEE_DAMAGE), 0);
    insertAction(new WorldSpawnDamageParticle("splash_attack",
                                              bmin::toString(model::COMBAT_MELEE_DAMAGE),
                                              victim->x,
                                              victim->y,
                                              500),
                 0);
    insertAction(nullptr, 500);
  } else {
    insertAction(new PlaySound("miss"), 0);
    insertAction(nullptr, 300);
  }
  insertAction(new CharacterSetSpriteIndexOffset(attackerId, 0), 0);
}

void PerformSpellCast::doZoneSpell(const model::AbilityTemplate& ability,
                                   model::CharacterInstance& caster,
                                   game::ActiveMapOrchestrator& orch) {
  const auto& depiction = ability.depiction;
  auto casterX = caster.x;
  auto casterY = caster.y;
  auto targetTileX = spellTargetInfo.tileX;
  auto targetTileY = spellTargetInfo.tileY;
  auto zoneW = ability.targetSelect.zoneSize.x;
  auto zoneH = ability.targetSelect.zoneSize.y;
  // ignore attacks/restores

  // TODO status effects

  bmin::DynArray<model::CharacterInstance*> charactersInZone;
  bmin::DynArray<int> damageDealtToCharactersInZone;

  for (int i = 0; i < zoneW; ++i) {
    for (int j = 0; j < zoneH; ++j) {
      const auto tileX = targetTileX + i - zoneW / 2;
      const auto tileY = targetTileY + j - zoneH / 2;
      auto chAtTile = orch.findAllCharactersAt(tileX, tileY);
      for (auto ch : chAtTile) {
        charactersInZone.pushBack(ch);
        damageDealtToCharactersInZone.pushBack(0);
      }
    }
  }

  for (size_t i = 0; i < charactersInZone.size(); i++) {
    auto ch = charactersInZone[i];
    auto damageDealt = 0;
    for (const auto& damage : ability.damages) {
      auto result = game::calculateAbilityDamage(damage, caster, *ch);
      damageDealt += result.damage;
    }
    damageDealtToCharactersInZone[i] = damageDealt;
  }

  model::updateCharacterFacingToward(
      caster, spellTargetInfo.tileX, spellTargetInfo.tileY);

  insertAction(new CharacterSetSpriteIndexOffset(casterId, 1), 0);
  insertAction(new PlaySound(depiction.startSound), 0);

  int delayMs = 300;
  if (depiction.projectileType != model::ProjectileType::PROJECTILE_NONE) {
    delayMs = game::getProjectileTravelDurationMs(depiction.projectilePath);
    auto animBase = model::projectileTypeToAnimBase(depiction.projectileType);
    if (!animBase.empty()) {
      if (model::projectileTypeHasFacing(depiction.projectileType)) {
        animBase += game::getProjectileFacingSuffix(spellTargetInfo.tileX - caster.x,
                                                    spellTargetInfo.tileY - caster.y);
      }
      insertAction(new WorldSpawnProjectile(animBase,
                                            static_cast<float>(casterX),
                                            static_cast<float>(casterY),
                                            static_cast<float>(targetTileX),
                                            static_cast<float>(targetTileY),
                                            delayMs,
                                            depiction.projectilePath),
                   0);
    }
  }

  insertAction(nullptr, delayMs);

  const int damageParticleLifetimeMs = 500;
  if (!depiction.dmgAnim.empty()) {
    for (size_t i = 0; i < charactersInZone.size(); i++) {
      auto ch = charactersInZone[i];
      auto chX = ch->x;
      auto chY = ch->y;
      auto damageDealt = damageDealtToCharactersInZone[i];
      insertAction(new WorldSpawnDamageParticle(depiction.dmgAnim,
                                                bmin::toString(damageDealt),
                                                chX,
                                                chY,
                                                damageParticleLifetimeMs),
                   i * 50);
      insertAction(new ModifyHP(casterId, damageDealt), i * 50);
    }
  }

  if (charactersInZone.size() > 0) {
    insertAction(new PlaySound(depiction.dmgSound), 0);
    insertAction(nullptr, damageParticleLifetimeMs);
  } else {
    LOG(INFO) << "Missed!" << LOG_ENDL;
  }
}

void PerformSpellCast::act() {
  if (!state) {
    return;
  }

  // gather data

  auto database = getDatabase();
  if (database == nullptr) {
    return;
  }
  game::ActiveMapOrchestrator orch(state->world.activeMap, state->mapInstances, database);
  orch.fetchMapGrid(state->world.activeMap.gridId);

  auto caster = orch.findCharacterById(casterId);
  if (caster == nullptr) {
    LOG(ERROR) << "PerformSpellCast: no map character for caster " << casterId
               << LOG_ENDL;
    return;
  }
  auto spell = database->findSpellTemplate(bmin::toStringView(spellId));
  if (spell == nullptr) {
    LOG(ERROR) << "PerformSpellCast: no spell for " << spellId << LOG_ENDL;
    return;
  }
  const auto ability =
      database->findAbilityTemplate(bmin::toStringView(spell->abilityName));
  if (ability == nullptr) {
    LOG(ERROR) << "PerformSpellCast: missing ability " << spell->abilityName << LOG_ENDL;
    return;
  }

  // verify

  // if (!verifySpellCanBeCast(*caster, *ability)) {
  //   LOG(ERROR) << "PerformSpellCast: spell not allowed to be cast" << LOG_ENDL;
  //   return;
  // }

  // do

  if (ability->apCost != 0) {
    insertAction(new ModifyAP(casterId, -ability->apCost), 0);
  }

  if (ability->targetSelect.targetType == model::TargetSelectType::TARGET_ZONE) {
    doZoneSpell(*ability, *caster, orch);
  }

  insertAction(new CharacterSetSpriteIndexOffset(casterId, 0), 0);
  insertAction(new WorldSetActionMode(model::WorldActionMode::NONE), 0);
}

} // namespace actions

} // namespace state

namespace state::actions {

Command::Command(AbstractAction* action) : action(action) {}

Command::Command(Command&& other) noexcept : action(other.release()) {}

Command& Command::operator=(Command&& other) noexcept {
  if (this != &other) {
    delete action;
    action = other.release();
  }
  return *this;
}

Command::~Command() { delete action; }

void Command::execute(State* targetState) {
  if (action != nullptr) {
    action->execute(targetState);
  }
}

AbstractAction* Command::release() {
  auto* released = action;
  action = nullptr;
  return released;
}

Command::operator AbstractAction*() && { return release(); }

Command doCPUCombatTurn() { return Command(new DoCPUCombatTurn()); }
Command doCombatAction(const bmin::String& id,
                       model::CombatActionType type,
                       const CombatActionContext& context) {
  return Command(new DoCombatAction(id, type, context));
}
Command endCombat() { return Command(new EndCombat()); }
Command goNextCombatTurn() { return Command(new GoNextCombatTurn()); }
Command modifyAP(bmin::String id, int delta) {
  return Command(new ModifyAP(std::move(id), delta));
}
Command modifyHP(bmin::String id, int delta) {
  return Command(new ModifyHP(std::move(id), delta));
}
Command modifyPartyMemberHp(bmin::String id, int delta) {
  return Command(new ModifyPartyMemberHp(std::move(id), delta));
}
Command playSound(bmin::String name) { return Command(new PlaySound(std::move(name))); }
Command setActiveCombatCharacter(bmin::String id) {
  return Command(new SetActiveCombatCharacter(std::move(id)));
}
Command startCombat() { return Command(new StartCombat()); }

Command adjustEquippedRune(const bmin::String& id, model::RuneType runeType, int delta) {
  return Command(new UiAdjustEquippedRune(id, runeType, delta));
}
Command cancelEquipRunes() { return Command(new UiCancelEquipRunes()); }
Command commitEquipRunes() { return Command(new UiCommitEquipRunes()); }
Command continueSpecialEvent() { return Command(new UiContinueSpecialEvent()); }
Command dropInventoryItem(bmin::String id, bmin::String itemId) {
  return Command(new UiDropInventoryItem(std::move(id), std::move(itemId)));
}
Command giveInventoryItem(bmin::String from,
                          bmin::String to,
                          bmin::String itemId,
                          int quantity) {
  return Command(new UiGiveInventoryItem(
      std::move(from), std::move(to), std::move(itemId), quantity));
}
Command pickUpItem(bmin::String itemId) {
  return Command(new UiPickUpItem(std::move(itemId)));
}
Command pushFloatingNotification(bmin::String message, UiFloatingNotificationType type) {
  return Command(new UiPushFloatingNotification(std::move(message), type));
}
Command removeFloatingNotification(bmin::String id) {
  return Command(new UiRemoveFloatingNotification(std::move(id)));
}
Command removeLayer(const bmin::String& id) { return Command(new UiRemoveLayer(id)); }
Command removeLayer(LayerId id) { return Command(new UiRemoveLayer(id)); }
Command reorderInventoryItem(const bmin::String& id, int index, int direction) {
  return Command(new UiReorderInventoryItem(id, index, direction));
}
Command selectSpecialEventChoice(int choiceIndex) {
  return Command(new UiSelectSpecialEventChoice(choiceIndex));
}
Command selectSpellCast(const bmin::String& spellId, const bmin::String& id) {
  return Command(new UiSelectSpellCast(spellId, id));
}
Command setCurrentPartyMember(int index) {
  return Command(new UiSetCurrentPartyMember(index));
}
Command setCurrentPartyMemberInventory(int index) {
  return Command(new UiSetCurrentPartyMemberInventory(index));
}
Command setCurrentPartyMemberMagic(int index) {
  return Command(new UiSetCurrentPartyMemberMagic(index));
}
Command setSelectedPartyMemberId(bmin::String id) {
  return Command(new UiSetSelectedPartyMemberId(std::move(id)));
}
Command setSpellReady(const bmin::String& id, const bmin::String& spellName, bool ready) {
  return Command(new UiSetSpellReady(id, spellName, ready));
}
Command showLayerDropContext(sdl2w::Window* window,
                             bmin::String id,
                             bmin::String itemId) {
  return Command(new UiShowLayerDropContext(window, std::move(id), std::move(itemId)));
}
Command showLayerEquipRunes(sdl2w::Window* window, const bmin::String& id) {
  return Command(new UiShowLayerEquipRunes(window, id));
}
Command showLayerGiveContext(sdl2w::Window* window,
                             bmin::String id,
                             bmin::String itemId) {
  return Command(new UiShowLayerGiveContext(window, std::move(id), std::move(itemId)));
}
Command showLayerInventory(sdl2w::Window* window) {
  return Command(new UiShowLayerInventory(window));
}
Command showLayerInventoryContext(sdl2w::Window* window,
                                  bmin::String itemName,
                                  bmin::String itemId) {
  return Command(
      new UiShowLayerInventoryContext(window, std::move(itemName), std::move(itemId)));
}
Command showLayerMagic(sdl2w::Window* window) {
  return Command(new UiShowLayerMagic(window));
}
Command showLayerPickUp(sdl2w::Window* window) {
  return Command(new UiShowLayerPickUp(window));
}
Command showLayerPickUp(sdl2w::Window* window, int x, int y) {
  return Command(new UiShowLayerPickUp(window, x, y));
}
Command showLayerPickupContext(sdl2w::Window* window, const model::ItemInstance& item) {
  return Command(new UiShowLayerPickupContext(window, item));
}
Command showLayerPopupText(sdl2w::Window* window, bmin::String title, bmin::String text) {
  return Command(new UiShowLayerPopupText(window, std::move(title), std::move(text)));
}
Command showLayerSpecialEvent(sdl2w::Window* window, bmin::String eventId) {
  return Command(new UiShowLayerSpecialEvent(window, std::move(eventId)));
}
Command showLayerSpellCast(sdl2w::Window* window, const bmin::String& id) {
  return Command(new UiShowLayerSpellCast(window, id));
}
Command showLayerSpellInfo(sdl2w::Window* window, const bmin::String& spellName) {
  return Command(new UiShowLayerSpellInfo(window, spellName));
}
Command toggleEquipInventoryItem(const bmin::String& id, const bmin::String& itemId) {
  return Command(new UiToggleEquipInventoryItem(id, itemId));
}
Command toggleManaSlotRune(const bmin::String& id, size_t slotIndex) {
  return Command(new UiToggleManaSlotRune(id, slotIndex));
}
Command updateHeldMove(HeldMove heldMove) {
  return Command(new UiUpdateHeldMove(std::move(heldMove)));
}

Command examineAt(int x, int y) { return Command(new WorldExamineAt(x, y)); }
Command examineAt(sdl2w::Window* window, int x, int y) {
  return Command(new WorldExamineAt(window, x, y));
}
Command interactAt() { return Command(new WorldInteractAt()); }
Command loadActiveMap(const bmin::String& gridId) {
  return Command(new WorldLoadActiveMap(gridId));
}
Command moveActionAim(int dx, int dy) { return Command(new WorldMoveActionAim(dx, dy)); }
Command movePlayer(int dx, int dy) { return Command(new WorldMovePlayer(dx, dy)); }
Command setActionAim(int x, int y) { return Command(new WorldSetActionAim(x, y)); }
Command setActionMode(model::WorldActionMode mode, const WorldSetActionModeCtx& context) {
  return Command(new WorldSetActionMode(mode, context));
}
Command spawnPlayer(const bmin::String& mapName, const bmin::String& markerName) {
  return Command(new WorldSpawnPlayer(mapName, markerName));
}
Command spawnPlayerAtMarker(bmin::String markerName) {
  return Command(new WorldSpawnPlayerAtMarker(std::move(markerName)));
}
Command spawnPlayerAtXY(int x, int y) { return Command(new WorldSpawnPlayerAtXY(x, y)); }
Command talkAt(int x, int y) { return Command(new WorldTalkAt(x, y)); }
Command travel(model::TravelTrigger trigger) {
  return Command(new WorldTravel(std::move(trigger)));
}

} // namespace state::actions
