#pragma once

#include "bmin/String.h"
#include "game/inventory/InventoryRules.h"
#include "sdl2w/L10n.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "state/AbstractAction.h"
#include "state/State.h"

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

    const auto result = game::giveInventoryItem(
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
      ++localState.uiState.floatingNotificationRevision;
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
