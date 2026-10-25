#pragma once

#include "bmin/StringInterop.h"
#include "game/combat/Damage.h"
#include "game/combat/MeleeAttackResolve.h"
#include "game/combat/ShootRules.h"
#include "game/combat/SpellRules.h"
#include "game/combat/SpellZone.h"
#include "game/combat/StatusRules.h"
#include "game/combat/projectileHelpers.h"
#include "game/map/ActiveMapCharacters.h"
#include "game/map/Camera.h"
#include "game/map/TileDistance.h"
#include "model/Combat.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/stats/CharacterStats.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include <algorithm>
#include "actions/combat/CharacterSetSpriteIndexOffset.hpp"
#include "actions/combat/ModifyAP.hpp"
#include "actions/combat/ModifyHP.hpp"
#include "actions/combat/PerformStatusAbility.hpp"
#include "actions/general/PlaySound.hpp"
#include "actions/world/WorldSetActionMode.hpp"
#include "actions/world/WorldSetCamera.hpp"
#include "actions/world/WorldSetCameraMode.hpp"
#include "actions/world/WorldSpawnDamageParticle.hpp"
#include "actions/world/WorldSpawnProjectile.hpp"

namespace state {

namespace actions {

class PerformShoot : public AbstractAction {
  static constexpr int kDamageParticleLifetimeMs = 500;
  static constexpr int kTargetStaggerMs = 50;

  ActionEvent getEvent() const override { return ActionEvent::PerformShoot; }
  bmin::String shooterId;
  model::SpellTargetInfo targetInfo;

  void playSoundIfNamed(const bmin::String& soundName) {
    if (soundName.empty()) {
      return;
    }
    insertAction(state::makeAction<PlaySound>(soundName), 0);
  }

  void addUnitTarget(bmin::DynArray<model::CharacterInstance*>& targets,
                     model::CharacterInstance* character,
                     const model::CharacterInstance& shooter,
                     const model::AbilityTemplate& ability) {
    if (character == nullptr) {
      return;
    }
    if (!model::matchesSpellAllegiance(
            state->player, shooter, *character, ability.targetSelect.allegianceSelectType)) {
      return;
    }
    for (size_t i = 0; i < targets.size(); ++i) {
      if (targets[i]->id == character->id) {
        return;
      }
    }
    const auto maxTargets = ability.targetSelect.numTargetableUnits;
    if (maxTargets > 0 && static_cast<int>(targets.size()) >= maxTargets) {
      return;
    }
    targets.pushBack(character);
  }

  void collectTargets(const model::AbilityTemplate& ability,
                      model::CharacterInstance& shooter,
                      model::ActiveMap& activeMap,
                      bmin::DynArray<model::CharacterInstance*>& targets) {
    const auto targetType = ability.targetSelect.targetType;
    if (targetType == model::TargetSelectType::TARGET_SELF) {
      targets.pushBack(&shooter);
      return;
    }
    if (targetType == model::TargetSelectType::TARGET_ALLY) {
      auto atTile =
          game::findAllCharactersAt(activeMap, targetInfo.tileX, targetInfo.tileY);
      for (auto* character : atTile) {
        if (character == nullptr) {
          continue;
        }
        if (!model::matchesSpellAllegiance(
                state->player,
                shooter,
                *character,
                model::TargetAllegianceSelectType::TARGET_ALLEGIANCE_SAME_AND_SELF)) {
          continue;
        }
        targets.pushBack(character);
        const auto maxTargets = ability.targetSelect.numTargetableUnits;
        if (maxTargets > 0 && static_cast<int>(targets.size()) >= maxTargets) {
          break;
        }
      }
      return;
    }
    if (targetType == model::TargetSelectType::TARGET_UNIT) {
      if (!targetInfo.targetCharacterId.empty()) {
        addUnitTarget(targets,
                      game::findCharacterById(activeMap, targetInfo.targetCharacterId),
                      shooter,
                      ability);
      }
      if (targets.empty()) {
        auto atTile =
            game::findAllCharactersAt(activeMap, targetInfo.tileX, targetInfo.tileY);
        for (auto* character : atTile) {
          addUnitTarget(targets, character, shooter, ability);
        }
      }
      return;
    }
    if (targetType == model::TargetSelectType::TARGET_ZONE) {
      const auto zoneW = ability.targetSelect.zoneSize.x;
      const auto zoneH = ability.targetSelect.zoneSize.y;
      for (int i = 0; i < zoneW; ++i) {
        for (int j = 0; j < zoneH; ++j) {
          const auto tileX = game::zoneTileCoord(targetInfo.tileX, zoneW, i);
          const auto tileY = game::zoneTileCoord(targetInfo.tileY, zoneH, j);
          auto atTile = game::findAllCharactersAt(activeMap, tileX, tileY);
          for (auto* character : atTile) {
            if (character == nullptr) {
              continue;
            }
            auto already = false;
            for (size_t n = 0; n < targets.size(); ++n) {
              if (targets[n]->id == character->id) {
                already = true;
                break;
              }
            }
            if (!already) {
              targets.pushBack(character);
            }
          }
        }
      }
      return;
    }
    LOG(ERROR) << "PerformShoot: unhandled target type "
               << model::targetSelectTypeToString(targetType) << " for "
               << ability.name << LOG_ENDL;
  }

  const model::CharacterStats* resolveStats(model::CharacterInstance& character,
                                            model::CharacterStats& npcStatsStorage,
                                            const db::Database& database,
                                            model::CharacterStats& zeroStats) {
    auto caster = model::SpellCasterRef{};
    if (model::resolveCombatSpellCaster(
            state->player, character, database, npcStatsStorage, caster) &&
        caster.stats != nullptr) {
      return caster.stats;
    }
    return &zeroStats;
  }

  bool resolveAttackOnTarget(const model::AbilityAttack& attack,
                             const model::AbilityDepiction& depiction,
                             const model::CharacterStats& shooterStats,
                             model::CharacterInstance& target,
                             const db::Database& database,
                             int staggerMs) {
    auto targetNpcStats = model::CharacterStats{};
    auto zeroStats = model::CharacterStats{};
    const auto* defenderStats =
        resolveStats(target, targetNpcStats, database, zeroStats);
    const auto targetArmorClass = game::meleeTargetArmorClass(*defenderStats);
    const auto attackBonus = game::meleeAttackToHitBonus(shooterStats, attack, false);
    const auto hit = game::rollMeleeAttackHit(
        attack.attackClass, attackBonus, targetArmorClass, false);
    if (!hit) {
      insertAction(state::makeAction<PlaySound>("miss"), staggerMs);
      return false;
    }
    if (!depiction.dmgSound.empty()) {
      insertAction(state::makeAction<PlaySound>(depiction.dmgSound), staggerMs);
    }
    auto damage = int{0};
    if (attack.dmg.has_value()) {
      damage = game::calculateAttackDamage(attack, shooterStats).damage;
      if (damage != 0) {
        insertAction(state::makeAction<ModifyHP>(target.id, -damage), staggerMs);
      }
    }
    if (!depiction.dmgAnim.empty()) {
      insertAction(state::makeAction<WorldSpawnDamageParticle>(
                       depiction.dmgAnim,
                       bmin::toString(damage),
                       target.x,
                       target.y,
                       scaledDelayMs(kDamageParticleLifetimeMs),
                       depiction.dmgTextColor),
                   staggerMs);
    }
    return true;
  }

  void applyStatusesOnHit(const model::AbilityTemplate& ability,
                          model::CharacterInstance& target,
                          const model::CharacterStats& shooterStats,
                          const db::Database& database,
                          int staggerMs) {
    for (size_t s = 0; s < ability.statuses.size(); ++s) {
      const auto& statusApply = ability.statuses[s];
      if (!game::applyStatusEffect(target, statusApply, shooterStats, database)) {
        continue;
      }
      const auto* statusTemplate = database.findStatusEffectTemplate(
          bmin::toStringView(statusApply.statusEffect));
      if (statusTemplate == nullptr) {
        continue;
      }
      bmin::DynArray<bmin::String> onAppliedAbilities;
      game::collectStatusActionAbilities(*statusTemplate,
                                         model::StatusEventType::STATUS_EVENT_ON_APPLIED,
                                         onAppliedAbilities);
      for (size_t a = 0; a < onAppliedAbilities.size(); ++a) {
        insertAction(
            state::makeAction<PerformStatusAbility>(target.id, onAppliedAbilities[a]),
            staggerMs);
      }
    }
  }

  void spawnProjectile(const model::AbilityDepiction& depiction,
                       const model::CharacterInstance& shooter,
                       int targetTileX,
                       int targetTileY,
                       int delayMs) {
    if (depiction.projectileType == model::ProjectileType::PROJECTILE_NONE) {
      return;
    }
    auto animBase = model::projectileTypeToAnimBase(depiction.projectileType);
    if (animBase.empty()) {
      return;
    }
    if (model::projectileTypeHasFacing(depiction.projectileType)) {
      animBase += game::getProjectileFacingSuffix(targetTileX - shooter.x,
                                                  targetTileY - shooter.y);
    }
    insertAction(state::makeAction<WorldSpawnProjectile>(animBase,
                                                         static_cast<float>(shooter.x),
                                                         static_cast<float>(shooter.y),
                                                         static_cast<float>(targetTileX),
                                                         static_cast<float>(targetTileY),
                                                         delayMs,
                                                         depiction.projectilePath),
                 0);
  }

  void act() override {
    if (!state) {
      return;
    }
    auto* database = getDatabase();
    if (database == nullptr) {
      return;
    }
    auto* shooter = game::findCharacterById(state->world.activeMap, shooterId);
    if (shooter == nullptr) {
      LOG(ERROR) << "PerformShoot: no map character for " << shooterId << LOG_ENDL;
      return;
    }
    const auto resolved =
        game::resolveEquippedShootAbility(state->player, *shooter, *database);
    if (!resolved.has_value()) {
      LOG(ERROR) << "PerformShoot: no ranged weapon for " << shooterId << LOG_ENDL;
      return;
    }
    const auto& ability = resolved->ability;
    const auto distance = game::chebyshevDistance(
        shooter->x, shooter->y, targetInfo.tileX, targetInfo.tileY);
    if (distance > ability.targetSelect.range) {
      LOG(INFO) << "PerformShoot: out of range for " << ability.name << LOG_ENDL;
      return;
    }
    const auto inCombat = state->world.combat.active;
    if (inCombat && shooter->currentAp <= 0) {
      LOG(INFO) << "PerformShoot: no AP remaining for " << ability.name << LOG_ENDL;
      return;
    }
    const auto spent = game::spendShootAuxCost(state->player, *shooter, ability);
    if (!spent.ok) {
      LOG(INFO) << "PerformShoot: aux cost unpaid for " << ability.name << LOG_ENDL;
      return;
    }

    if (inCombat) {
      insertAction(state::makeAction<ModifyAP>(shooterId, -model::COMBAT_SHOOT_COST), 0);
    }
    if (spent.hpCost > 0) {
      insertAction(state::makeAction<ModifyHP>(shooterId, -spent.hpCost), 0);
    }

    model::updateCharacterFacingToward(*shooter, targetInfo.tileX, targetInfo.tileY);
    insertAction(state::makeAction<CharacterSetSpriteIndexOffset>(shooterId, 1), 0);
    playSoundIfNamed(ability.depiction.startSound);

    auto delayMs = scaledDelayMs(300);
    if (ability.depiction.projectileType != model::ProjectileType::PROJECTILE_NONE) {
      delayMs = scaledDelayMs(
          game::getProjectileTravelDurationMs(ability.depiction.projectilePath));
    }
    spawnProjectile(
        ability.depiction, *shooter, targetInfo.tileX, targetInfo.tileY, delayMs);
    insertAction(nullptr, delayMs);

    const auto shooterX = shooter->x;
    const auto shooterY = shooter->y;
    const auto targetX = targetInfo.tileX;
    const auto targetY = targetInfo.tileY;
    const auto shouldPanHit =
        state->world.combat.active && (targetX != shooterX || targetY != shooterY) &&
        state->world.camera.viewW > 0 && state->world.camera.viewH > 0;
    const auto turnEnds =
        inCombat && shooter->currentAp - model::COMBAT_SHOOT_COST <= 0;
    if (shouldPanHit) {
      const auto hitCam = game::computeCameraFollow(
          targetX, targetY, state->world.camera.viewW, state->world.camera.viewH);
      insertAction(state::makeAction<WorldSetCameraMode>(model::CameraMode::Controlled), 0);
      insertAction(state::makeAction<WorldSetCamera>(hitCam.camX, hitCam.camY), 0);
    }

    bmin::DynArray<model::CharacterInstance*> targets;
    collectTargets(ability, *shooter, state->world.activeMap, targets);

    auto shooterNpcStats = model::CharacterStats{};
    auto zeroStats = model::CharacterStats{};
    const auto* shooterStats = resolveStats(*shooter, shooterNpcStats, *database, zeroStats);

    if (targets.empty() || ability.attacks.empty()) {
      insertAction(state::makeAction<PlaySound>("miss"), 0);
      if (targets.empty()) {
        LOG(INFO) << "Missed!" << LOG_ENDL;
      }
    }

    for (size_t i = 0; i < targets.size(); ++i) {
      auto* target = targets[i];
      if (target == nullptr) {
        continue;
      }
      const auto staggerMs = static_cast<int>(i) * scaledDelayMs(kTargetStaggerMs);
      auto targetHit = false;
      for (size_t a = 0; a < ability.attacks.size(); ++a) {
        targetHit = resolveAttackOnTarget(ability.attacks[a],
                                          ability.depiction,
                                          *shooterStats,
                                          *target,
                                          *database,
                                          staggerMs) ||
                    targetHit;
      }
      if (targetHit) {
        applyStatusesOnHit(ability, *target, *shooterStats, *database, staggerMs);
      }
    }

    if (shouldPanHit) {
      const auto cameraHoldMs = std::max(350, scaledDelayMs(250));
      insertAction(nullptr, cameraHoldMs);
    }
    if (shouldPanHit && !turnEnds) {
      const auto returnCam = game::computeCameraFollow(
          shooterX, shooterY, state->world.camera.viewW, state->world.camera.viewH);
      insertAction(state::makeAction<WorldSetCameraMode>(model::CameraMode::Follow), 0);
      insertAction(state::makeAction<WorldSetCamera>(returnCam.camX, returnCam.camY), 0);
    }

    insertAction(state::makeAction<CharacterSetSpriteIndexOffset>(shooterId, 0), 0);
    insertAction(state::makeAction<WorldSetActionMode>(model::WorldActionMode::NONE), 0);
  }

public:
  PerformShoot(const bmin::String& shooterId, const model::SpellTargetInfo& targetInfo)
      : shooterId(shooterId), targetInfo(targetInfo) {}
};

} // namespace actions

} // namespace state
