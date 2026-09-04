module;
#include <utility>
#include <cstddef>
#include <algorithm>
#include <optional>

export module carcer.actions:ui;
export import carcer.state;
import sdl2w;
import carcer.game.map;
import bmin.string_interop;
import carcer.lib.StringUtil;
import carcer.game.combat;
#include "macros.h"

export {

namespace state {

namespace actions {

/** Live-adjust equipped runes while the Equip Runes modal is open (delta +1 / -1). */
class UiAdjustEquippedRune : public AbstractAction {
  bmin::String characterPlayerId;
  model::RuneType runeType = model::RuneType::HEAT;
  int delta = 0;

  void act() override {
    auto* characterPlayer =
        model::playerFindPartyMemberById(state->player, characterPlayerId);
    if (!characterPlayer) {
      LOG(WARN) << "UiAdjustEquippedRune::act: character not found "
                << characterPlayerId << LOG_ENDL;
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

namespace state {

namespace actions {

/** Close the Equip Runes layer without committing edits.
    TODO(reconciler): snapshot restore belongs to the Layer itself (it can
    revert on close) once LayerManager::update() reconciles layerStack --
    see LayerRequest in carcer.state. */
class UiCancelEquipRunes : public AbstractAction {
  void act() override {
    if (!state) {
      return;
    }
    removeLayerRequest(*state, LayerId::EquipRunes);
  }
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

/** Keep live equipped-rune edits and close the Equip Runes layer. */
class UiCommitEquipRunes : public AbstractAction {
  void act() override {
    if (!state) {
      return;
    }
    removeLayerRequest(*state, LayerId::EquipRunes);
  }
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class UiDropInventoryItem : public AbstractAction {
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
      LOG(WARN) << "UiDropInventoryItem::act: no character on map to drop at"
                << LOG_ENDL;
      return;
    }

    if (model::characterPlayerIsItemEquippedById(*partyMember, itemId)) {
      model::characterPlayerToggleEquipItem(*partyMember, itemId, *database);
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

namespace state {

namespace actions {

class UiGiveInventoryItem : public AbstractAction {
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

    const auto result = model::characterPlayerGiveInventoryItem(
        *fromMember, *toMember, itemId, quantity, *database);

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

namespace state {

namespace actions {

class UiPickUpItem : public AbstractAction {
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
    if (model::characterGetWeightCarrying(*partyMember, database) + addedWeight >
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

namespace state {

namespace actions {

class UiPushFloatingNotification : public AbstractAction {
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
  }

public:
  UiPushFloatingNotification(bmin::String _message, UiFloatingNotificationType _type)
      : message(std::move(_message)), type(_type) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class UiRemoveFloatingNotification : public AbstractAction {
  bmin::String notificationId;

  void act() override {
    auto& notifications = state->uiState.floatingNotifications;
    notifications.erase(
        std::remove_if(notifications.begin(),
                       notifications.end(),
                       [&](const UiFloatingNotification& n) { return n.id == notificationId; }),
        notifications.end());
  }

public:
  explicit UiRemoveFloatingNotification(bmin::String _notificationId)
      : notificationId(std::move(_notificationId)) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class UiRemoveLayer : public AbstractAction {
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

namespace state {

namespace actions {

class UiReorderInventoryItem : public AbstractAction {
  bmin::String characterPlayerId;
  int inventoryIndex = 0;
  int direction = 0;

  void act() override {
    auto& localState = *state;
    auto* characterPlayer =
        model::playerFindPartyMemberById(localState.player, characterPlayerId);
    if (!characterPlayer) {
      LOG(WARN) << "UiReorderInventoryItem::act: character not found " << characterPlayerId
                << LOG_ENDL;
      return;
    }
    if (!model::characterPlayerReorderInventoryItem(
            *characterPlayer, static_cast<size_t>(inventoryIndex), direction)) {
      LOG(WARN) << "UiReorderInventoryItem::act: reorder failed index="
                << inventoryIndex << " direction=" << direction << LOG_ENDL;
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

namespace state {

namespace actions {

class UiSetCurrentPartyMember : public AbstractAction {
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

namespace state {

namespace actions {

class UiSetCurrentPartyMemberInventory : public AbstractAction {
  int partyMemberInventoryIndex = 0;

  void act() override {
    auto& localState = *state;

    int nextIndex = partyMemberInventoryIndex;
    if (partyMemberInventoryIndex < 0 ||
        partyMemberInventoryIndex >= static_cast<int>(localState.player.party.size())) {
      nextIndex = 0;
      LOG(WARN) << "UiSetCurrentPartyMemberInventory::act: index out of range "
                << partyMemberInventoryIndex << " "
                << localState.player.party.size() << LOG_ENDL;
    }
    localState.player.currentPartyMemberInventoryIndex = nextIndex;
  }

public:
  explicit UiSetCurrentPartyMemberInventory(int _partyMemberInventoryIndex)
      : partyMemberInventoryIndex(_partyMemberInventoryIndex) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class UiSetCurrentPartyMemberMagic : public AbstractAction {
  int partyMemberMagicIndex = 0;

  void act() override {
    auto& localState = *state;

    int nextIndex = partyMemberMagicIndex;
    if (partyMemberMagicIndex < 0 ||
        partyMemberMagicIndex >= static_cast<int>(localState.player.party.size())) {
      nextIndex = 0;
      LOG(WARN) << "UiSetCurrentPartyMemberMagic::act: index out of range "
                << partyMemberMagicIndex << " "
                << localState.player.party.size() << LOG_ENDL;
    }
    localState.player.currentPartyMemberMagicIndex = nextIndex;
  }

public:
  explicit UiSetCurrentPartyMemberMagic(int _partyMemberMagicIndex)
      : partyMemberMagicIndex(_partyMemberMagicIndex) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

/** UI selection only — does not affect map movement / party avatar. */
class UiSetSelectedPartyMemberId : public AbstractAction {
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

namespace state {

namespace actions {

class UiSetSpellReady : public AbstractAction {
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

    // model::characterPlayerUnprepareSpell(*characterPlayer, bmin::toStringView(spellName));
  }

public:
  UiSetSpellReady(const bmin::String& _characterPlayerId,
                  const bmin::String& _spellName,
                  bool _ready)
      : characterPlayerId(_characterPlayerId), spellName(_spellName), ready(_ready) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class UiShowLayerDropContext : public AbstractAction {
  bmin::String characterPlayerId;
  bmin::String itemId;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::DropConfirm,
                                  .a = characterPlayerId,
                                  .b = itemId});
  }

public:
  UiShowLayerDropContext(sdl2w::Window* /*_window*/,
                         bmin::String _characterPlayerId,
                         bmin::String _itemId)
      : characterPlayerId(std::move(_characterPlayerId)),
        itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class UiShowLayerEquipRunes : public AbstractAction {
  bmin::String characterPlayerId;

  void act() override {
    if (!state) {
      return;
    }
    pushLayerRequest(*state,
                     LayerRequest{.id = LayerId::EquipRunes, .a = characterPlayerId});
  }

public:
  UiShowLayerEquipRunes(sdl2w::Window* /*_window*/, const bmin::String& _characterPlayerId)
      : characterPlayerId(_characterPlayerId) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class UiShowLayerGiveContext : public AbstractAction {
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

namespace state {

namespace actions {

class UiShowLayerInventory : public AbstractAction {
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

namespace state {

namespace actions {

class UiShowLayerInventoryContext : public AbstractAction {
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

namespace state {

namespace actions {

class UiShowLayerMagic : public AbstractAction {
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

namespace state {

namespace actions {

class UiShowLayerPickUp : public AbstractAction {
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

namespace state {

namespace actions {

class UiShowLayerPickupContext : public AbstractAction {
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

namespace state {

namespace actions {

class UiShowLayerPopupText : public AbstractAction {
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
  UiShowLayerPopupText(sdl2w::Window* /*_window*/, bmin::String _title, bmin::String _text)
      : title(std::move(_title)), text(std::move(_text)) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class UiShowLayerSpecialEvent : public AbstractAction {
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

namespace state {

namespace actions {

class UiShowLayerSpellCast : public AbstractAction {
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

namespace state {

namespace actions {

class UiShowLayerSpellInfo : public AbstractAction {
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

namespace state {

namespace actions {

class UiToggleEquipInventoryItem : public AbstractAction {
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
        model::characterPlayerToggleEquipItem(*characterPlayer, itemId, *database);

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
  UiToggleEquipInventoryItem(const bmin::String& _characterPlayerId, const bmin::String& _itemId)
      : characterPlayerId(_characterPlayerId), itemId(_itemId) {}
};

} // namespace actions

} // namespace state

namespace state {

namespace actions {

class UiToggleManaSlotRune : public AbstractAction {
  bmin::String characterPlayerId;
  size_t slotIndex = 0;

  void act() override {
    auto& localState = *state;
    auto* characterPlayer =
        model::playerFindPartyMemberById(localState.player, characterPlayerId);
    if (!characterPlayer) {
      LOG(WARN) << "UiToggleManaSlotRune::act: character not found "
                << characterPlayerId << LOG_ENDL;
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

namespace state {

namespace actions {

class UiUpdateHeldMove : public AbstractAction {
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

namespace state {

namespace actions {

/** Validate a known spell for combat cast; on success close cast list and enter SPELL
 * aim. */
class UiSelectSpellCast : public AbstractAction {
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

  // body in ui.cpp: needs :world (WorldSetActionMode), impl-only.
  void act() override;

public:
  explicit UiSelectSpellCast(const bmin::String& _spellId, const bmin::String& _chId)
      : spellId((_spellId)), chId((_chId)) {}
};

} // namespace actions

} // namespace state

} // export
