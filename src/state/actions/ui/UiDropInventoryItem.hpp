#pragma once

#include "bmin/String.h"
#include "layers/LayerManager.h"
#include "layers/ui/LayerDropConfirm.h"
#include "layers/ui/LayerInventoryContext.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "state/AbstractAction.h"
#include "state/State.h"

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

    int quantity = 0;
    for (const auto& item : partyMember->inventory) {
      if (item.id == itemId) {
        quantity = item.quantity;
        break;
      }
    }
    if (quantity < 1) {
      LOG(WARN) << "UiDropInventoryItem::act: item not found in inventory" << LOG_ENDL;
      return;
    }

    auto* database = getDatabase();
    if (!database) {
      LOG(WARN) << "UiDropInventoryItem::act: database is nullptr" << LOG_ENDL;
      return;
    }

    if (model::characterPlayerIsItemEquippedById(*partyMember, itemId)) {
      model::characterPlayerToggleEquipItem(*partyMember, itemId, *database);
    }

    // TODO: place dropped item in the world at the player's location.

    model::characterPlayerRemoveItemFromInventoryById(*partyMember, itemId, quantity);

    auto layerManager = getLayerManager();
    if (!layerManager) {
      return;
    }

    auto* dropLayer = layerManager->getLayerById(layers::LayerDropConfirm::LAYER_ID);
    if (dropLayer) {
      dropLayer->remove();
    }
    auto* inventoryContextLayer =
        layerManager->getLayerById(layers::LayerInventoryContext::LAYER_ID);
    if (inventoryContextLayer) {
      inventoryContextLayer->remove();
    }
    layerManager->moveToFront(layerManager->getLastActiveLayer());
  }

public:
  UiDropInventoryItem(bmin::String _characterPlayerId, bmin::String _itemId)
      : characterPlayerId(std::move(_characterPlayerId)), itemId(std::move(_itemId)) {}
};

} // namespace actions

} // namespace state
