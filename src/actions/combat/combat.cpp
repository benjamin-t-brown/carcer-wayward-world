module;
#include <cstddef>
#include <cstdlib>
#include <utility>

module carcer.actions.combat;
import carcer.actions.general;
import carcer.actions.world_effects;
import sdl2w;
import bmin.string_interop;
import carcer.data;
import carcer.model;
import carcer.game.map;
import carcer.game.combat;
#include "macros.h"

namespace state {

namespace actions {

void PerformMeleeAttack::act() {
  if (!state) {
    return;
  }
  auto* database = getDatabase();
  if (database == nullptr) {
    return;
  }

  game::ActiveMapOrchestrator orch;
  auto* attacker = orch.findCharacterById(attackerId);
  auto* victim = orch.findCharacterById(victimId);
  if (attacker == nullptr || victim == nullptr) {
    return;
  }

  model::updateCharacterFacingToward(*attacker, victim->x, victim->y);

  insertAction(new CharacterSetSpriteIndexOffset(attackerId, 1), 0);

  const auto hit = (std::rand() % 100) < model::COMBAT_HIT_CHANCE_PERCENT;
  if (hit) {
    insertAction(new PlaySound("hit_punch1"), 0);
    insertAction(nullptr, 75);
    insertAction(new ModifyHP(victimId, -model::COMBAT_MELEE_DAMAGE), 0);
    insertAction(
        new WorldSpawnDamageParticle("splash_attack",
                                     bmin::toString(model::COMBAT_MELEE_DAMAGE),
                                     victim->x,
                                     victim->y,
                                     500),
        0);
    insertAction(nullptr, 500);
  } else {
    insertAction(new PlaySound("miss"), 0);
    insertAction(nullptr, 300);
  }
  insertAction(new CharacterSetSpriteIndexOffset(attackerId, 0), 0);
}

void PerformSpellCast::doZoneSpell(const model::AbilityTemplate& ability,
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

void PerformSpellCast::act() {
  if (!state) {
    return;
  }

  // gather data

  auto database = getDatabase();
  if (database == nullptr) {
    return;
  }
  game::ActiveMapOrchestrator orch;
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

} // namespace actions

} // namespace state
