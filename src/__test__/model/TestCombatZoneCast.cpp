#include "db/Database.h"
#include "game/map/MapPersistence.h"
#include "game/combat/SpellRules.h"
#include "game/combat/Damage.h"
#include "model/Combat.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/Abilities.hpp"
#include "model/templates/AbilityTypes.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/MapGrids.hpp"
#include "model/templates/RuneTypes.h"
#include "model/templates/Spells.hpp"
#include "model/templates/Tileset.hpp"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "actions/combat/DoCombatAction.hpp"
#include "actions/combat/PerformSpellCast.hpp"
#include "actions/combat/StartCombat.hpp"
#include "bmin/String.h"

namespace {

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertTrue(bool cond, const char* label) {
  if (!cond) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

void addWalkableTileset(db::Database& database) {
  auto tileset = model::TilesetTemplate{};
  tileset.name = "test_terrain";
  auto meta = model::TileMetadata{};
  meta.id = 0;
  meta.isWalkable = true;
  meta.isSeeThrough = true;
  tileset.tiles.pushBack(meta);
  database.addTilesetTemplate(tileset);
}

void pumpActions(state::StateManager& stateManager, int maxMs = 4000) {
  for (int elapsed = 0; elapsed < maxMs; elapsed += 50) {
    stateManager.update(50);
    const auto& actions = stateManager.getActionData();
    if (actions.sequentialActions.empty() && actions.sequentialActionsNext.empty() &&
        actions.insertActions.empty()) {
      return;
    }
  }
}

void addFlameAbilityAndSpell(db::Database& database) {
  model::AbilityTemplate ability;
  ability.name = "SPELL_FLAME_TEST";
  ability.type = model::AbilityType::ABILITY_SPELL;
  ability.targetSelect.targetType = model::TargetSelectType::TARGET_ZONE;
  ability.targetSelect.zoneSize = {.x = 1, .y = 1};
  ability.targetSelect.range = 10;
  ability.apCost = 6;
  ability.costType = model::AbilityCostType::ABILITY_COST_MANA;
  ability.costValue = 2;
  ability.depiction.dmgAnim = "expl_fire";
  ability.depiction.startSound = "cast_flame";
  ability.depiction.dmgSound = "hit_fireball";
  ability.depiction.projectileType = model::ProjectileType::PROJECTILE_NONE;
  ability.depiction.projectilePath = model::ProjectilePath::PROJECTILE_PATH_NONE;

  model::AbilityDamage dmg;
  dmg.damageType = model::DamageType::DAMAGE_TYPE_HEAT;
  dmg.dmgDice = {model::Dice::D0};
  dmg.dmgBonus = 5;
  dmg.dmgStat = model::StatsEnum::STAT_MND;
  dmg.dmgStatMult = 0.f;
  ability.damages.pushBack(dmg);
  database.addAbilityTemplate(ability);

  model::SpellTemplate spell;
  spell.name = "FLAME_TEST";
  spell.abilityName = "SPELL_FLAME_TEST";
  spell.requiredRunes = {model::SpellRuneRequirement{model::RuneType::HEAT, 1}};
  database.addSpellTemplate(spell);
}

void setupTinyMap(db::Database& database, state::State& state) {
  addWalkableTileset(database);
  auto map = model::MapInstance{};
  map.id = "zone_cast_map";
  map.templateName = "zone_cast_map";
  map.width = 5;
  map.height = 5;
  map.spriteWidth = 28;
  map.spriteHeight = 32;
  auto layer = bmin::DynArray<model::TileInstance>{};
  for (auto y = 0; y < 5; y++) {
    for (auto x = 0; x < 5; x++) {
      auto tile = model::TileInstance{};
      tile.x = x;
      tile.y = y;
      tile.tilesetName = "test_terrain";
      layer.pushBack(tile);
    }
  }
  model::mapLayerAt(model::mapInstanceTiles(map), 0) = std::move(layer);
  state.mapInstances[map.templateName] = std::move(map);

  model::MapGridTemplate grid;
  grid.name = "zone_cast_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = 5;
  grid.mapHeight = 5;
  grid.cells = {{"zone_cast_map"}};
  database.addMapGridTemplate(grid);
  state.world.activeMap.gridId = "zone_cast_grid";
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestCombatZoneCast" << LOG_ENDL;
  auto ok = true;

  db::Database database;
  addFlameAbilityAndSpell(database);
  state::DatabaseInterface::setDatabase(&database);

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);
  auto& state = stateManager.getState();
  setupTinyMap(database, state);

  model::CharacterPlayer ally;
  ally.instanceId = "ally-1";
  ally.name = "Hero";
  ally.templateName = "hero";
  ally.currentHp = 100;
  ally.currentMp = 10;
  ally.knownSpells = {"FLAME_TEST"};
  ally.equippedRunes = {model::RuneType::HEAT};
  ally.stats.generic.mnd = 6;
  state.player.party.pushBack(ally);

  auto allyInst = model::CharacterInstance{};
  allyInst.id = "ally-1";
  allyInst.name = "Hero";
  allyInst.templateName = "hero";
  allyInst.x = 1;
  allyInst.y = 1;
  allyInst.currentHp = 100;
  allyInst.maxHp = 100;
  allyInst.hpInitialized = true;
  state.world.activeMap.characters.pushBack(std::move(allyInst));

  auto enemy = model::CharacterInstance{};
  enemy.id = "enemy-1";
  enemy.name = "Goblin";
  enemy.templateName = "goblin";
  enemy.type = model::CharacterTemplateType::ENEMY;
  enemy.x = 2;
  enemy.y = 1;
  enemy.currentHp = 20;
  enemy.maxHp = 20;
  enemy.hpInitialized = true;
  state.world.activeMap.characters.pushBack(std::move(enemy));

  {
    auto& caster = state.player.party[0];
    caster.equippedRunes.clear();
    const auto fail = model::castCombatZoneSpell(
        caster, "FLAME_TEST", 2, 1, state.world, database);
    ok = assertTrue(fail.result == model::CastSpellResult::CANNOT_CAST,
                    "fail without HEAT") &&
         ok;
    ok = assertEqual(caster.currentMp, 10, "mp unchanged without runes") && ok;
  }

  {
    auto& caster = state.player.party[0];
    caster.equippedRunes = {model::RuneType::HEAT};
    caster.currentMp = 0;
    const auto fail =
        model::castCombatZoneSpell(caster, "FLAME_TEST", 2, 1, state.world, database);
    ok = assertTrue(fail.result == model::CastSpellResult::CANNOT_CAST,
                    "fail without mana") &&
         ok;
  }

  {
    auto& caster = state.player.party[0];
    caster.equippedRunes = {model::RuneType::HEAT};
    caster.currentMp = 10;
    const auto success =
        model::castCombatZoneSpell(caster, "FLAME_TEST", 2, 1, state.world, database);
    ok = assertTrue(success.result == model::CastSpellResult::CAST, "zone cast ok") &&
         ok;
    ok = assertEqual(caster.currentMp, 8, "spent 2 mp") && ok;
    ok = assertEqual(success.casterApCost, 6, "apCost 6") && ok;
    ok = assertEqual(static_cast<int>(success.hits.size()), 1, "one hit") && ok;
    if (!success.hits.empty()) {
      ok = assertTrue(success.hits[0].characterId == "enemy-1", "hit enemy") && ok;
      ok = assertEqual(success.hits[0].hpDelta, -5, "damages[] D0+5") && ok;
    }
  }

  {
    model::AbilityTemplate mindFlame;
    mindFlame.name = "SPELL_FLAME_MND";
    mindFlame.type = model::AbilityType::ABILITY_SPELL;
    mindFlame.targetSelect.targetType = model::TargetSelectType::TARGET_ZONE;
    mindFlame.targetSelect.zoneSize = {.x = 1, .y = 1};
    mindFlame.apCost = 0;
    mindFlame.costType = model::AbilityCostType::ABILITY_COST_MANA;
    mindFlame.costValue = 0;
    model::AbilityDamage dmg;
    dmg.damageType = model::DamageType::DAMAGE_TYPE_HEAT;
    dmg.dmgDice = {model::Dice::D0};
    dmg.dmgBonus = 0;
    dmg.dmgStat = model::StatsEnum::STAT_MND;
    dmg.dmgStatMult = 1.f;
    mindFlame.damages.pushBack(dmg);
    database.addAbilityTemplate(mindFlame);

    model::SpellTemplate spell;
    spell.name = "FLAME_MND";
    spell.abilityName = "SPELL_FLAME_MND";
    spell.requiredRunes = {model::SpellRuneRequirement{model::RuneType::HEAT, 1}};
    database.addSpellTemplate(spell);

    auto& caster = state.player.party[0];
    caster.knownSpells.pushBack("FLAME_MND");
    caster.currentMp = 10;
    const auto scaled =
        model::castCombatZoneSpell(caster, "FLAME_MND", 2, 1, state.world, database);
    ok = assertTrue(scaled.result == model::CastSpellResult::CAST, "mnd zone cast") &&
         ok;
    ok = assertEqual(static_cast<int>(scaled.hits.size()), 1, "mnd one hit") && ok;
    if (!scaled.hits.empty()) {
      ok = assertEqual(scaled.hits[0].hpDelta, -6, "damages[] uses party MND") && ok;
    }
  }

  {
    auto& caster = state.player.party[0];
    caster.currentMp = 10;
    const auto empty =
        model::castCombatZoneSpell(caster, "FLAME_TEST", 4, 4, state.world, database);
    ok = assertTrue(empty.result == model::CastSpellResult::CAST, "empty zone cast") &&
         ok;
    ok = assertEqual(static_cast<int>(empty.hits.size()), 0, "empty zone no hits") &&
         ok;
    ok = assertEqual(caster.currentMp, 8, "empty zone still spends mp") && ok;
  }

  // NPC caster: mana only via map CharacterInstance (no party sheet / runes).
  {
    model::CharacterTemplate npcTmpl;
    npcTmpl.name = "npc_caster";
    npcTmpl.combat.mp = 10;
    database.addCharacterTemplate(npcTmpl);

    model::CharacterInstance npcCaster;
    npcCaster.id = "enemy-caster";
    npcCaster.templateName = "npc_caster";
    npcCaster.x = 0;
    npcCaster.y = 0;
    npcCaster.currentMp = 10;
    npcCaster.maxMp = 10;
    npcCaster.mpInitialized = true;
    state.world.activeMap.characters.pushBack(npcCaster);
    auto& npcOnMap =
        state.world.activeMap.characters[state.world.activeMap.characters.size() - 1];

    const auto npcOk = model::castCombatZoneSpell(
        state.player, npcOnMap, "FLAME_TEST", 2, 1, state.world, database);
    ok = assertTrue(npcOk.result == model::CastSpellResult::CAST, "npc mana-only cast") &&
         ok;
    ok = assertEqual(npcOnMap.currentMp, 8, "npc spent 2 mp") && ok;

    npcOnMap.currentMp = 0;
    const auto npcFail = model::castCombatZoneSpell(
        state.player, npcOnMap, "FLAME_TEST", 2, 1, state.world, database);
    ok = assertTrue(npcFail.result == model::CastSpellResult::CANNOT_CAST,
                    "npc fails without mana") &&
         ok;
  }

  {
    model::CharacterInstance* enemy = nullptr;
    for (auto& ch : state.world.activeMap.characters) {
      if (ch.id == "enemy-1") {
        enemy = &ch;
        break;
      }
    }
    ok = assertTrue(enemy != nullptr, "enemy for PerformSpellCast") && ok;
    const auto hpBefore =
        enemy ? model::getCharacterHp(state.player, *enemy) : 0;
    const auto casterHpBefore = state.player.party[0].currentHp;
    ok = assertEqual(state.world.activeMap.characters[0].stats.generic.mnd,
                     0,
                     "map instance MND is 0") &&
         ok;

    model::SpellTargetInfo target;
    target.tileX = 2;
    target.tileY = 1;
    stateManager.enqueueAction(
        state::makeAction<state::actions::PerformSpellCast>("ally-1", "FLAME_MND", target),
        0);
    pumpActions(stateManager);

    enemy = nullptr;
    for (auto& ch : state.world.activeMap.characters) {
      if (ch.id == "enemy-1") {
        enemy = &ch;
        break;
      }
    }
    if (enemy) {
      ok = assertEqual(model::getCharacterHp(state.player, *enemy),
                       hpBefore - 6,
                       "PerformSpellCast uses ability damages[] + party MND") &&
           ok;
    }
    ok = assertEqual(state.player.party[0].currentHp,
                     casterHpBefore,
                     "spell damage applies to the target, not the caster") &&
         ok;
  }

  {
    model::AbilityTemplate singeAbility;
    singeAbility.name = "SPELL_SINGE_TEST";
    singeAbility.type = model::AbilityType::ABILITY_SPELL;
    singeAbility.targetSelect.targetType = model::TargetSelectType::TARGET_UNIT;
    singeAbility.targetSelect.allegianceSelectType =
        model::TargetAllegianceSelectType::TARGET_ALLEGIANCE_OTHER;
    singeAbility.targetSelect.numTargetableUnits = 1;
    singeAbility.targetSelect.zoneSize = {.x = 1, .y = 1};
    singeAbility.targetSelect.range = 10;
    singeAbility.apCost = 0;
    singeAbility.depiction.dmgAnim = "splash_fire";
    singeAbility.depiction.dmgTextColor = "#111111";
    singeAbility.depiction.projectileType = model::ProjectileType::PROJECTILE_NONE;
    singeAbility.depiction.projectilePath = model::ProjectilePath::PROJECTILE_PATH_NONE;
    model::AbilityDamage dmg;
    dmg.damageType = model::DamageType::DAMAGE_TYPE_HEAT;
    dmg.dmgDice = {model::Dice::D0};
    dmg.dmgBonus = 4;
    dmg.dmgStat = model::StatsEnum::STAT_MND;
    dmg.dmgStatMult = 0.f;
    singeAbility.damages.pushBack(dmg);
    database.addAbilityTemplate(singeAbility);

    model::SpellTemplate singeSpell;
    singeSpell.name = "SINGE_TEST";
    singeSpell.abilityName = "SPELL_SINGE_TEST";
    database.addSpellTemplate(singeSpell);

    model::CharacterInstance* enemy = nullptr;
    for (auto& ch : state.world.activeMap.characters) {
      if (ch.id == "enemy-1") {
        enemy = &ch;
        break;
      }
    }
    ok = assertTrue(enemy != nullptr, "enemy for TARGET_UNIT cast") && ok;
    const auto enemyHpBefore =
        enemy ? model::getCharacterHp(state.player, *enemy) : 0;
    const auto casterHpBefore = state.player.party[0].currentHp;

    model::SpellTargetInfo target;
    target.tileX = 2;
    target.tileY = 1;
    state.world.activeMap.damageParticles.clear();
    stateManager.enqueueAction(
        state::makeAction<state::actions::PerformSpellCast>("ally-1", "SINGE_TEST", target),
        0);
    bmin::String particleColor;
    for (int elapsed = 0; elapsed < 4000; elapsed += 50) {
      stateManager.update(50);
      const auto& particles = state.world.activeMap.damageParticles;
      if (!particles.empty() && !particles.back().textColor.empty()) {
        particleColor = particles.back().textColor;
      }
      const auto& actions = stateManager.getActionData();
      if (actions.sequentialActions.empty() && actions.sequentialActionsNext.empty() &&
          actions.insertActions.empty()) {
        break;
      }
    }

    enemy = nullptr;
    for (auto& ch : state.world.activeMap.characters) {
      if (ch.id == "enemy-1") {
        enemy = &ch;
        break;
      }
    }
    if (enemy) {
      ok = assertEqual(model::getCharacterHp(state.player, *enemy),
                       enemyHpBefore - 4,
                       "TARGET_UNIT no-projectile spell damages enemy at tile") &&
           ok;
    }
    ok = assertEqual(state.player.party[0].currentHp,
                     casterHpBefore,
                     "TARGET_UNIT OTHER does not damage caster") &&
         ok;
    ok = assertTrue(particleColor == "#111111",
                    "TARGET_UNIT particle uses depiction dmgTextColor") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestCombatZoneCast assertions failed" << LOG_ENDL;
    return 1;
  }
  LOG(INFO) << "TestCombatZoneCast completed successfully" << LOG_ENDL;
  return 0;
}
