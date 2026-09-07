#pragma once

#include "bmin/String.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "sdl2w/L10n.h"
#include "state/AbstractAction.h"
#include "state/State.h"

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
