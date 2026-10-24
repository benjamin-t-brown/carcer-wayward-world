#include "db/Database.h"
#include "game/combat/EnemyBehavior.h"
#include "game/map/Camera.h"
#include "game/map/CharacterConstruction.h"
#include "model/Combat.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/CharacterPlayer.h"
#include "model/templates/Abilities.hpp"
#include "model/templates/AbilityTypes.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/MapGrids.hpp"
#include "model/templates/Spells.hpp"
#include "model/templates/Tileset.hpp"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/State.hpp"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "actions/combat/DoCPUCombatTurn.hpp"
#include "actions/combat/PerformSpellCast.hpp"
#include "bmin/String.h"
#include <cstdlib>

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

bool assertFalse(bool cond, const char* label) {
  if (cond) {
    LOG(ERROR) << label << " expected false" << LOG_ENDL;
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

void setupMap(db::Database& database, state::State& state) {
  addWalkableTileset(database);
  auto map = model::MapInstance{};
  map.id = "spell_ai_map";
  map.templateName = "spell_ai_map";
  map.width = 8;
  map.height = 8;
  map.spriteWidth = 28;
  map.spriteHeight = 32;
  auto layer = bmin::DynArray<model::TileInstance>{};
  for (auto y = 0; y < 8; y++) {
    for (auto x = 0; x < 8; x++) {
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
  grid.name = "spell_ai_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = 8;
  grid.mapHeight = 8;
  grid.cells = {{"spell_ai_map"}};
  database.addMapGridTemplate(grid);
  state.world.activeMap.gridId = "spell_ai_grid";
}

void addSpell(db::Database& database,
              const char* spellName,
              const char* abilityName,
              model::TargetSelectType targetType,
              int zoneW,
              int zoneH,
              int range,
              int apCost,
              int manaCost) {
  model::AbilityTemplate ability;
  ability.name = abilityName;
  ability.type = model::AbilityType::ABILITY_SPELL;
  ability.targetSelect.targetType = targetType;
  ability.targetSelect.allegianceSelectType =
      model::TargetAllegianceSelectType::TARGET_ALLEGIANCE_OTHER;
  ability.targetSelect.zoneSize = {.x = zoneW, .y = zoneH};
  ability.targetSelect.range = range;
  ability.apCost = apCost;
  if (manaCost > 0) {
    ability.costType = model::AbilityCostType::ABILITY_COST_MANA;
    ability.costValue = manaCost;
  } else {
    ability.costType = model::AbilityCostType::ABILITY_COST_NONE;
  }
  ability.depiction.projectileType = model::ProjectileType::PROJECTILE_NONE;
  ability.depiction.projectilePath = model::ProjectilePath::PROJECTILE_PATH_NONE;
  if (targetType == model::TargetSelectType::TARGET_SELF) {
    model::AbilityRestore restore;
    restore.restoreWhich = model::CurrentStatEnum::CURRENT_STAT_HP;
    restore.restoreDice = {model::Dice::D0};
    restore.restoreBonus = 8;
    ability.restores.pushBack(restore);
  } else {
    model::AbilityDamage dmg;
    dmg.damageType = model::DamageType::DAMAGE_TYPE_HEAT;
    dmg.dmgDice = {model::Dice::D0};
    dmg.dmgBonus = 4;
    dmg.dmgStat = model::StatsEnum::STAT_MND;
    dmg.dmgStatMult = 0.f;
    ability.damages.pushBack(dmg);
  }
  database.addAbilityTemplate(ability);

  model::SpellTemplate spell;
  spell.name = spellName;
  spell.abilityName = abilityName;
  database.addSpellTemplate(spell);
}

void addStatusOnlyZoneSpell(db::Database& database,
                            const char* spellName,
                            const char* abilityName,
                            const char* statusName) {
  model::AbilityTemplate ability;
  ability.name = abilityName;
  ability.type = model::AbilityType::ABILITY_SPELL;
  ability.targetSelect.targetType = model::TargetSelectType::TARGET_ZONE;
  ability.targetSelect.allegianceSelectType =
      model::TargetAllegianceSelectType::TARGET_ALLEGIANCE_OTHER;
  ability.targetSelect.zoneSize = {.x = 1, .y = 1};
  ability.targetSelect.range = 10;
  ability.apCost = 6;
  ability.costType = model::AbilityCostType::ABILITY_COST_MANA;
  ability.costValue = 2;
  ability.depiction.projectileType = model::ProjectileType::PROJECTILE_NONE;
  ability.depiction.projectilePath = model::ProjectilePath::PROJECTILE_PATH_NONE;
  model::AbilityStatus status;
  status.statusEffect = statusName;
  ability.statuses.pushBack(status);
  database.addAbilityTemplate(ability);

  model::SpellTemplate spell;
  spell.name = spellName;
  spell.abilityName = abilityName;
  database.addSpellTemplate(spell);
}

model::CombatSpellAiRule alwaysRule(const char* spellName, int weight = 1) {
  model::CombatSpellAiRule rule;
  rule.spell = spellName;
  rule.weight = weight;
  model::CombatSpellAiCondition always;
  always.type = model::CombatSpellAiConditionType::ALWAYS;
  rule.conditions.pushBack(always);
  return rule;
}

model::CharacterInstance* findCh(model::ActiveMap& activeMap, const char* id) {
  for (auto& ch : activeMap.characters) {
    if (ch.id == id) {
      return &ch;
    }
  }
  return nullptr;
}

void addNpcAt(state::State& state, const char* id, int x, int y) {
  model::CharacterInstance inst;
  inst.id = id;
  inst.templateName = "npc";
  inst.type = model::CharacterTemplateType::TOWNSPERSON;
  inst.x = x;
  inst.y = y;
  inst.currentHp = 40;
  inst.maxHp = 40;
  inst.hpInitialized = true;
  state.world.activeMap.characters.pushBack(std::move(inst));
}

void addPartyAt(state::State& state, const char* id, int x, int y, int hp = 40) {
  model::CharacterPlayer member;
  member.instanceId = id;
  member.name = id;
  member.templateName = "hero";
  member.params.combat.hp = 40;
  member.currentHp = hp;
  state.player.party.pushBack(std::move(member));

  model::CharacterInstance inst;
  inst.id = id;
  inst.templateName = "hero";
  inst.type = model::CharacterTemplateType::TOWNSPERSON;
  inst.x = x;
  inst.y = y;
  inst.currentHp = hp;
  inst.maxHp = 40;
  inst.hpInitialized = true;
  state.world.activeMap.characters.pushBack(std::move(inst));
}

model::CharacterInstance& addMage(state::State& state,
                                  db::Database& database,
                                  int hp,
                                  int maxHp,
                                  int mp) {
  model::CharacterTemplate tmpl;
  tmpl.type = model::CharacterTemplateType::ENEMY;
  tmpl.name = "mage_ai";
  tmpl.combat.hp = maxHp;
  tmpl.combat.mp = mp;
  database.addCharacterTemplate(tmpl);

  model::CharacterInstance mage;
  mage.id = "mage-1";
  mage.templateName = "mage_ai";
  mage.type = model::CharacterTemplateType::ENEMY;
  mage.x = 1;
  mage.y = 1;
  mage.currentHp = hp;
  mage.maxHp = maxHp;
  mage.hpInitialized = true;
  mage.currentMp = mp;
  mage.maxMp = mp;
  mage.mpInitialized = true;
  mage.currentAp = model::COMBAT_STARTING_AP;
  mage.agitated = true;
  state.world.activeMap.characters.pushBack(std::move(mage));
  game::applyCharacterTemplateFromDatabase(
      state.world.activeMap.characters[state.world.activeMap.characters.size() - 1],
      database);
  auto* onMap = findCh(state.world.activeMap, "mage-1");
  onMap->currentHp = hp;
  onMap->hpInitialized = true;
  onMap->currentMp = mp;
  onMap->mpInitialized = true;
  onMap->currentAp = model::COMBAT_STARTING_AP;
  return *onMap;
}

void pumpActionsOnly(state::StateManager& stateManager, int maxMs = 4000) {
  for (int elapsed = 0; elapsed < maxMs; elapsed += 50) {
    stateManager.update(50);
    const auto& actions = stateManager.getActionData();
    if (actions.sequentialActions.empty() && actions.sequentialActionsNext.empty() &&
        actions.insertActions.empty()) {
      return;
    }
  }
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestEnemySpellAi" << LOG_ENDL;
  auto ok = true;

  {
    db::Database database;
    addSpell(database,
             "HEAL_SELF_AI",
             "SPELL_HEAL_SELF_AI",
             model::TargetSelectType::TARGET_SELF,
             1,
             1,
             1,
             1,
             0);
    addSpell(database,
             "FIREBALL_AI",
             "SPELL_FIREBALL_AI",
             model::TargetSelectType::TARGET_ZONE,
             3,
             3,
             10,
             6,
             5);
    addSpell(database,
             "FLAME_AI",
             "SPELL_FLAME_AI",
             model::TargetSelectType::TARGET_ZONE,
             1,
             1,
             10,
             6,
             2);
    addSpell(database,
             "BLESS_AI",
             "SPELL_BLESS_AI",
             model::TargetSelectType::TARGET_SELF,
             1,
             1,
             1,
             1,
             0);
    state::DatabaseInterface::setDatabase(&database);

    state::State state;
    setupMap(database, state);
    addPartyAt(state, "ally-1", 3, 1);
    addPartyAt(state, "ally-2", 3, 2);
    addPartyAt(state, "ally-3", 2, 2);
    auto& mage = addMage(state, database, 10, 40, 10);
    mage.combatSpellAi.spells = {"HEAL_SELF_AI", "FIREBALL_AI", "FLAME_AI", "BLESS_AI"};

    model::CombatSpellAiRule heal;
    heal.spell = "HEAL_SELF_AI";
    model::CombatSpellAiCondition lowHp;
    lowHp.type = model::CombatSpellAiConditionType::SELF_HP_BELOW_PERCENT;
    lowHp.value = 50;
    heal.conditions.pushBack(lowHp);
    mage.combatSpellAi.rules.pushBack(heal);
    mage.combatTurnsTaken = 1;

    game::EnemySpellAction action;
    ok = assertTrue(game::chooseEnemySpellAction(
                        state.world, state.mapInstances, state.player, mage, database, action),
                    "heal when below 50%") &&
         ok;
    ok = assertEqualStr(action.spellName, "HEAL_SELF_AI", "heal spell") && ok;
    ok = assertEqualStr(action.targetChId, "mage-1", "heal targets self") && ok;

    mage.currentHp = 40;
    ok = assertFalse(game::chooseEnemySpellAction(
                         state.world, state.mapInstances, state.player, mage, database, action),
                     "no heal when healthy") &&
         ok;

    mage.combatSpellAi.rules.clear();
    model::CombatSpellAiRule fireball;
    fireball.spell = "FIREBALL_AI";
    model::CombatSpellAiCondition zone3;
    zone3.type = model::CombatSpellAiConditionType::ZONE_HOSTILE_COUNT_AT_LEAST;
    zone3.value = 3;
    fireball.conditions.pushBack(zone3);
    mage.combatSpellAi.rules.pushBack(fireball);
    mage.currentHp = 40;
    ok = assertTrue(game::chooseEnemySpellAction(
                        state.world, state.mapInstances, state.player, mage, database, action),
                    "fireball when 3 hostiles") &&
         ok;
    ok = assertEqualStr(action.spellName, "FIREBALL_AI", "fireball spell") && ok;

    state.player.party[2].currentHp = 0;
    auto* ally3 = findCh(state.world.activeMap, "ally-3");
    if (ally3) {
      ally3->currentHp = 0;
      ally3->hpInitialized = true;
    }
    ok = assertFalse(game::chooseEnemySpellAction(
                         state.world, state.mapInstances, state.player, mage, database, action),
                     "no fireball with only 2 hostiles") &&
         ok;

    mage.combatSpellAi.rules.pushBack(alwaysRule("HEAL_SELF_AI"));
    ok = assertTrue(game::chooseEnemySpellAction(
                        state.world, state.mapInstances, state.player, mage, database, action),
                    "FIRST skips targeting miss and uses next") &&
         ok;
    ok = assertEqualStr(action.spellName, "HEAL_SELF_AI", "self spell after zone miss") &&
         ok;

    state.player.party[2].currentHp = 40;
    if (ally3) {
      ally3->currentHp = 40;
    }

    mage.combatSpellAi.rules.clear();
    model::CombatSpellAiRule bless;
    bless.spell = "BLESS_AI";
    model::CombatSpellAiCondition lacks;
    lacks.type = model::CombatSpellAiConditionType::LACKS_STATUS;
    lacks.status = "BLESSED";
    bless.conditions.pushBack(lacks);
    mage.combatSpellAi.rules.pushBack(bless);
    ok = assertTrue(game::chooseEnemySpellAction(
                        state.world, state.mapInstances, state.player, mage, database, action),
                    "buff when lacking status") &&
         ok;
    mage.statusEffects.pushBack({.statusEffectName = "BLESSED", .remainingTurns = 3});
    ok = assertFalse(game::chooseEnemySpellAction(
                         state.world, state.mapInstances, state.player, mage, database, action),
                     "melee while buff remains") &&
         ok;

    mage.statusEffects.clear();
    mage.combatSpellAi.selection = model::CombatSpellAiSelection::RANDOM;
    mage.combatSpellAi.rules.clear();
    mage.combatSpellAi.rules.pushBack(alwaysRule("FLAME_AI"));
    mage.combatSpellAi.rules.pushBack(alwaysRule("FIREBALL_AI"));
    std::srand(1);
    ok = assertTrue(game::chooseEnemySpellAction(
                        state.world, state.mapInstances, state.player, mage, database, action),
                    "random picks a legal spell") &&
         ok;
    ok = assertTrue(action.spellName == "FLAME_AI" || action.spellName == "FIREBALL_AI",
                    "random spell is one of the two") &&
         ok;

    mage.combatSpellAi.selection = model::CombatSpellAiSelection::FIRST;
    mage.combatSpellAi.skipChance = 100;
    mage.combatSpellAi.rules.clear();
    mage.combatSpellAi.rules.pushBack(alwaysRule("FLAME_AI"));
    ok = assertFalse(game::chooseEnemySpellAction(
                         state.world, state.mapInstances, state.player, mage, database, action),
                     "skipChance 100 never casts") &&
         ok;

    mage.combatSpellAi.skipChance = 0;
    mage.currentMp = 1;
    mage.combatSpellAi.rules.clear();
    mage.combatSpellAi.rules.pushBack(alwaysRule("FIREBALL_AI"));
    mage.combatSpellAi.rules.pushBack(alwaysRule("HEAL_SELF_AI"));
    ok = assertTrue(game::chooseEnemySpellAction(
                        state.world, state.mapInstances, state.player, mage, database, action),
                    "FIRST skips unaffordable MP and uses next") &&
         ok;
    ok = assertEqualStr(action.spellName, "HEAL_SELF_AI", "cheaper spell after MP miss") &&
         ok;

    mage.currentMp = 0;
    mage.combatSpellAi.rules.clear();
    mage.combatSpellAi.rules.pushBack(alwaysRule("FIREBALL_AI"));
    ok = assertFalse(game::chooseEnemySpellAction(
                         state.world, state.mapInstances, state.player, mage, database, action),
                     "no MP falls through") &&
         ok;

    mage.currentMp = 10;
    mage.currentAp = model::COMBAT_STARTING_AP;
    mage.combatSpellAi.selection = model::CombatSpellAiSelection::RANDOM;
    mage.combatSpellAi.rules.clear();
    ok = assertTrue(game::chooseEnemySpellAction(
                        state.world, state.mapInstances, state.player, mage, database, action),
                    "empty rules still pick from known spells") &&
         ok;
    ok = assertTrue(action.spellName == "HEAL_SELF_AI" || action.spellName == "FIREBALL_AI" ||
                        action.spellName == "FLAME_AI" || action.spellName == "BLESS_AI",
                    "implicit ALWAYS uses known spells") &&
         ok;

    mage.combatSpellAi.selection = model::CombatSpellAiSelection::FIRST;
    mage.combatSpellAi.rules.pushBack(alwaysRule("FIREBALL_AI"));
    ok = assertTrue(game::chooseEnemySpellAction(
                        state.world, state.mapInstances, state.player, mage, database, action),
                    "6 AP spell is eligible from 4 AP") &&
         ok;
    ok = assertEqualStr(action.spellName, "FIREBALL_AI", "6 AP fireball chosen") && ok;
  }

  {
    db::Database database;
    addSpell(database,
             "FLAME_AI",
             "SPELL_FLAME_AI",
             model::TargetSelectType::TARGET_ZONE,
             1,
             1,
             10,
             6,
             2);
    state::DatabaseInterface::setDatabase(&database);

    state::State state;
    setupMap(database, state);
    addPartyAt(state, "pc-1", 5, 5);
    addNpcAt(state, "npc-1", 2, 1);
    auto& mage = addMage(state, database, 25, 25, 10);
    mage.combatSpellAi.spells = {"FLAME_AI"};
    mage.combatSpellAi.rules.pushBack(alwaysRule("FLAME_AI"));

    game::EnemySpellAction action;
    ok = assertTrue(game::chooseEnemySpellAction(
                        state.world, state.mapInstances, state.player, mage, database, action),
                    "default zone picks a hostile tile") &&
         ok;
    ok = assertEqual(action.targetLoc.x, 2, "default prefers closer hostile") && ok;
    ok = assertEqual(action.targetLoc.y, 1, "default prefers closer hostile y") && ok;

    mage.combatSpellAi.rules.clear();
    auto onlyPc = alwaysRule("FLAME_AI");
    onlyPc.zoneTargeting = model::CombatSpellAiZoneTargeting::ONLY_PC;
    mage.combatSpellAi.rules.pushBack(onlyPc);
    ok = assertTrue(game::chooseEnemySpellAction(
                        state.world, state.mapInstances, state.player, mage, database, action),
                    "only PC finds the party member") &&
         ok;
    ok = assertEqual(action.targetLoc.x, 5, "only PC ignores closer NPC x") && ok;
    ok = assertEqual(action.targetLoc.y, 5, "only PC ignores closer NPC y") && ok;

    mage.combatSpellAi.rules.clear();
    mage.combatSpellAi.zoneTargeting = model::CombatSpellAiZoneTargeting::ONLY_PC;
    ok = assertTrue(game::chooseEnemySpellAction(
                        state.world, state.mapInstances, state.player, mage, database, action),
                    "empty rules inherit AI only-PC targeting") &&
         ok;
    ok = assertEqual(action.targetLoc.x, 5, "inherited only PC x") && ok;
    ok = assertEqual(action.targetLoc.y, 5, "inherited only PC y") && ok;

    mage.combatSpellAi.zoneTargeting = model::CombatSpellAiZoneTargeting::DEFAULT;
    mage.combatSpellAi.rules.clear();
    auto randomZone = alwaysRule("FLAME_AI");
    randomZone.zoneTargeting = model::CombatSpellAiZoneTargeting::RANDOM_IN_RANGE;
    mage.combatSpellAi.rules.pushBack(randomZone);
    std::srand(1);
    ok = assertTrue(game::chooseEnemySpellAction(
                        state.world, state.mapInstances, state.player, mage, database, action),
                    "random in range finds a hostile character") &&
         ok;
    const auto randomHitsHostile =
        (action.targetLoc.x == 2 && action.targetLoc.y == 1) ||
        (action.targetLoc.x == 5 && action.targetLoc.y == 5);
    ok = assertTrue(randomHitsHostile, "random in range centers on a hostile") && ok;
  }

  {
    db::Database database;
    addSpell(database,
             "FIREBALL_CENTER_AI",
             "SPELL_FIREBALL_CENTER_AI",
             model::TargetSelectType::TARGET_ZONE,
             3,
             3,
             10,
             6,
             2);
    state::DatabaseInterface::setDatabase(&database);

    state::State state;
    setupMap(database, state);
    addPartyAt(state, "pc-1", 5, 5);
    auto& mage = addMage(state, database, 25, 25, 10);
    mage.combatSpellAi.spells = {"FIREBALL_CENTER_AI"};
    auto randomZone = alwaysRule("FIREBALL_CENTER_AI");
    randomZone.zoneTargeting = model::CombatSpellAiZoneTargeting::RANDOM_IN_RANGE;
    mage.combatSpellAi.rules.pushBack(randomZone);

    game::EnemySpellAction action;
    for (int seed = 1; seed <= 16; seed++) {
      std::srand(seed);
      ok = assertTrue(game::chooseEnemySpellAction(state.world,
                                                   state.mapInstances,
                                                   state.player,
                                                   mage,
                                                   database,
                                                   action),
                      "3x3 random aims at the only in-range character") &&
           ok;
      ok = assertEqual(action.targetLoc.x, 5, "3x3 random does not use empty center x") &&
           ok;
      ok = assertEqual(action.targetLoc.y, 5, "3x3 random does not use empty center y") &&
           ok;
    }

    addSpell(database,
             "FIREBALL_SHORT_AI",
             "SPELL_FIREBALL_SHORT_AI",
             model::TargetSelectType::TARGET_ZONE,
             3,
             3,
             4,
             6,
             2);
    addPartyAt(state, "pc-far", 7, 7);
    auto* mageAfterFar = findCh(state.world.activeMap, "mage-1");
    ok = assertTrue(mageAfterFar != nullptr, "mage still on map after far PC") && ok;
    if (mageAfterFar == nullptr) {
      return ok;
    }
    mageAfterFar->combatSpellAi.spells = {"FIREBALL_SHORT_AI"};
    mageAfterFar->combatSpellAi.rules.clear();
    auto shortRange = alwaysRule("FIREBALL_SHORT_AI");
    shortRange.zoneTargeting = model::CombatSpellAiZoneTargeting::RANDOM_IN_RANGE;
    mageAfterFar->combatSpellAi.rules.pushBack(shortRange);
    std::srand(1);
    ok = assertTrue(game::chooseEnemySpellAction(state.world,
                                                 state.mapInstances,
                                                 state.player,
                                                 *mageAfterFar,
                                                 database,
                                                 action),
                    "out-of-range character is not a center") &&
         ok;
    ok = assertEqual(action.targetLoc.x, 5, "keeps the in-range character x") && ok;
    ok = assertEqual(action.targetLoc.y, 5, "keeps the in-range character y") && ok;
  }

  {
    db::Database database;
    addSpell(database,
             "FIREBALL_AI",
             "SPELL_FIREBALL_AI",
             model::TargetSelectType::TARGET_ZONE,
             3,
             3,
             10,
             6,
             2);
    state::DatabaseInterface::setDatabase(&database);
    state::StateManager stateManager;
    state::StateManagerInterface::setStateManager(&stateManager);
    auto& state = stateManager.getState();
    setupMap(database, state);
    addPartyAt(state, "ally-1", 3, 1);
    auto& mage = addMage(state, database, 25, 25, 10);
    mage.combatBehaviorCombat = model::CombatBehaviorName::SPELLCASTER;
    mage.combatSpellAi.spells = {"FIREBALL_AI"};
    mage.combatSpellAi.rules.pushBack(alwaysRule("FIREBALL_AI"));

    state.world.combat.active = true;
    state.world.combat.turnOrderIds = {"ally-1", "mage-1"};
    state.world.combat.activeCharacterId = "mage-1";
    state.world.combat.isWaitingForAction = true;

    stateManager.enqueueAction(state::makeAction<state::actions::DoCPUCombatTurn>(), 0);
    pumpActionsOnly(stateManager);

    auto* after = findCh(stateManager.getState().world.activeMap, "mage-1");
    ok = assertTrue(after != nullptr, "mage after cpu spell turn") && ok;
    if (after) {
      ok = assertEqual(after->currentAp, model::COMBAT_STARTING_AP - 6,
                       "6 AP cast from 4 AP goes negative") &&
           ok;
      ok = assertEqual(after->currentMp, 8, "cpu spell spent MP") && ok;
    }
  }

  {
    db::Database database;
    addSpell(database,
             "FIREBALL_AI",
             "SPELL_FIREBALL_AI",
             model::TargetSelectType::TARGET_ZONE,
             3,
             3,
             10,
             6,
             2);
    state::DatabaseInterface::setDatabase(&database);
    state::StateManager stateManager;
    state::StateManagerInterface::setStateManager(&stateManager);
    auto& state = stateManager.getState();
    setupMap(database, state);
    addPartyAt(state, "ally-1", 3, 1);
    auto& melee = addMage(state, database, 25, 25, 10);
    melee.combatBehaviorCombat = model::CombatBehaviorName::SEEK_AND_MELEE;
    melee.combatSpellAi.spells = {"FIREBALL_AI"};
    melee.combatSpellAi.rules.pushBack(alwaysRule("FIREBALL_AI"));

    state.world.combat.active = true;
    state.world.combat.turnOrderIds = {"ally-1", "mage-1"};
    state.world.combat.activeCharacterId = "mage-1";
    state.world.combat.isWaitingForAction = true;

    stateManager.enqueueAction(state::makeAction<state::actions::DoCPUCombatTurn>(), 0);
    pumpActionsOnly(stateManager);

    auto* after = findCh(stateManager.getState().world.activeMap, "mage-1");
    ok = assertTrue(after != nullptr, "seek melee after cpu turn") && ok;
    if (after) {
      ok = assertEqual(after->currentMp, 10, "SEEK_AND_MELEE does not spend MP") && ok;
      ok = assertTrue(after->currentAp > model::COMBAT_STARTING_AP - 6,
                      "SEEK_AND_MELEE does not pay spell AP") &&
           ok;
    }
  }

  {
    db::Database database;
    addSpell(database,
             "FIREBALL_AI",
             "SPELL_FIREBALL_AI",
             model::TargetSelectType::TARGET_ZONE,
             3,
             3,
             10,
             6,
             2);
    state::DatabaseInterface::setDatabase(&database);
    state::StateManager stateManager;
    state::StateManagerInterface::setStateManager(&stateManager);
    auto& state = stateManager.getState();
    setupMap(database, state);
    addPartyAt(state, "ally-1", 3, 1);
    auto& mage = addMage(state, database, 25, 25, 10);
    mage.combatBehaviorCombat = model::CombatBehaviorName::SPELLCASTER;
    mage.combatSpellAi.spells = {"FIREBALL_AI"};
    model::CombatSpellAiRule fireball;
    fireball.spell = "FIREBALL_AI";
    model::CombatSpellAiCondition zone3;
    zone3.type = model::CombatSpellAiConditionType::ZONE_HOSTILE_COUNT_AT_LEAST;
    zone3.value = 3;
    fireball.conditions.pushBack(zone3);
    mage.combatSpellAi.rules.pushBack(fireball);

    game::EnemySpellAction action;
    ok = assertFalse(game::chooseEnemySpellAction(
                         state.world, state.mapInstances, state.player, mage, database, action),
                     "all spells fail when no zone target") &&
         ok;

    state.world.combat.active = true;
    state.world.combat.turnOrderIds = {"ally-1", "mage-1"};
    state.world.combat.activeCharacterId = "mage-1";
    state.world.combat.isWaitingForAction = true;

    const auto startX = mage.x;
    const auto startY = mage.y;
    stateManager.enqueueAction(state::makeAction<state::actions::DoCPUCombatTurn>(), 0);
    pumpActionsOnly(stateManager);

    auto* after = findCh(stateManager.getState().world.activeMap, "mage-1");
    ok = assertTrue(after != nullptr, "mage after targeting-miss cpu turn") && ok;
    if (after) {
      ok = assertEqual(after->currentMp, 10, "targeting miss does not spend MP") && ok;
      ok = assertTrue(after->currentAp > model::COMBAT_STARTING_AP - 6,
                      "targeting miss does not pay spell AP") &&
           ok;
      ok = assertTrue(after->x != startX || after->y != startY,
                      "targeting miss falls through to melee move") &&
           ok;
    }
  }

  {
    db::Database database;
    addSpell(database,
             "FIREBALL_AI",
             "SPELL_FIREBALL_AI",
             model::TargetSelectType::TARGET_ZONE,
             3,
             3,
             10,
             6,
             2);
    state::DatabaseInterface::setDatabase(&database);
    state::State state;
    setupMap(database, state);
    addPartyAt(state, "ally-1", 3, 1);
    auto& mage = addMage(state, database, 25, 25, 10);
    mage.combatBehaviorCombat = model::CombatBehaviorName::SPELLCASTER;
    mage.combatSpellAi.spells = {"FIREBALL_AI"};
    mage.combatSpellAi.rules.pushBack(alwaysRule("FIREBALL_AI"));

    ok = assertEqual(static_cast<int>(game::resolveCombatTurnBehavior(mage)),
                     static_cast<int>(model::CombatBehaviorName::SPELLCASTER),
                     "empty turnModes uses combat behavior") &&
         ok;

    model::CombatSpellAiTurnMode meleeMode;
    meleeMode.behavior = model::CombatBehaviorName::SEEK_AND_MELEE;
    mage.combatSpellAi.turnModes.pushBack(meleeMode);
    mage.combatBehaviorChosenTurn = -1;
    ok = assertEqual(static_cast<int>(game::resolveCombatTurnBehavior(mage)),
                     static_cast<int>(model::CombatBehaviorName::SEEK_AND_MELEE),
                     "turnModes pick melee") &&
         ok;

    mage.combatSpellAi.turnModes.clear();
    model::CombatSpellAiTurnMode casterMode;
    casterMode.behavior = model::CombatBehaviorName::SPELLCASTER;
    mage.combatSpellAi.turnModes.pushBack(casterMode);
    ok = assertEqual(static_cast<int>(game::resolveCombatTurnBehavior(mage)),
                     static_cast<int>(model::CombatBehaviorName::SEEK_AND_MELEE),
                     "turn mode sticks for the same turn") &&
         ok;

    mage.combatTurnsTaken += 1;
    ok = assertEqual(static_cast<int>(game::resolveCombatTurnBehavior(mage)),
                     static_cast<int>(model::CombatBehaviorName::SPELLCASTER),
                     "new turn re-rolls turn mode") &&
         ok;

    mage.combatSpellAi.turnModes.clear();
    model::CombatSpellAiTurnMode casterWeighted;
    casterWeighted.behavior = model::CombatBehaviorName::SPELLCASTER;
    casterWeighted.weight = 10;
    model::CombatSpellAiTurnMode meleeZero;
    meleeZero.behavior = model::CombatBehaviorName::SEEK_AND_MELEE;
    meleeZero.weight = 0;
    mage.combatSpellAi.turnModes.pushBack(casterWeighted);
    mage.combatSpellAi.turnModes.pushBack(meleeZero);
    auto zeroWeightPickedMelee = false;
    for (int i = 0; i < 32; i++) {
      mage.combatTurnsTaken += 1;
      if (game::resolveCombatTurnBehavior(mage) ==
          model::CombatBehaviorName::SEEK_AND_MELEE) {
        zeroWeightPickedMelee = true;
        break;
      }
    }
    ok = assertFalse(zeroWeightPickedMelee, "weight 0 turn mode is never picked") && ok;
  }

  {
    db::Database database;
    addSpell(database,
             "FIREBALL_AI",
             "SPELL_FIREBALL_AI",
             model::TargetSelectType::TARGET_ZONE,
             3,
             3,
             10,
             6,
             2);
    state::DatabaseInterface::setDatabase(&database);
    state::StateManager stateManager;
    state::StateManagerInterface::setStateManager(&stateManager);
    auto& state = stateManager.getState();
    setupMap(database, state);
    addPartyAt(state, "ally-1", 3, 1);
    auto& mage = addMage(state, database, 25, 25, 10);
    mage.combatBehaviorCombat = model::CombatBehaviorName::SPELLCASTER;
    mage.combatSpellAi.spells = {"FIREBALL_AI"};
    mage.combatSpellAi.rules.pushBack(alwaysRule("FIREBALL_AI"));
    model::CombatSpellAiTurnMode meleeMode;
    meleeMode.behavior = model::CombatBehaviorName::SEEK_AND_MELEE;
    mage.combatSpellAi.turnModes.pushBack(meleeMode);

    state.world.combat.active = true;
    state.world.combat.turnOrderIds = {"ally-1", "mage-1"};
    state.world.combat.activeCharacterId = "mage-1";
    state.world.combat.isWaitingForAction = true;

    stateManager.enqueueAction(state::makeAction<state::actions::DoCPUCombatTurn>(), 0);
    pumpActionsOnly(stateManager);

    auto* after = findCh(stateManager.getState().world.activeMap, "mage-1");
    ok = assertTrue(after != nullptr, "mage after melee turn-mode cpu turn") && ok;
    if (after) {
      ok = assertEqual(after->currentMp, 10, "melee turn mode does not spend MP") && ok;
      ok = assertTrue(after->currentAp > model::COMBAT_STARTING_AP - 6,
                      "melee turn mode does not pay spell AP") &&
           ok;
    }
  }

  {
    db::Database database;
    addStatusOnlyZoneSpell(database, "SINGE_AI", "SPELL_SINGE_AI", "BURNING");
    state::DatabaseInterface::setDatabase(&database);
    state::StateManager stateManager;
    state::StateManagerInterface::setStateManager(&stateManager);
    auto& state = stateManager.getState();
    setupMap(database, state);
    addPartyAt(state, "ally-1", 5, 5);
    auto& mage = addMage(state, database, 25, 25, 10);
    mage.combatBehaviorCombat = model::CombatBehaviorName::SPELLCASTER;

    state.world.combat.active = true;
    state.world.combat.turnOrderIds = {"ally-1", "mage-1"};
    state.world.combat.activeCharacterId = "mage-1";
    state.world.camera.viewW = 100;
    state.world.camera.viewH = 80;
    state.world.camera.cameraMode = model::CameraMode::Follow;
    state.world.camera.cameraFollowCharacterId = "mage-1";

    model::SpellTargetInfo target;
    target.tileX = 5;
    target.tileY = 5;
    stateManager.enqueueAction(
        state::makeAction<state::actions::PerformSpellCast>("mage-1", "SINGE_AI", target),
        0);

    const auto hitCam = game::computeCameraFollow(5, 5, 100, 80);
    const auto returnCam = game::computeCameraFollow(1, 1, 100, 80);
    auto sawHitCam = false;
    auto snappedBackToCaster = false;
    for (int elapsed = 0; elapsed < 4000; elapsed += 50) {
      stateManager.update(50);
      const auto& camera = stateManager.getState().world.camera;
      if (camera.cameraMode == model::CameraMode::Controlled && camera.camX == hitCam.camX &&
          camera.camY == hitCam.camY) {
        sawHitCam = true;
      } else if (sawHitCam && camera.cameraMode == model::CameraMode::Follow &&
                 camera.camX == returnCam.camX && camera.camY == returnCam.camY) {
        snappedBackToCaster = true;
      }
      const auto& actions = stateManager.getActionData();
      if (actions.sequentialActions.empty() && actions.sequentialActionsNext.empty() &&
          actions.insertActions.empty()) {
        break;
      }
    }
    ok = assertTrue(sawHitCam, "AI spell hit pans camera to target") && ok;
    ok = assertFalse(snappedBackToCaster,
                     "spent turn does not snap camera back to caster") &&
         ok;
    const auto& spentCamera = stateManager.getState().world.camera;
    ok = assertTrue(spentCamera.cameraMode == model::CameraMode::Controlled &&
                        spentCamera.camX == hitCam.camX &&
                        spentCamera.camY == hitCam.camY,
                    "spent turn leaves camera on target until next actor") &&
         ok;

    auto& continueMage = *findCh(stateManager.getState().world.activeMap, "mage-1");
    continueMage.currentAp = 10;
    continueMage.currentMp = 10;
    stateManager.getState().world.camera.cameraMode = model::CameraMode::Follow;
    stateManager.getState().world.camera.cameraFollowCharacterId = "mage-1";
    stateManager.enqueueAction(
        state::makeAction<state::actions::PerformSpellCast>("mage-1", "SINGE_AI", target),
        0);
    auto sawContinueHit = false;
    auto restoredMs = -1;
    auto firstHitMs = -1;
    for (int elapsed = 0; elapsed < 4000; elapsed += 50) {
      stateManager.update(50);
      const auto& camera = stateManager.getState().world.camera;
      if (camera.cameraMode == model::CameraMode::Controlled && camera.camX == hitCam.camX &&
          camera.camY == hitCam.camY) {
        sawContinueHit = true;
        if (firstHitMs < 0) {
          firstHitMs = elapsed;
        }
      } else if (sawContinueHit && restoredMs < 0 &&
                 camera.cameraMode == model::CameraMode::Follow) {
        restoredMs = elapsed;
      }
      const auto& actions = stateManager.getActionData();
      if (actions.sequentialActions.empty() && actions.sequentialActionsNext.empty() &&
          actions.insertActions.empty()) {
        break;
      }
    }
    ok = assertTrue(sawContinueHit, "AP remaining pans camera to target") && ok;
    ok = assertTrue(restoredMs >= 0 && firstHitMs >= 0 && restoredMs - firstHitMs >= 250,
                    "AP remaining holds then returns to caster") &&
         ok;
    const auto& camera = stateManager.getState().world.camera;
    ok = assertTrue(camera.cameraMode == model::CameraMode::Follow,
                    "AP remaining restores follow") &&
         ok;
    ok = assertEqual(camera.camX, returnCam.camX, "AP remaining camera returns to caster x") &&
         ok;
    ok = assertEqual(camera.camY, returnCam.camY, "AP remaining camera returns to caster y") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestEnemySpellAi failed" << LOG_ENDL;
    return 1;
  }
  LOG(INFO) << "TestEnemySpellAi completed successfully" << LOG_ENDL;
  return 0;
}
