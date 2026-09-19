#pragma once

#include "bmin/String.h"
#include "bmin/StringInterop.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "model/Combat.h"
#include "model/instances/Player.h"
#include "sdl2w/L10n.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include "state/State.hpp"
#include "actions/combat/DoCombatAction.hpp"

namespace state {

namespace actions {

class UiSelectSpellAllyTarget : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::UiSelectSpellAllyTarget; }
  bmin::String spellId;
  bmin::String casterId;
  bmin::String targetId;

  void pushWarning(bmin::String message) {
    UiFloatingNotification notification;
    notification.id = model::createRandomId();
    notification.message = std::move(message);
    notification.type = UiFloatingNotificationType::WARNING;
    model::timerStructStart(notification.timer,
                            state->settings.floatingNotificationDurationMs);
    state->uiState.floatingNotifications.pushBack(std::move(notification));
    ++state->uiState.floatingNotificationRevision;
  }

  void act() override {
    if (!state) {
      return;
    }
    auto& world = state->world;
    if (!world.combat.active || !world.combat.isWaitingForAction ||
        !model::isPartyMember(state->player, world.combat.activeCharacterId)) {
      pushWarning(TRANSLATE("Cannot cast that spell."));
      return;
    }
    if (!model::isPartyMember(state->player, targetId)) {
      pushWarning(TRANSLATE("Choose a party member."));
      return;
    }

    game::ActiveMapOrchestrator orch(world.activeMap, state->mapInstances, getDatabase());
    auto* target = orch.findCharacterById(targetId);
    if (target == nullptr) {
      pushWarning(TRANSLATE("That ally is not on the map."));
      return;
    }

    removeLayerRequest(*state, LayerId::SpellAllyTarget);
    removeLayerRequest(*state, LayerId::SpellCast);
    insertAction(state::makeAction<DoCombatAction>(
                     casterId,
                     model::CombatActionType::SPELL,
                     CombatActionContext{.targetChId = targetId,
                                         .abilityId = spellId,
                                         .targetLoc = {target->x, target->y}}),
                 0);
  }

public:
  UiSelectSpellAllyTarget(const bmin::String& _spellId,
                          const bmin::String& _casterId,
                          const bmin::String& _targetId)
      : spellId(_spellId), casterId(_casterId), targetId(_targetId) {}
};

} // namespace actions

} // namespace state
