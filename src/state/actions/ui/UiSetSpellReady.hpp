#pragma once

#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "game/combat/SpellRules.h"
#include "model/instances/Player.h"
#include "sdl2w/L10n.h"
#include "state/AbstractAction.h"
#include "state/State.h"

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
