#pragma once

#include "bmin/StringInterop.h"
#include "game/combat/Damage.h"
#include "game/combat/SpellRules.h"
#include "game/combat/StatusRules.h"
#include "game/combat/projectileHelpers.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/TileDistance.h"
#include "model/Combat.h"
#include "model/instances/Player.h"
#include "model/templates/AbilityTypes.h"
#include "sdl2w/Logger.h"
#include "state/AbstractAction.hpp"
#include "actions/combat/CharacterSetSpriteIndexOffset.hpp"
#include "actions/combat/ModifyAP.hpp"
#include "actions/combat/ModifyHP.hpp"
#include "actions/combat/PerformStatusAbility.hpp"
#include "actions/general/PlaySound.hpp"
#include "actions/world/WorldSetActionMode.hpp"
#include "actions/world/WorldSpawnDamageParticle.hpp"
#include "actions/world/WorldSpawnProjectile.hpp"

namespace state {

namespace actions {

class PerformSpellCast : public AbstractAction {
  ActionEvent getEvent() const override { return ActionEvent::PerformSpellCast; }
  bmin::String casterId;
  bmin::String spellId;
  model::SpellTargetInfo spellTargetInfo;

  void playSoundIfNamed(const bmin::String& soundName) {
    if (soundName.empty()) {
      return;
    }
    insertAction(state::makeAction<PlaySound>(soundName), 0);
  }

  void applySpellToCharacters(const model::AbilityTemplate& ability,
                              model::CharacterInstance& caster,
                              const model::CharacterStats& casterStats,
                              const bmin::DynArray<model::CharacterInstance*>& targets,
                              int targetTileX,
                              int targetTileY) {
    const auto& depiction = ability.depiction;
    auto casterX = caster.x;
    auto casterY = caster.y;

    bmin::DynArray<int> hpDeltaToCharacters;
    for (size_t i = 0; i < targets.size(); i++) {
      auto* ch = targets[i];
      const auto rolledHpDelta =
          game::calculateAbilityTemplateHpDelta(ability, casterStats);
      const auto currentHp = model::getCharacterHp(state->player, *ch);
      const auto hpDelta = model::appliedHpDelta(currentHp, ch->maxHp, rolledHpDelta);
      hpDeltaToCharacters.pushBack(hpDelta);
      LOG(DEBUG) << "PerformSpellCast: " << spellId << " vs "
                 << model::formatCharacterLogLabel(state->world.activeMap, ch->id)
                 << " hpDelta=" << hpDelta << " (rolled=" << rolledHpDelta << ")"
                 << LOG_ENDL;
    }

    model::updateCharacterFacingToward(caster, targetTileX, targetTileY);

    insertAction(state::makeAction<CharacterSetSpriteIndexOffset>(casterId, 1), 0);
    playSoundIfNamed(depiction.startSound);

    int delayMs = 300;
    if (depiction.projectileType != model::ProjectileType::PROJECTILE_NONE) {
      delayMs = game::getProjectileTravelDurationMs(depiction.projectilePath);
      auto animBase = model::projectileTypeToAnimBase(depiction.projectileType);
      if (!animBase.empty()) {
        if (model::projectileTypeHasFacing(depiction.projectileType)) {
          animBase += game::getProjectileFacingSuffix(targetTileX - caster.x,
                                                      targetTileY - caster.y);
        }
        insertAction(state::makeAction<WorldSpawnProjectile>(animBase,
                                              static_cast<float>(casterX),
                                              static_cast<float>(casterY),
                                              static_cast<float>(targetTileX),
                                              static_cast<float>(targetTileY),
                                              delayMs,
                                              depiction.projectilePath),
                     0);
      }
    }

    insertAction(nullptr, delayMs);

    const int damageParticleLifetimeMs = 500;
    auto spawnedSpellParticle = false;
    for (size_t i = 0; i < targets.size(); i++) {
      auto* ch = targets[i];
      const auto hpDelta = hpDeltaToCharacters[i];
      if (hpDelta != 0) {
        insertAction(state::makeAction<ModifyHP>(ch->id, hpDelta), i * 50);
      }
      if (hpDelta != 0 && !depiction.dmgAnim.empty()) {
        const auto particleText =
            hpDelta > 0 ? bmin::toString(hpDelta) : bmin::toString(-hpDelta);
        insertAction(state::makeAction<WorldSpawnDamageParticle>(depiction.dmgAnim,
                                                  particleText,
                                                  ch->x,
                                                  ch->y,
                                                  damageParticleLifetimeMs,
                                                  depiction.dmgTextColor),
                     i * 50);
        spawnedSpellParticle = true;
      }
    }

    if (targets.size() > 0) {
      if (spawnedSpellParticle) {
        playSoundIfNamed(depiction.dmgSound);
        insertAction(nullptr, damageParticleLifetimeMs);
      }
    } else {
      LOG(INFO) << "Missed!" << LOG_ENDL;
    }

    auto* database = getDatabase();
    if (database == nullptr || ability.statuses.empty()) {
      return;
    }
    for (size_t i = 0; i < targets.size(); i++) {
      auto* ch = targets[i];
      for (const auto& statusApply : ability.statuses) {
        if (!game::applyStatusEffect(*ch, statusApply, casterStats, *database)) {
          continue;
        }
        const auto* statusTemplate =
            database->findStatusEffectTemplate(bmin::toStringView(statusApply.statusEffect));
        if (statusTemplate == nullptr) {
          continue;
        }
        bmin::DynArray<bmin::String> onAppliedAbilities;
        game::collectStatusActionAbilities(*statusTemplate,
                                           model::StatusEventType::STATUS_EVENT_ON_APPLIED,
                                           onAppliedAbilities);
        for (size_t a = 0; a < onAppliedAbilities.size(); a++) {
          insertAction(state::makeAction<PerformStatusAbility>(ch->id, onAppliedAbilities[a]),
                       static_cast<int>(i * 50 + a * 50));
        }
      }
    }
  }

  void doZoneSpell(const model::AbilityTemplate& ability,
                   model::CharacterInstance& caster,
                   const model::CharacterStats& casterStats,
                   game::ActiveMapOrchestrator& orch) {
    auto targetTileX = spellTargetInfo.tileX;
    auto targetTileY = spellTargetInfo.tileY;
    auto zoneW = ability.targetSelect.zoneSize.x;
    auto zoneH = ability.targetSelect.zoneSize.y;

    bmin::DynArray<model::CharacterInstance*> charactersInZone;
    for (int i = 0; i < zoneW; ++i) {
      for (int j = 0; j < zoneH; ++j) {
        const auto tileX = targetTileX + i - zoneW / 2;
        const auto tileY = targetTileY + j - zoneH / 2;
        auto chAtTile = orch.findAllCharactersAt(tileX, tileY);
        for (auto ch : chAtTile) {
          charactersInZone.pushBack(ch);
        }
      }
    }

    applySpellToCharacters(
        ability, caster, casterStats, charactersInZone, targetTileX, targetTileY);
  }

  bool matchesSpellAllegiance(const model::CharacterInstance& caster,
                              const model::CharacterInstance& target,
                              model::TargetAllegianceSelectType allegiance) {
    const auto isSelf = target.id == caster.id;
    const auto sameTeam = model::isCharacterAlly(state->player, caster) ==
                          model::isCharacterAlly(state->player, target);
    switch (allegiance) {
    case model::TargetAllegianceSelectType::TARGET_ALLEGIANCE_OTHER:
      return !isSelf && !sameTeam;
    case model::TargetAllegianceSelectType::TARGET_ALLEGIANCE_SAME:
      return !isSelf && sameTeam;
    case model::TargetAllegianceSelectType::TARGET_ALLEGIANCE_SAME_AND_SELF:
      return sameTeam;
    case model::TargetAllegianceSelectType::TARGET_ALLEGIANCE_ALL:
      return !isSelf;
    case model::TargetAllegianceSelectType::TARGET_ALLEGIANCE_ALL_AND_SELF:
      return true;
    }
    return false;
  }

  void addUnitSpellTarget(bmin::DynArray<model::CharacterInstance*>& targets,
                          model::CharacterInstance* ch,
                          const model::CharacterInstance& caster,
                          const model::AbilityTemplate& ability) {
    if (ch == nullptr) {
      return;
    }
    if (!matchesSpellAllegiance(caster, *ch, ability.targetSelect.allegianceSelectType)) {
      return;
    }
    for (size_t i = 0; i < targets.size(); i++) {
      if (targets[i]->id == ch->id) {
        return;
      }
    }
    const auto maxTargets = ability.targetSelect.numTargetableUnits;
    if (maxTargets > 0 && static_cast<int>(targets.size()) >= maxTargets) {
      return;
    }
    targets.pushBack(ch);
  }

  void doUnitSpell(const model::AbilityTemplate& ability,
                   model::CharacterInstance& caster,
                   const model::CharacterStats& casterStats,
                   game::ActiveMapOrchestrator& orch) {
    auto targetTileX = spellTargetInfo.tileX;
    auto targetTileY = spellTargetInfo.tileY;

    bmin::DynArray<model::CharacterInstance*> targets;
    if (!spellTargetInfo.targetCharacterId.empty()) {
      addUnitSpellTarget(targets,
                         orch.findCharacterById(spellTargetInfo.targetCharacterId),
                         caster,
                         ability);
    }
    if (targets.empty()) {
      auto atTile = orch.findAllCharactersAt(targetTileX, targetTileY);
      for (auto* ch : atTile) {
        addUnitSpellTarget(targets, ch, caster, ability);
      }
    }

    applySpellToCharacters(
        ability, caster, casterStats, targets, targetTileX, targetTileY);
  }

  void doAllySpell(const model::AbilityTemplate& ability,
                   model::CharacterInstance& caster,
                   const model::CharacterStats& casterStats,
                   game::ActiveMapOrchestrator& orch) {
    auto* target = orch.findCharacterById(spellTargetInfo.targetCharacterId);
    if (target == nullptr) {
      LOG(ERROR) << "PerformSpellCast: TARGET_ALLY missing map character "
                 << spellTargetInfo.targetCharacterId << LOG_ENDL;
      return;
    }
    if (!model::isPartyMember(state->player, target->id)) {
      LOG(ERROR) << "PerformSpellCast: TARGET_ALLY is not a party member "
                 << target->id << LOG_ENDL;
      return;
    }

    bmin::DynArray<model::CharacterInstance*> targets;
    targets.pushBack(target);
    applySpellToCharacters(
        ability, caster, casterStats, targets, target->x, target->y);
  }

  bool verifySpellCanBeCast(const model::CharacterInstance& caster,
                            const model::AbilityTemplate& ability) {
    if (ability.costType == model::AbilityCostType::ABILITY_COST_MANA) {
      if (caster.currentMp < ability.costValue) {
        return false;
      }
    }

    return true;
  }

  void act() override {
    if (!state) {
      return;
    }

    // gather data

    auto database = getDatabase();
    if (database == nullptr) {
      return;
    }
    game::ActiveMapOrchestrator orch(state->world.activeMap, state->mapInstances, getDatabase());
    orch.fetchMapGrid(state->world.activeMap.gridId);

    auto caster = orch.findCharacterById(casterId);
    if (caster == nullptr) {
      LOG(ERROR) << "PerformSpellCast: no map character for caster " << casterId
                 << LOG_ENDL;
      return;
    }
    auto spell = database->findSpellTemplate(bmin::toStringView(spellId));
    if (spell == nullptr) {
      LOG(ERROR) << "PerformSpellCast: no spell for " << spellId << LOG_ENDL;
      return;
    }
    const auto ability =
        database->findAbilityTemplate(bmin::toStringView(spell->abilityName));
    if (ability == nullptr) {
      LOG(ERROR) << "PerformSpellCast: missing ability " << spell->abilityName
                 << LOG_ENDL;
      return;
    }

    // verify

    // if (!verifySpellCanBeCast(*caster, *ability)) {
    //   LOG(ERROR) << "PerformSpellCast: spell not allowed to be cast" << LOG_ENDL;
    //   return;
    // }

    // do

    auto npcStats = model::CharacterStats{};
    auto casterRef = model::SpellCasterRef{};
    const model::CharacterStats* casterStats = nullptr;
    if (model::resolveCombatSpellCaster(
            state->player, *caster, *database, npcStats, casterRef) &&
        casterRef.stats != nullptr) {
      casterStats = casterRef.stats;
    }
    auto zeroStats = model::CharacterStats{};
    if (casterStats == nullptr) {
      casterStats = &zeroStats;
    }

    if (ability->apCost != 0) {
      insertAction(state::makeAction<ModifyAP>(casterId, -ability->apCost), 0);
    }

    if (ability->targetSelect.targetType == model::TargetSelectType::TARGET_ZONE) {
      doZoneSpell(*ability, *caster, *casterStats, orch);
    } else if (ability->targetSelect.targetType == model::TargetSelectType::TARGET_ALLY) {
      doAllySpell(*ability, *caster, *casterStats, orch);
    } else if (ability->targetSelect.targetType == model::TargetSelectType::TARGET_UNIT) {
      doUnitSpell(*ability, *caster, *casterStats, orch);
    } else {
      LOG(ERROR) << "PerformSpellCast: unhandled target type "
                 << model::targetSelectTypeToString(ability->targetSelect.targetType)
                 << " for " << spellId << LOG_ENDL;
    }

    insertAction(state::makeAction<CharacterSetSpriteIndexOffset>(casterId, 0), 0);
    insertAction(state::makeAction<WorldSetActionMode>(model::WorldActionMode::NONE), 0);
  }

public:
  explicit PerformSpellCast(const bmin::String& casterId,
                            const bmin::String& spellId,
                            const model::SpellTargetInfo& spellTargetInfo)
      : casterId(casterId), spellId(spellId), spellTargetInfo(spellTargetInfo) {}
};

} // namespace actions

} // namespace state
