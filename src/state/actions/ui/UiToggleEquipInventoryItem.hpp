#pragma once

#include "bmin/String.h"
#include "lib/sdl2w/L10n.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "state/AbstractAction.h"
#include "state/State.h"

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
