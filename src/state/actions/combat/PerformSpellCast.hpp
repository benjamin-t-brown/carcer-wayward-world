#pragma once

#include "game/map/ActiveMapOrchestrator.h"
#include "model/Combat.h"
#include "model/SpellRules.h"
#include "model/instances/Player.h"
#include "sdl2w/Logger.h"
#include "state/actions/combat/ActionBase.hpp"
#include "state/actions/combat/CharacterSetSpriteIndexOffset.hpp"
#include "state/actions/combat/ModifyAP.hpp"
#include "state/actions/combat/ModifyHP.hpp"
#include "state/actions/combat/PlaySound.hpp"
#include "state/actions/world/WorldSpawnDamageParticle.hpp"
#include "bmin/StringInterop.h"

namespace state {

namespace actions {

/** Combat SPELL execution: canCastSpell → spend ability mana cost → apply linked ability effects. */
class PerformSpellCast : public CombatAction {
  model::CombatSpellTarget spellTarget;

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    const auto& actorId = state->world.combat.activeCharacterId;
    auto* caster = model::playerFindPartyMemberById(state->player, actorId);
    if (caster == nullptr) {
      LOG(INFO) << "PerformSpellCast: caster is not a party member: " << actorId
                << LOG_ENDL;
      return;
    }

    if (spellTarget.spellId.empty()) {
      LOG(INFO) << "PerformSpellCast: empty spellId" << LOG_ENDL;
      return;
    }

    // Party-member casts during the player's combat turn are player-controlled.
    const auto playerControlled = true;
    const auto outcome =
        model::castSpell(*caster,
                         bmin::toStringView(spellTarget.spellId),
                         bmin::toStringView(spellTarget.targetCharacterId),
                         playerControlled,
                         *database);

    if (outcome.result != model::CastSpellResult::CAST) {
      LOG(INFO) << "PerformSpellCast: cast failed for " << spellTarget.spellId
                << " result=" << static_cast<int>(outcome.result) << LOG_ENDL;
      return;
    }

    game::ActiveMapOrchestrator orch;
    auto* actor = orch.findCharacterById(actorId);
    auto* target = orch.findCharacterById(outcome.effects.targetCharacterId);
    if (actor != nullptr && target != nullptr && actor->id != target->id) {
      model::updateCharacterFacingToward(*actor, target->x, target->y);
    }

    insertCombatAction(new CharacterSetSpriteIndexOffset(actorId, 1), 0);

    if (!outcome.effects.depiction.startSound.empty()) {
      insertCombatAction(new PlaySound(outcome.effects.depiction.startSound), 0);
    }

    if (outcome.effects.hpDelta != 0 && target != nullptr) {
      if (!outcome.effects.depiction.dmgSound.empty()) {
        insertCombatAction(new PlaySound(outcome.effects.depiction.dmgSound), 0);
      }
      insertCombatAction(nullptr, 75);
      insertCombatAction(
          new ModifyHP(outcome.effects.targetCharacterId, outcome.effects.hpDelta), 0);

      if (!outcome.effects.depiction.dmgAnim.empty()) {
        const auto particleAmount =
            outcome.effects.hpDelta < 0 ? -outcome.effects.hpDelta : outcome.effects.hpDelta;
        insertCombatAction(new WorldSpawnDamageParticle(outcome.effects.depiction.dmgAnim,
                                                        target->x,
                                                        target->y,
                                                        particleAmount,
                                                        500),
                           0);
      }
      insertCombatAction(nullptr, 300);
    }

    if (outcome.effects.mpDelta != 0) {
      auto* targetPlayer =
          model::playerFindPartyMemberById(state->player, outcome.effects.targetCharacterId);
      if (targetPlayer != nullptr) {
        targetPlayer->currentMp += outcome.effects.mpDelta;
        if (targetPlayer->currentMp < 0) {
          targetPlayer->currentMp = 0;
        }
      }
    }

    if (outcome.effects.apDelta != 0) {
      insertCombatAction(
          new ModifyAP(outcome.effects.targetCharacterId, outcome.effects.apDelta), 0);
    }

    if (outcome.effects.casterApCost != 0) {
      insertCombatAction(new ModifyAP(actorId, -outcome.effects.casterApCost), 0);
    }

    insertCombatAction(new CharacterSetSpriteIndexOffset(actorId, 0), 0);

    LOG(INFO) << "PerformSpellCast: " << actorId << " cast " << spellTarget.spellId
              << " on " << outcome.effects.targetCharacterId
              << " hpDelta=" << outcome.effects.hpDelta
              << " mpSpent (caster now " << caster->currentMp << ")" << LOG_ENDL;
  }

public:
  explicit PerformSpellCast(model::CombatSpellTarget _spellTarget)
      : spellTarget(std::move(_spellTarget)) {}
};

} // namespace actions

} // namespace state
