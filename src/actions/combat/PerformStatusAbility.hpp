#pragma once

#include "bmin/StringInterop.h"
#include "game/combat/Damage.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "model/Combat.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include "actions/combat/ModifyHP.hpp"
#include "actions/general/PlaySound.hpp"
#include "actions/world/WorldSpawnDamageParticle.hpp"

namespace state {

namespace actions {

/** Applies an ability's HP delta and depiction to one character (status ticks). */
class PerformStatusAbility : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::PerformStatusAbility; }
  bmin::String characterId;
  bmin::String abilityName;

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
    auto* character = orch.findCharacterById(characterId);
    if (character == nullptr) {
      return;
    }
    const auto* ability =
        database->findAbilityTemplate(bmin::toStringView(abilityName));
    if (ability == nullptr) {
      LOG(ERROR) << "PerformStatusAbility: missing ability " << abilityName << LOG_ENDL;
      return;
    }

    auto zeroStats = model::CharacterStats{};
    const auto rolledHpDelta =
        game::calculateAbilityTemplateHpDelta(*ability, zeroStats);
    const auto currentHp = model::getCharacterHp(state->player, *character);
    const auto hpDelta =
        model::appliedHpDelta(currentHp, character->maxHp, rolledHpDelta);
    LOG(DEBUG) << "PerformStatusAbility: " << abilityName << " vs "
               << model::formatCharacterLogLabel(state->world.activeMap, characterId)
               << " hpDelta=" << hpDelta << " (rolled=" << rolledHpDelta << ")"
               << LOG_ENDL;

    const auto& depiction = ability->depiction;
    if (hpDelta != 0) {
      insertAction(state::makeAction<ModifyHP>(characterId, hpDelta), 0);
    }
    if (hpDelta != 0 && !depiction.dmgAnim.empty()) {
      const auto particleText =
          hpDelta > 0 ? bmin::toString(hpDelta) : bmin::toString(-hpDelta);
      insertAction(state::makeAction<WorldSpawnDamageParticle>(depiction.dmgAnim,
                                                particleText,
                                                character->x,
                                                character->y,
                                                500,
                                                depiction.dmgTextColor),
                   0);
    }
    if (!depiction.dmgSound.empty()) {
      insertAction(state::makeAction<PlaySound>(depiction.dmgSound), 0);
    }
    insertAction(nullptr, 500);
  }

public:
  PerformStatusAbility(bmin::String _characterId, bmin::String _abilityName)
      : characterId(std::move(_characterId)), abilityName(std::move(_abilityName)) {}
};

} // namespace actions

} // namespace state
