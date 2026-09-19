#pragma once

#include "bmin/StringInterop.h"
#include "game/combat/Damage.h"
#include "game/combat/MeleeAttackResolve.h"
#include "game/combat/SpellRules.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "model/Combat.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/stats/CharacterStats.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include "actions/combat/CharacterSetSpriteIndexOffset.hpp"
#include "actions/combat/ModifyHP.hpp"
#include "actions/general/PlaySound.hpp"
#include "actions/world/WorldSpawnDamageParticle.hpp"

namespace state {

namespace actions {

class PerformMeleeAttack : public AbstractAction {
  static constexpr int kHitRecoverDelayMs = 500;
  static constexpr int kMissRecoverDelayMs = 300;
  static constexpr int kInterHandDelayMs = 100;
  static constexpr int kDamageParticleLifetimeMs = 500;

  ActionEvent getEvent() const override { return ActionEvent::PerformMeleeAttack; }
  bmin::String attackerId;
  bmin::String victimId;

  void playSoundIfNamed(const bmin::String& soundName) {
    if (soundName.empty()) {
      return;
    }
    insertAction(state::makeAction<PlaySound>(soundName), 0);
  }

  int performAttack(const model::AbilityAttack& attack,
                     const model::AbilityDepiction& depiction,
                     const bmin::String& abilityName,
                     bool applyOffHandPenalty,
                     bool unarmed,
                     const model::CharacterStats& attackerStats,
                     int targetArmorClass,
                     int victimX,
                     int victimY) {
    insertAction(state::makeAction<CharacterSetSpriteIndexOffset>(attackerId, 1), 0);
    playSoundIfNamed(depiction.startSound);

    const auto abilityBonus = attack.dmg.has_value() ? attack.dmg->attackBonus : 0;
    const auto mastery = game::weaponMasteryBonus(attackerStats, attack, unarmed);
    const auto attackBonus = abilityBonus + mastery;
    LOG(DEBUG) << "PerformMeleeAttack: "
               << model::formatCharacterLogLabel(state->world.activeMap, attackerId)
               << " vs "
               << model::formatCharacterLogLabel(state->world.activeMap, victimId)
               << " ability=" << abilityName
               << (applyOffHandPenalty ? " off-hand" : " main-hand")
               << " unarmed=" << unarmed << " mastery=" << mastery
               << " abilityBonus=" << abilityBonus << LOG_ENDL;
    const auto hit = game::rollMeleeAttackHit(
        attack.attackClass, attackBonus, targetArmorClass, applyOffHandPenalty);
    auto damage = int{0};
    if (hit) {
      playSoundIfNamed(depiction.dmgSound);
      if (attack.dmg.has_value()) {
        damage = game::calculateAttackDamage(attack, attackerStats).damage;
        if (damage != 0) {
          insertAction(state::makeAction<ModifyHP>(victimId, -damage), 0);
        }
      }
      if (!depiction.dmgAnim.empty()) {
        insertAction(state::makeAction<WorldSpawnDamageParticle>(depiction.dmgAnim,
                                              bmin::toString(damage),
                                              victimX,
                                              victimY,
                                              kDamageParticleLifetimeMs,
                                              depiction.dmgTextColor),
                     0);
      }
      insertAction(nullptr, kHitRecoverDelayMs);
    } else {
      insertAction(state::makeAction<PlaySound>("miss"), 0);
      insertAction(nullptr, kMissRecoverDelayMs);
    }
    insertAction(state::makeAction<CharacterSetSpriteIndexOffset>(attackerId, 0), 0);
    return damage;
  }

  int performHand(const game::ResolvedMeleeAbility& hand,
                   bool dualWield,
                   const model::CharacterStats& attackerStats,
                   int targetArmorClass,
                   int victimX,
                   int victimY) {
    if (hand.ability.attacks.empty()) {
      LOG(INFO) << "PerformMeleeAttack: empty attacks for " << hand.ability.name
                << LOG_ENDL;
      return 0;
    }
    const auto applyOffHandPenalty = dualWield && hand.offHand;
    auto damageDealt = int{0};
    for (size_t i = 0; i < hand.ability.attacks.size(); ++i) {
      damageDealt += performAttack(hand.ability.attacks[i],
                                   hand.ability.depiction,
                                   hand.ability.name,
                                   applyOffHandPenalty,
                                   hand.unarmed,
                                   attackerStats,
                                   targetArmorClass,
                                   victimX,
                                   victimY);
    }
    return damageDealt;
  }

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }

    game::ActiveMapOrchestrator orch(state->world.activeMap, state->mapInstances, getDatabase());
    auto* attacker = orch.findCharacterById(attackerId);
    auto* victim = orch.findCharacterById(victimId);
    if (attacker == nullptr || victim == nullptr) {
      return;
    }

    model::updateCharacterFacingToward(*attacker, victim->x, victim->y);

    const auto resolved =
        game::resolveMeleeAttackAbilities(state->player, *attacker, *database);
    auto npcStats = model::CharacterStats{};
    auto caster = model::SpellCasterRef{};
    const model::CharacterStats* attackerStats = nullptr;
    if (model::resolveCombatSpellCaster(
            state->player, *attacker, *database, npcStats, caster) &&
        caster.stats != nullptr) {
      attackerStats = caster.stats;
    }
    auto zeroStats = model::CharacterStats{};
    if (attackerStats == nullptr) {
      attackerStats = &zeroStats;
    }

    auto victimNpcStats = model::CharacterStats{};
    auto victimCaster = model::SpellCasterRef{};
    const model::CharacterStats* defenderStats = nullptr;
    if (model::resolveCombatSpellCaster(
            state->player, *victim, *database, victimNpcStats, victimCaster) &&
        victimCaster.stats != nullptr) {
      defenderStats = victimCaster.stats;
    }
    if (defenderStats == nullptr) {
      defenderStats = &zeroStats;
    }
    const auto targetArmorClass = game::meleeTargetArmorClass(*defenderStats);

    const auto dualWield = resolved.size() >= 2;
    const auto victimX = victim->x;
    const auto victimY = victim->y;
    auto remainingHp = model::getCharacterHp(state->player, *victim);
    for (size_t i = 0; i < resolved.size(); ++i) {
      if (i > 0 && remainingHp <= 0) {
        LOG(DEBUG) << "PerformMeleeAttack: skipping off-hand, target would be defeated"
                   << LOG_ENDL;
        break;
      }
      remainingHp -= performHand(
          resolved[i], dualWield, *attackerStats, targetArmorClass, victimX, victimY);
      if (i + 1 < resolved.size() && remainingHp > 0) {
        insertAction(nullptr, kInterHandDelayMs);
      }
    }
  }

public:
  PerformMeleeAttack(bmin::String _attackerId, bmin::String _victimId)
      : attackerId(std::move(_attackerId)), victimId(std::move(_victimId)) {}
};

} // namespace actions

} // namespace state
