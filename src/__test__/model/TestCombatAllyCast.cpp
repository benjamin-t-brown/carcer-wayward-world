#include "db/Database.h"
#include "game/map/MapPersistence.h"
#include "model/Combat.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/Abilities.hpp"
#include "model/templates/AbilityTypes.h"
#include "model/templates/MapGrids.hpp"
#include "model/templates/RuneTypes.h"
#include "model/templates/Spells.hpp"
#include "model/templates/Tileset.hpp"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/LayerRequest.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "actions/combat/PerformSpellCast.hpp"
#include "actions/navigation/UiSelectSpellCast.hpp"
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

bool assertEqualStr(const bmin::String& actual, const char* expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
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

bmin::String pumpActions(state::StateManager& stateManager, int maxMs = 4000) {
  bmin::String lastParticleText;
  for (int elapsed = 0; elapsed < maxMs; elapsed += 50) {
    stateManager.update(50);
    const auto& particles = stateManager.getState().world.activeMap.damageParticles;
    if (!particles.empty()) {
      lastParticleText = particles.back().text;
    }
    const auto& actions = stateManager.getActionData();
    if (actions.sequentialActions.empty() && actions.sequentialActionsNext.empty() &&
        actions.insertActions.empty()) {
      return lastParticleText;
    }
  }
  return lastParticleText;
}

void addAllyHealAbilityAndSpell(db::Database& database) {
  model::AbilityTemplate ability;
  ability.name = "SPELL_AID_TEST";
  ability.type = model::AbilityType::ABILITY_SPELL;
  ability.targetSelect.targetType = model::TargetSelectType::TARGET_ALLY;
  ability.apCost = 2;
  ability.costType = model::AbilityCostType::ABILITY_COST_MANA;
  ability.costValue = 1;
  ability.depiction.dmgAnim = "expl_buff";
  ability.depiction.startSound = "cast_chime";
  ability.depiction.dmgSound = "spell_cure";
  ability.depiction.projectileType = model::ProjectileType::PROJECTILE_NONE;
  ability.depiction.projectilePath = model::ProjectilePath::PROJECTILE_PATH_NONE;

  model::AbilityDamage dmg;
  dmg.damageType = model::DamageType::DAMAGE_TYPE_EPHEMERAL;
  dmg.dmgDice = {model::Dice::D0};
  dmg.dmgBonus = 4;
  dmg.dmgStat = model::StatsEnum::STAT_MND;
  dmg.dmgStatMult = 0.f;
  ability.damages.pushBack(dmg);
  database.addAbilityTemplate(ability);

  model::SpellTemplate spell;
  spell.name = "AID_TEST";
  spell.abilityName = "SPELL_AID_TEST";
  database.addSpellTemplate(spell);
}

void setupTinyMap(db::Database& database, state::State& state) {
  addWalkableTileset(database);
  auto map = model::MapInstance{};
  map.id = "ally_cast_map";
  map.templateName = "ally_cast_map";
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
  grid.name = "ally_cast_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = 5;
  grid.mapHeight = 5;
  grid.cells = {{"ally_cast_map"}};
  database.addMapGridTemplate(grid);
  state.world.activeMap.gridId = "ally_cast_grid";
}

model::CharacterPlayer makePartyMember(const char* id, const char* name, int x) {
  (void)x;
  model::CharacterPlayer member;
  member.instanceId = id;
  member.name = name;
  member.templateName = "hero";
  member.params.label = name;
  member.currentHp = 40;
  member.currentMp = 10;
  member.stats.generic.mnd = 1;
  return member;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestCombatAllyCast" << LOG_ENDL;
  auto ok = true;

  db::Database database;
  addAllyHealAbilityAndSpell(database);
  state::DatabaseInterface::setDatabase(&database);

  state::StateManager stateManager;
  state::StateManagerInterface::setStateManager(&stateManager);
  auto& state = stateManager.getState();
  setupTinyMap(database, state);

  state.player.party.pushBack(makePartyMember("ally-1", "Hero", 1));
  state.player.party[0].knownSpells = {"AID_TEST"};
  state.player.party.pushBack(makePartyMember("ally-2", "Friend", 3));

  auto casterInst = model::CharacterInstance{};
  casterInst.id = "ally-1";
  casterInst.name = "Hero";
  casterInst.templateName = "hero";
  casterInst.x = 1;
  casterInst.y = 1;
  casterInst.currentHp = 40;
  casterInst.maxHp = 40;
  casterInst.hpInitialized = true;
  state.world.activeMap.characters.pushBack(std::move(casterInst));

  auto friendInst = model::CharacterInstance{};
  friendInst.id = "ally-2";
  friendInst.name = "Friend";
  friendInst.templateName = "hero";
  friendInst.x = 3;
  friendInst.y = 1;
  friendInst.currentHp = 40;
  friendInst.maxHp = 40;
  friendInst.hpInitialized = true;
  state.world.activeMap.characters.pushBack(std::move(friendInst));

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

  state.world.combat.active = true;
  state.world.combat.isWaitingForAction = true;
  state.world.combat.activeCharacterId = "ally-1";

  {
    state.uiState.layerCommands.clear();
    state::actions::UiSelectSpellCast select("AID_TEST", "ally-1");
    select.execute(&state);
    ok = assertTrue(state.world.actionMode == model::WorldActionMode::NONE,
                    "ally spell does not enter map aim") &&
         ok;
    bool pushedAllyPicker = false;
    bool removedSpellCast = false;
    for (const auto& command : state.uiState.layerCommands) {
      if (command.type == state::LayerCommandType::Push &&
          command.request.id == state::LayerId::SpellAllyTarget &&
          command.request.a == "ally-1" && command.request.b == "AID_TEST") {
        pushedAllyPicker = true;
      }
      if (command.type == state::LayerCommandType::Remove &&
          command.request.id == state::LayerId::SpellCast) {
        removedSpellCast = true;
      }
    }
    ok = assertTrue(pushedAllyPicker, "opens ally picker layer") && ok;
    ok = assertTrue(!removedSpellCast, "keeps spell picker open under ally popover") &&
         ok;
  }

  {
    const auto casterHpBefore = state.player.party[0].currentHp;
    const auto friendHpBefore = state.player.party[1].currentHp;
    model::CharacterInstance* enemyPtr = nullptr;
    for (auto& ch : state.world.activeMap.characters) {
      if (ch.id == "enemy-1") {
        enemyPtr = &ch;
        break;
      }
    }
    const auto enemyHpBefore = enemyPtr ? model::getCharacterHp(state.player, *enemyPtr) : 0;

    model::SpellTargetInfo target;
    target.targetCharacterId = "ally-2";
    target.tileX = 3;
    target.tileY = 1;
    stateManager.enqueueAction(
        state::makeAction<state::actions::PerformSpellCast>("ally-1", "AID_TEST", target),
        0);
    pumpActions(stateManager);

    ok = assertEqual(state.player.party[1].currentHp,
                     friendHpBefore - 4,
                     "ally target takes ability damage") &&
         ok;
    ok = assertEqual(state.player.party[0].currentHp,
                     casterHpBefore,
                     "caster hp unchanged") &&
         ok;
    enemyPtr = nullptr;
    for (auto& ch : state.world.activeMap.characters) {
      if (ch.id == "enemy-1") {
        enemyPtr = &ch;
        break;
      }
    }
    if (enemyPtr) {
      ok = assertEqual(model::getCharacterHp(state.player, *enemyPtr),
                       enemyHpBefore,
                       "enemy not hit by ally spell") &&
           ok;
    }
  }

  {
    model::AbilityTemplate healAbility;
    healAbility.name = "SPELL_HEAL_TEST";
    healAbility.type = model::AbilityType::ABILITY_SPELL;
    healAbility.targetSelect.targetType = model::TargetSelectType::TARGET_ALLY;
    healAbility.apCost = 1;
    healAbility.depiction.dmgAnim = "expl_buff";
    healAbility.depiction.projectileType = model::ProjectileType::PROJECTILE_NONE;
    healAbility.depiction.projectilePath = model::ProjectilePath::PROJECTILE_PATH_NONE;
    model::AbilityRestore restore;
    restore.restoreWhich = model::CurrentStatEnum::CURRENT_STAT_HP;
    restore.restoreDice = {model::Dice::D0};
    restore.restoreBonus = 10;
    restore.restoreStat = model::StatsEnum::STAT_CON;
    restore.restoreStatMult = 0;
    healAbility.restores.pushBack(restore);
    database.addAbilityTemplate(healAbility);

    model::SpellTemplate healSpell;
    healSpell.name = "HEAL_TEST";
    healSpell.abilityName = "SPELL_HEAL_TEST";
    database.addSpellTemplate(healSpell);

    state.player.party[1].currentHp = 20;
    model::SpellTargetInfo target;
    target.targetCharacterId = "ally-2";
    target.tileX = 3;
    target.tileY = 1;
    stateManager.enqueueAction(
        state::makeAction<state::actions::PerformSpellCast>("ally-1", "HEAL_TEST", target),
        0);
    pumpActions(stateManager);

    ok = assertEqual(state.player.party[1].currentHp, 30, "ally restore heals hp") && ok;
  }

  {
    state.player.party[1].currentHp = 38;
    state.world.activeMap.damageParticles.clear();
    model::SpellTargetInfo target;
    target.targetCharacterId = "ally-2";
    target.tileX = 3;
    target.tileY = 1;
    stateManager.enqueueAction(
        state::makeAction<state::actions::PerformSpellCast>("ally-1", "HEAL_TEST", target),
        0);
    const auto particleText = pumpActions(stateManager);

    ok = assertEqual(state.player.party[1].currentHp, 40, "heal capped at max hp") &&
         ok;
    ok = assertEqualStr(particleText, "2", "floater shows actual hp restored") && ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestCombatAllyCast failed" << LOG_ENDL;
    return 1;
  }
  LOG(INFO) << "TestCombatAllyCast completed successfully" << LOG_ENDL;
  return 0;
}
