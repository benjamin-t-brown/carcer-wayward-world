#include "db/Database.h"
#include "game/combat/MeleeAttackResolve.h"
#include "game/map/CharacterConstruction.h"
#include "model/Combat.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/MapInstance.h"
#include "model/templates/Abilities.hpp"
#include "model/templates/AbilityTypes.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Items.h"
#include "model/templates/MapGrids.hpp"
#include "model/templates/Tileset.hpp"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/StateManager.h"
#include "state/StateManagerInterface.h"
#include "actions/combat/DoCombatAction.hpp"
#include "actions/combat/PerformMeleeAttack.hpp"
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

model::CharacterInstance* findOnActiveMap(model::ActiveMap& activeMap,
                                          const bmin::String& id) {
  for (auto& ch : activeMap.characters) {
    if (ch.id == id) {
      return &ch;
    }
  }
  return nullptr;
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

model::AbilityAttackDmg makeFixedDmg(int bonus) {
  auto dmg = model::AbilityAttackDmg{};
  dmg.dmgDice = {model::Dice::D0};
  dmg.dmgBonus = bonus;
  dmg.dmgStat = model::StatsEnum::STAT_STR;
  dmg.dmgStatMult = 0.f;
  dmg.attackBonus = 0;
  return dmg;
}

model::AbilityTemplate makeAutoHitAbility(const bmin::String& name,
                                          int bonus,
                                          const bmin::String& dmgSound,
                                          const bmin::String& dmgAnim) {
  auto ability = model::AbilityTemplate{};
  ability.name = name;
  auto attack = model::AbilityAttack{};
  attack.attackClass = model::AttackClass::ATTACK_CLASS_AUTO_HIT;
  attack.damageType = model::DamageType::DAMAGE_TYPE_EDGED;
  attack.dmg = makeFixedDmg(bonus);
  ability.attacks.pushBack(attack);
  ability.depiction.dmgAnim = dmgAnim;
  ability.depiction.dmgSound = dmgSound;
  return ability;
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

model::MapInstance makeEmptyMap(int width, int height) {
  auto map = model::MapInstance{};
  map.id = "melee_test_map";
  map.templateName = "melee_test_map";
  map.width = width;
  map.height = height;
  map.spriteWidth = 28;
  map.spriteHeight = 32;
  map.tileLayerNumber = 0;
  auto layer = bmin::DynArray<model::TileInstance>{};
  for (auto y = 0; y < height; y++) {
    for (auto x = 0; x < width; x++) {
      auto tile = model::TileInstance{};
      tile.x = x;
      tile.y = y;
      tile.tilesetName = "test_terrain";
      layer.pushBack(tile);
    }
  }
  model::mapLayerAt(model::mapInstanceTiles(map), 0) = std::move(layer);
  return map;
}

void setupCombatGrid(db::Database& database, state::State& state) {
  addWalkableTileset(database);
  auto map = makeEmptyMap(5, 5);
  state.mapInstances[map.templateName] = std::move(map);

  model::MapGridTemplate grid;
  grid.name = "melee_test_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = 5;
  grid.mapHeight = 5;
  grid.cells = {{"melee_test_map"}};
  database.addMapGridTemplate(grid);
  state.world.activeMap.gridId = "melee_test_grid";
}

state::State makeCombatState(db::Database& database) {
  auto allyTemplate = model::CharacterTemplate{};
  allyTemplate.type = model::CharacterTemplateType::TOWNSPERSON;
  allyTemplate.name = "hero";
  allyTemplate.combat.hp = 100;
  database.addCharacterTemplate(allyTemplate);

  auto enemyTemplate = model::CharacterTemplate{};
  enemyTemplate.type = model::CharacterTemplateType::ENEMY;
  enemyTemplate.name = "slime";
  enemyTemplate.combat.hp = 40;
  enemyTemplate.stats.generic.str = 2;
  database.addCharacterTemplate(enemyTemplate);

  state::DatabaseInterface::setDatabase(&database);

  state::State state;
  setupCombatGrid(database, state);
  state.world.camera.viewW = 100;
  state.world.camera.viewH = 80;

  auto member = model::CharacterPlayer{};
  member.instanceId = "ally-1";
  member.name = "Hero";
  member.templateName = "hero";
  member.currentHp = 100;
  member.stats.generic.str = 7;
  state.player.party.pushBack(std::move(member));

  auto ally = model::CharacterInstance{};
  ally.id = "ally-1";
  ally.name = "Hero";
  ally.templateName = "hero";
  ally.x = 2;
  ally.y = 2;
  state.world.activeMap.characters.pushBack(std::move(ally));

  auto enemy = model::CharacterInstance{};
  enemy.id = "enemy-1";
  enemy.name = "Slime";
  enemy.templateName = "slime";
  enemy.type = model::CharacterTemplateType::ENEMY;
  enemy.x = 3;
  enemy.y = 2;
  enemy.currentHp = 40;
  enemy.hpInitialized = true;
  state.world.activeMap.characters.pushBack(std::move(enemy));

  for (size_t i = 0; i < state.world.activeMap.characters.size(); i++) {
    game::applyCharacterTemplateFromDatabase(state.world.activeMap.characters[i],
                                             database);
  }

  return state;
}

void addMeleeItem(db::Database& database,
                  const bmin::String& name,
                  const bmin::String& abilityName,
                  int dmgBonus) {
  auto item = model::ItemTemplate{};
  item.name = name;
  item.itemType = model::ItemType::WEAPON_MELEE;
  auto weapon = model::ItemWeaponConfig{};
  weapon.abilityName = abilityName;
  weapon.dmgOverrides.pushBack(makeFixedDmg(dmgBonus));
  item.weapon = weapon;
  database.addItemTemplate(item);
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestCombatMeleeResolve" << LOG_ENDL;
  auto ok = true;

  db::Database database;
  database.addAbilityTemplate(makeAutoHitAbility(
      game::kMeleeAttackDefaultName, 5, "hit_punch3", "splash_attack"));
  database.addAbilityTemplate(
      makeAutoHitAbility("MELEE_ATTACK_METAL_KNIFE", 4, "hit_metal1", "splash_attack"));
  database.addAbilityTemplate(
      makeAutoHitAbility("MELEE_ATTACK_METAL_SWORD", 6, "hit_metal1", "splash_attack"));
  addMeleeItem(database, "DaggerBronze", "MELEE_ATTACK_METAL_KNIFE", 8);
  addMeleeItem(database, "SwordBronze", "MELEE_ATTACK_METAL_SWORD", 9);

  auto emptyAttacks = model::AbilityTemplate{};
  emptyAttacks.name = "MELEE_EMPTY_ATTACKS";
  emptyAttacks.depiction.dmgSound = "should_not_play";
  emptyAttacks.depiction.dmgAnim = "should_not_spawn";
  database.addAbilityTemplate(emptyAttacks);

  auto emptyDepict = makeAutoHitAbility("MELEE_EMPTY_DEPICTION", 3, "", "");
  database.addAbilityTemplate(emptyDepict);

  auto state = makeCombatState(database);
  state::StateManager stateManager;
  stateManager.getState() = state;
  state::StateManagerInterface::setStateManager(&stateManager);

  stateManager.enqueueAction(state::makeAction<state::actions::StartCombat>(), 0);
  pumpActions(stateManager);

  {
    auto& world = stateManager.getState().world;
    ok = assertTrue(world.combat.active, "combat.active") && ok;
    auto* ally = findOnActiveMap(world.activeMap, "ally-1");
    ok = assertTrue(ally != nullptr, "ally on map") && ok;
    if (ally) {
      ok = assertEqual(ally->currentAp, model::COMBAT_STARTING_AP, "ally starting AP") &&
           ok;
    }
    const auto resolved =
        game::resolveMeleeAttackAbilities(stateManager.getState().player, *ally, database);
    ok = assertEqual(static_cast<int>(resolved.size()), 1, "unarmed resolve size") &&
         ok;
    if (!resolved.empty()) {
      ok = assertTrue(resolved[0].ability.name == game::kMeleeAttackDefaultName,
                      "unarmed default ability") &&
           ok;
    }

    stateManager.enqueueAction(
        state::makeAction<state::actions::DoCombatAction>(
            "ally-1",
            model::CombatActionType::MOVE,
            state::actions::CombatActionContext{.targetLoc = {1, 0}}),
        0);
    pumpActions(stateManager);

    ally = findOnActiveMap(stateManager.getState().world.activeMap, "ally-1");
    auto* enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(ally != nullptr && enemy != nullptr, "combatants after unarmed") &&
         ok;
    if (ally) {
      ok = assertEqual(ally->currentAp,
                       model::COMBAT_STARTING_AP - model::COMBAT_ATTACK_COST,
                       "unarmed bump spends COMBAT_ATTACK_COST once") &&
           ok;
    }
    if (enemy) {
      ok = assertEqual(model::getCharacterHp(stateManager.getState().player, *enemy),
                       35,
                       "unarmed AUTO_HIT deals 5") &&
           ok;
    }
  }

  {
    auto& party = stateManager.getState().player.party[0];
    party.inventory = {{.itemName = "DaggerBronze", .id = "dagger1", .quantity = 1},
                       {.itemName = "SwordBronze", .id = "sword1", .quantity = 1}};
    party.equipment.weapon0Id = "dagger1";
    party.equipment.weapon1Id.clear();
    auto* ally = findOnActiveMap(stateManager.getState().world.activeMap, "ally-1");
    auto* enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(ally != nullptr && enemy != nullptr, "dagger combatants") && ok;
    if (ally) {
      ally->currentAp = model::COMBAT_STARTING_AP;
    }
    stateManager.getState().world.combat.activeCharacterId = "ally-1";
    stateManager.getState().world.combat.activeTurnIndex = 0;
    stateManager.getState().world.combat.active = true;

    const auto resolved =
        game::resolveMeleeAttackAbilities(stateManager.getState().player, *ally, database);
    ok = assertEqual(static_cast<int>(resolved.size()), 1, "dagger resolve size") && ok;
    if (!resolved.empty()) {
      ok = assertTrue(resolved[0].ability.name == "MELEE_ATTACK_METAL_KNIFE",
                      "dagger ability") &&
           ok;
      ok = assertTrue(resolved[0].ability.attacks[0].dmg.has_value() &&
                          resolved[0].ability.attacks[0].dmg->dmgBonus == 8,
                      "dagger override bonus") &&
           ok;
    }

    const auto hpBefore =
        enemy ? model::getCharacterHp(stateManager.getState().player, *enemy) : 0;
    stateManager.enqueueAction(state::makeAction<state::actions::PerformMeleeAttack>(
                                   "ally-1", "enemy-1"),
                               0);
    pumpActions(stateManager);
    enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    if (enemy) {
      ok = assertEqual(model::getCharacterHp(stateManager.getState().player, *enemy),
                       hpBefore - 8,
                       "dagger override damage") &&
           ok;
    }
  }

  {
    auto& party = stateManager.getState().player.party[0];
    party.equipment.weapon0Id = "dagger1";
    party.equipment.weapon1Id = "sword1";
    auto* ally = findOnActiveMap(stateManager.getState().world.activeMap, "ally-1");
    auto* enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(ally != nullptr && enemy != nullptr, "dual combatants") && ok;
    if (ally) {
      ally->currentAp = model::COMBAT_STARTING_AP;
    }
    stateManager.getState().world.combat.activeCharacterId = "ally-1";
    stateManager.getState().world.combat.activeTurnIndex = 0;

    const auto resolved =
        game::resolveMeleeAttackAbilities(stateManager.getState().player, *ally, database);
    ok = assertEqual(static_cast<int>(resolved.size()), 2, "dual-wield resolve size") &&
         ok;
    if (resolved.size() >= 2) {
      ok = assertTrue(resolved[1].offHand, "second ability off-hand") && ok;
    }

    const auto hpBefore =
        enemy ? model::getCharacterHp(stateManager.getState().player, *enemy) : 0;
    stateManager.enqueueAction(
        state::makeAction<state::actions::DoCombatAction>(
            "ally-1",
            model::CombatActionType::MOVE,
            state::actions::CombatActionContext{.targetLoc = {1, 0}}),
        0);
    pumpActions(stateManager);
    ally = findOnActiveMap(stateManager.getState().world.activeMap, "ally-1");
    enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    if (ally) {
      ok = assertEqual(ally->currentAp,
                       model::COMBAT_STARTING_AP - model::COMBAT_ATTACK_COST,
                       "dual-wield spends AP once") &&
           ok;
    }
    if (enemy) {
      ok = assertEqual(model::getCharacterHp(stateManager.getState().player, *enemy),
                       hpBefore - 8 - 9,
                       "dual-wield both hands damage") &&
           ok;
    }
  }

  {
    auto* ally = findOnActiveMap(stateManager.getState().world.activeMap, "ally-1");
    auto* enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    ok = assertTrue(ally != nullptr && enemy != nullptr, "skip off-hand combatants") &&
         ok;
    if (ally) {
      ally->currentAp = model::COMBAT_STARTING_AP;
    }
    if (enemy) {
      model::setCharacterHp(stateManager.getState().player, *enemy, 8);
    }
    stateManager.getState().world.combat.activeCharacterId = "ally-1";
    stateManager.getState().world.combat.activeTurnIndex = 0;
    stateManager.getState().world.combat.active = true;
    stateManager.enqueueAction(state::makeAction<state::actions::PerformMeleeAttack>(
                                   "ally-1", "enemy-1"),
                               0);
    pumpActions(stateManager);
    enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    if (enemy) {
      ok = assertEqual(model::getCharacterHp(stateManager.getState().player, *enemy),
                       0,
                       "off-hand skipped when main-hand would defeat") &&
           ok;
    }
  }

  {
    auto emptyWeapon = model::ItemTemplate{};
    emptyWeapon.name = "EmptySwing";
    emptyWeapon.itemType = model::ItemType::WEAPON_MELEE;
    auto weapon = model::ItemWeaponConfig{};
    weapon.abilityName = "MELEE_EMPTY_ATTACKS";
    emptyWeapon.weapon = weapon;
    database.addItemTemplate(emptyWeapon);

    auto& party = stateManager.getState().player.party[0];
    party.inventory.pushBack({.itemName = "EmptySwing", .id = "empty1", .quantity = 1});
    party.equipment.weapon0Id = "empty1";
    party.equipment.weapon1Id.clear();
    auto* enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    const auto hpBefore =
        enemy ? model::getCharacterHp(stateManager.getState().player, *enemy) : 0;
    const auto particlesBefore =
        static_cast<int>(stateManager.getState().world.activeMap.damageParticles.size());
    stateManager.getState().soundsToPlay.clear();
    stateManager.enqueueAction(state::makeAction<state::actions::PerformMeleeAttack>(
                                   "ally-1", "enemy-1"),
                               0);
    pumpActions(stateManager);
    enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    if (enemy) {
      ok = assertEqual(model::getCharacterHp(stateManager.getState().player, *enemy),
                       hpBefore,
                       "empty attacks no HP change") &&
           ok;
    }
    ok = assertTrue(!stateManager.getState().soundsToPlay.contains(
                        bmin::String("should_not_play")),
                    "empty attacks skip sound") &&
         ok;
    ok = assertEqual(
        static_cast<int>(stateManager.getState().world.activeMap.damageParticles.size()),
        particlesBefore,
        "empty attacks skip particle") &&
         ok;
  }

  {
    auto quietWeapon = model::ItemTemplate{};
    quietWeapon.name = "QuietKnife";
    quietWeapon.itemType = model::ItemType::WEAPON_MELEE;
    auto weapon = model::ItemWeaponConfig{};
    weapon.abilityName = "MELEE_EMPTY_DEPICTION";
    quietWeapon.weapon = weapon;
    database.addItemTemplate(quietWeapon);

    auto& party = stateManager.getState().player.party[0];
    party.inventory.pushBack({.itemName = "QuietKnife", .id = "quiet1", .quantity = 1});
    party.equipment.weapon0Id = "quiet1";
    party.equipment.weapon1Id.clear();
    auto* enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    const auto hpBefore =
        enemy ? model::getCharacterHp(stateManager.getState().player, *enemy) : 0;
    const auto particlesBefore =
        static_cast<int>(stateManager.getState().world.activeMap.damageParticles.size());
    stateManager.getState().soundsToPlay.clear();
    stateManager.enqueueAction(state::makeAction<state::actions::PerformMeleeAttack>(
                                   "ally-1", "enemy-1"),
                               0);
    pumpActions(stateManager);
    enemy = findOnActiveMap(stateManager.getState().world.activeMap, "enemy-1");
    if (enemy) {
      ok = assertEqual(model::getCharacterHp(stateManager.getState().player, *enemy),
                       hpBefore - 3,
                       "empty depiction still deals damage") &&
           ok;
    }
    ok = assertTrue(stateManager.getState().soundsToPlay.empty(),
                    "empty depiction strings not queued") &&
         ok;
    ok = assertEqual(
        static_cast<int>(stateManager.getState().world.activeMap.damageParticles.size()),
        particlesBefore,
        "empty dmgAnim skips particle") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestCombatMeleeResolve assertions failed" << LOG_ENDL;
    return 1;
  }
  LOG(INFO) << "TestCombatMeleeResolve completed successfully" << LOG_ENDL;
  return 0;
}
