#pragma once

#include "actions/combat/CharacterSetSpriteIndexOffset.hpp"
#include "actions/general/PlaySound.hpp"
#include "actions/world/ModifyPartyMemberHp.hpp"
#include "actions/world/WorldSpawnDamageParticle.hpp"
#include "bmin/StringInterop.h"
#include "game/combat/Damage.h"
#include "game/combat/MeleeAttackResolve.h"
#include "game/combat/SpellRules.h"
#include "game/map/ActiveMapCharacters.h"
#include "model/Combat.h"
#include "model/instances/CharacterPlayer.h"
#include "model/stats/CharacterStats.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include <cstdlib>

namespace state {

namespace actions {

// Town melee: same resolve / depiction timing as combat melee, but damages a random
// living party member via ModifyPartyMemberHp and spawns FX on the party avatar tile.
class PerformTownMeleeAttack : public AbstractAction {
  static constexpr int kHitRecoverDelayMs = 500;
  static constexpr int kMissRecoverDelayMs = 300;
  static constexpr int kInterHandDelayMs = 100;
  static constexpr int kDamageParticleLifetimeMs = 500;

  bmin::String attackerId;

  ActionEvent getEvent() const override { return ActionEvent::PerformTownMeleeAttack; }

  static model::CharacterPlayer* pickRandomLivingPartyMember(model::Player& player) {
    bmin::DynArray<model::CharacterPlayer*> living;
    for (size_t i = 0; i < player.party.size(); i++) {
      if (player.party[i].currentHp > 0) {
        living.pushBack(&player.party[i]);
      }
    }
    if (living.empty()) {
      return nullptr;
    }
    const auto index = static_cast<size_t>(std::rand() % static_cast<int>(living.size()));
    return living[index];
  }

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
                     bool offHand,
                     bool unarmed,
                     const model::CharacterStats& attackerStats,
                     int targetArmorClass,
                     const bmin::String& victimInstanceId,
                     int avatarX,
                     int avatarY) {
    const auto attackPose = offHand ? 2 : 1;
    insertAction(state::makeAction<CharacterSetSpriteIndexOffset>(attackerId, attackPose),
                 0);
    playSoundIfNamed(depiction.startSound);

    const auto abilityBonus = attack.dmg.has_value() ? attack.dmg->attackBonus : 0;
    const auto attackBonus =
        game::meleeAttackToHitBonus(attackerStats, attack, unarmed);
    LOG(DEBUG) << "PerformTownMeleeAttack: "
               << model::formatCharacterLogLabel(state->world.activeMap, attackerId)
               << " vs party member " << victimInstanceId << " ability=" << abilityName
               << (applyOffHandPenalty ? " off-hand" : " main-hand")
               << " unarmed=" << unarmed << " abilityBonus=" << abilityBonus
               << " attackBonus=" << attackBonus << LOG_ENDL;
    const auto hit = game::rollMeleeAttackHit(attack.attackClass,
                                              attackBonus,
                                              targetArmorClass,
                                              applyOffHandPenalty,
                                              true);
    auto damage = int{0};
    if (hit) {
      playSoundIfNamed(depiction.dmgSound);
      if (attack.dmg.has_value()) {
        damage = game::calculateAttackDamage(attack, attackerStats).damage;
        if (damage != 0) {
          insertAction(
              state::makeAction<ModifyPartyMemberHp>(victimInstanceId, -damage), 0);
        }
      }
      if (!depiction.dmgAnim.empty()) {
        insertAction(state::makeAction<WorldSpawnDamageParticle>(depiction.dmgAnim,
                                              bmin::toString(damage),
                                              avatarX,
                                              avatarY,
                                              scaledDelayMs(kDamageParticleLifetimeMs),
                                              depiction.dmgTextColor),
                     0);
      }
      insertAction(nullptr, scaledDelayMs(kHitRecoverDelayMs));
    } else {
      insertAction(state::makeAction<PlaySound>("miss"), 0);
      insertAction(nullptr, scaledDelayMs(kMissRecoverDelayMs));
    }
    insertAction(state::makeAction<CharacterSetSpriteIndexOffset>(attackerId, 0), 0);
    return damage;
  }

  int performHand(const game::ResolvedMeleeAbility& hand,
                   bool dualWield,
                   const model::CharacterStats& attackerStats,
                   int targetArmorClass,
                   const bmin::String& victimInstanceId,
                   int avatarX,
                   int avatarY) {
    if (hand.ability.attacks.empty()) {
      LOG(INFO) << "PerformTownMeleeAttack: empty attacks for " << hand.ability.name
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
                                   hand.offHand,
                                   hand.unarmed,
                                   attackerStats,
                                   targetArmorClass,
                                   victimInstanceId,
                                   avatarX,
                                   avatarY);
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

    auto* attacker = game::findCharacterById(state->world.activeMap, attackerId);
    auto* avatar =
        game::findPartyAvatarOnActiveMap(state->world.activeMap, state->player);
    if (attacker == nullptr || avatar == nullptr) {
      return;
    }

    auto* victim = pickRandomLivingPartyMember(state->player);
    if (victim == nullptr) {
      return;
    }

    model::updateCharacterFacingToward(*attacker, avatar->x, avatar->y);

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

    const auto targetArmorClass = game::meleeTargetArmorClass(victim->stats);
    const auto dualWield = resolved.size() >= 2;
    const auto avatarX = avatar->x;
    const auto avatarY = avatar->y;
    const auto victimInstanceId = victim->instanceId;
    auto remainingHp = victim->currentHp;
    for (size_t i = 0; i < resolved.size(); ++i) {
      if (i > 0 && remainingHp <= 0) {
        LOG(DEBUG)
            << "PerformTownMeleeAttack: skipping off-hand, target would be defeated"
            << LOG_ENDL;
        break;
      }
      remainingHp -= performHand(resolved[i],
                                 dualWield,
                                 *attackerStats,
                                 targetArmorClass,
                                 victimInstanceId,
                                 avatarX,
                                 avatarY);
      if (i + 1 < resolved.size() && remainingHp > 0) {
        insertAction(nullptr, scaledDelayMs(kInterHandDelayMs));
      }
    }
  }

public:
  explicit PerformTownMeleeAttack(bmin::String _attackerId)
      : attackerId(std::move(_attackerId)) {}
};

} // namespace actions

} // namespace state
