#pragma once

#include "bmin/String.h"
#include "game/inventory/InventoryRules.h"
#include "game/map/TileTriggers.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/ItemInstance.hpp"
#include "model/instances/Player.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"

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
      LOG(WARN) << "UiDropInventoryItem::act: no character on map to drop at"
                << LOG_ENDL;
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
