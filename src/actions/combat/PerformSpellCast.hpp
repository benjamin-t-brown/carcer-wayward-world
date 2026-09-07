#pragma once

#include "bmin/StringInterop.h"
#include "game/combat/Damage.h"
#include "game/combat/SpellRules.h"
#include "game/combat/projectileHelpers.h"
#include "game/map/ActiveMapOrchestrator.h"
#include "game/map/TileDistance.h"
#include "model/Combat.h"
#include "model/instances/Player.h"
#include "model/templates/AbilityTypes.h"
#include "sdl2w/Logger.h"
#include "actions/combat/ActionBase.hpp"
#include "actions/combat/CharacterSetSpriteIndexOffset.hpp"
#include "actions/combat/ModifyAP.hpp"
#include "actions/combat/ModifyHP.hpp"
#include "actions/general/PlaySound.hpp"
#include "actions/world/WorldSetActionMode.hpp"
#include "actions/world/WorldSpawnDamageParticle.hpp"
#include "actions/world/WorldSpawnProjectile.hpp"

namespace state {

namespace actions {

class PerformSpellCast : public CombatAction {
  ActionEvent getEvent() const override { return ActionEvent::PerformSpellCast; }
  bmin::String casterId;
  bmin::String spellId;
  model::SpellTargetInfo spellTargetInfo;

  void doZoneSpell(const model::AbilityTemplate& ability,
                   model::CharacterInstance& caster,
                   game::ActiveMapOrchestrator& orch) {
    const auto& depiction = ability.depiction;
    auto casterX = caster.x;
    auto casterY = caster.y;
    auto targetTileX = spellTargetInfo.tileX;
    auto targetTileY = spellTargetInfo.tileY;
    auto zoneW = ability.targetSelect.zoneSize.x;
    auto zoneH = ability.targetSelect.zoneSize.y;
    // ignore attacks/restores

    // TODO status effects

    bmin::DynArray<model::CharacterInstance*> charactersInZone;
    bmin::DynArray<int> damageDealtToCharactersInZone;

    for (int i = 0; i < zoneW; ++i) {
      for (int j = 0; j < zoneH; ++j) {
        const auto tileX = targetTileX + i - zoneW / 2;
        const auto tileY = targetTileY + j - zoneH / 2;
        auto chAtTile = orch.findAllCharactersAt(tileX, tileY);
        for (auto ch : chAtTile) {
          charactersInZone.pushBack(ch);
          damageDealtToCharactersInZone.pushBack(0);
        }
      }
    }

    for (size_t i = 0; i < charactersInZone.size(); i++) {
      auto ch = charactersInZone[i];
      auto damageDealt = 0;
      for (const auto& damage : ability.damages) {
        auto result = game::calculateAbilityDamage(damage, caster, *ch);
        damageDealt += result.damage;
      }
      damageDealtToCharactersInZone[i] = damageDealt;
    }

    model::updateCharacterFacingToward(
        caster, spellTargetInfo.tileX, spellTargetInfo.tileY);

    insertAction(new CharacterSetSpriteIndexOffset(casterId, 1), 0);
    insertAction(new PlaySound(depiction.startSound), 0);

    int delayMs = 300;
    if (depiction.projectileType != model::ProjectileType::PROJECTILE_NONE) {
      delayMs = game::getProjectileTravelDurationMs(depiction.projectilePath);
      auto animBase = model::projectileTypeToAnimBase(depiction.projectileType);
      if (!animBase.empty()) {
        if (model::projectileTypeHasFacing(depiction.projectileType)) {
          animBase += game::getProjectileFacingSuffix(spellTargetInfo.tileX - caster.x,
                                                      spellTargetInfo.tileY - caster.y);
        }
        insertAction(new WorldSpawnProjectile(animBase,
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
    if (!depiction.dmgAnim.empty()) {
      for (size_t i = 0; i < charactersInZone.size(); i++) {
        auto ch = charactersInZone[i];
        auto chX = ch->x;
        auto chY = ch->y;
        auto damageDealt = damageDealtToCharactersInZone[i];
        insertAction(new WorldSpawnDamageParticle(depiction.dmgAnim,
                                                  bmin::toString(damageDealt),
                                                  chX,
                                                  chY,
                                                  damageParticleLifetimeMs),
                     i * 50);
        insertAction(new ModifyHP(casterId, damageDealt), i * 50);
      }
    }

    if (charactersInZone.size() > 0) {
      insertAction(new PlaySound(depiction.dmgSound), 0);
      insertAction(nullptr, damageParticleLifetimeMs);
    } else {
      LOG(INFO) << "Missed!" << LOG_ENDL;
    }
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

    if (ability->apCost != 0) {
      insertAction(new ModifyAP(casterId, -ability->apCost), 0);
    }

    if (ability->targetSelect.targetType == model::TargetSelectType::TARGET_ZONE) {
      doZoneSpell(*ability, *caster, orch);
    }

    insertAction(new CharacterSetSpriteIndexOffset(casterId, 0), 0);
    insertAction(new WorldSetActionMode(model::WorldActionMode::NONE), 0);
  }

public:
  explicit PerformSpellCast(const bmin::String& casterId,
                            const bmin::String& spellId,
                            const model::SpellTargetInfo& spellTargetInfo)
      : casterId(casterId), spellId(spellId), spellTargetInfo(spellTargetInfo) {}
};

} // namespace actions

} // namespace state
