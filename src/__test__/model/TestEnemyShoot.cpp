#include "db/Database.h"
#include "game/combat/EnemyBehavior.h"
#include "game/map/ActiveMapCharacters.h"
#include "game/combat/ShootRules.h"
#include "game/map/CharacterConstruction.h"
#include "model/Combat.h"
#include "model/instances/CharacterInstance.hpp"
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
#include "actions/combat/PerformShoot.hpp"
#include "bmin/String.h"
#include <cstdlib>

namespace {

bool assertTrue(bool cond, const char* label) {
  if (!cond) {
    LOG(ERROR) << label << " expected true" << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

model::AbilityTemplate makeBow(int range, model::AbilityCostType costType, int costValue) {
  auto ability = model::AbilityTemplate{};
  ability.name = "SHOOT_BOW";
  ability.apCost = 9;
  ability.costType = costType;
  ability.costValue = costValue;
  ability.targetSelect.targetType = model::TargetSelectType::TARGET_UNIT;
  ability.targetSelect.allegianceSelectType =
      model::TargetAllegianceSelectType::TARGET_ALLEGIANCE_OTHER;
  ability.targetSelect.range = range;
  ability.depiction.dmgAnim = "splash_attack";
  ability.depiction.projectileType = model::ProjectileType::ARROW_NORMAL;
  ability.depiction.projectilePath = model::ProjectilePath::PROJECTILE_PATH_SHORT;
  auto attack = model::AbilityAttack{};
  attack.attackClass = model::AttackClass::ATTACK_CLASS_AUTO_HIT;
  attack.damageType = model::DamageType::DAMAGE_TYPE_PIERCING;
  auto dmg = model::AbilityAttackDmg{};
  dmg.dmgDice = {model::Dice::D0};
  dmg.dmgBonus = 4;
  dmg.dmgStatMult = 0.f;
  attack.dmg = dmg;
  ability.attacks.pushBack(attack);
  return ability;
}

void addBowItem(db::Database& database) {
  auto item = model::ItemTemplate{};
  item.name = "LongbowOak";
  item.itemType = model::ItemType::WEAPON_RANGED;
  auto weapon = model::ItemWeaponConfig{};
  weapon.abilityName = "SHOOT_BOW";
  item.weapon = weapon;
  database.addItemTemplate(item);
}

void setupGrid(db::Database& database, state::State& state) {
  auto tileset = model::TilesetTemplate{};
  tileset.name = "test_terrain";
  tileset.spriteBase = "test_terrain";
  auto meta = model::TileMetadata{};
  meta.id = 0;
  meta.isWalkable = true;
  meta.isSeeThrough = true;
  tileset.tiles.pushBack(meta);
  database.addTilesetTemplate(tileset);

  auto map = model::MapInstance{};
  map.id = "enemy_ai_map";
  map.templateName = "enemy_ai_map";
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
      tile.tileId = 0;
      layer.pushBack(tile);
    }
  }
  model::mapLayerAt(model::mapInstanceTiles(map), 0) = std::move(layer);
  state.mapInstances[map.templateName] = std::move(map);

  model::MapGridTemplate grid;
  grid.name = "enemy_ai_grid";
  grid.gridWidth = 1;
  grid.gridHeight = 1;
  grid.mapWidth = 8;
  grid.mapHeight = 8;
  grid.cells = {{"enemy_ai_map"}};
  database.addMapGridTemplate(grid);
  state.world.activeMap.gridId = "enemy_ai_grid";
}

model::CharacterInstance makeEnemy(model::ShootTargeting targeting) {
  auto enemy = model::CharacterInstance{};
  enemy.id = "goblin";
  enemy.type = model::CharacterTemplateType::ENEMY;
  enemy.shootTargeting = targeting;
  enemy.currentAp = 4;
  enemy.x = 0;
  enemy.y = 0;
  return enemy;
}

void giveBow(model::CharacterInstance& enemy, int arrows) {
  model::CharacterInventoryItem bow;
  bow.itemName = "LongbowOak";
  bow.id = "bow";
  bow.quantity = 1;
  model::CharacterInventoryItem ammo;
  ammo.itemName = "ArrowsIron";
  ammo.id = "arrows";
  ammo.quantity = arrows;
  enemy.inventory.pushBack(bow);
  enemy.inventory.pushBack(ammo);
  enemy.weapon0Id = "bow";
  enemy.ammoId = arrows > 0 ? "arrows" : bmin::String{};
  enemy.inventorySeeded = true;
}

model::CharacterPlayer makeMember(const char* id, int hp) {
  auto member = model::CharacterPlayer{};
  member.instanceId = id;
  member.currentHp = hp;
  member.params.combat.hp = 30;
  return member;
}

model::CharacterInstance makeAlly(const char* id, int x, int y) {
  auto ally = model::CharacterInstance{};
  ally.id = id;
  ally.type = model::CharacterTemplateType::TOWNSPERSON;
  ally.x = x;
  ally.y = y;
  ally.maxHp = 30;
  return ally;
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

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestEnemyShoot" << LOG_ENDL;
  auto ok = true;
  std::srand(1);

  db::Database database;
  database.addAbilityTemplate(
      makeBow(2, model::AbilityCostType::ABILITY_COST_AMMO, 1));
  addBowItem(database);
  state::DatabaseInterface::setDatabase(&database);

  state::State state;
  setupGrid(database, state);

  auto enemy = makeEnemy(model::ShootTargeting::RANDOM);
  giveBow(enemy, 5);
  state.player.party.pushBack(makeMember("leader", 20));
  state.player.party.pushBack(makeMember("hurt", 4));
  state.world.activeMap.characters.pushBack(enemy);
  state.world.activeMap.characters.pushBack(makeAlly("leader", 1, 0));
  state.world.activeMap.characters.pushBack(makeAlly("hurt", 2, 0));

  auto* goblin = &state.world.activeMap.characters[0];
  auto decision = game::chooseSeekAndShootCombatAction(
      state.world, state.mapInstances, state.player, *goblin, database);
  ok = assertTrue(decision.choice == game::EnemyShootChoice::Shoot,
                  "in-range archer shoots") &&
       ok;
  ok = assertTrue(decision.targetChId == "leader" || decision.targetChId == "hurt",
                  "random shot is one of the eligible members") &&
       ok;

  goblin->shootTargeting = model::ShootTargeting::LOWEST_HEALTH;
  decision = game::chooseSeekAndShootCombatAction(
      state.world, state.mapInstances, state.player, *goblin, database);
  ok = assertTrue(decision.choice == game::EnemyShootChoice::Shoot &&
                      decision.targetChId == "hurt",
                  "lowest health is the shot target") &&
       ok;

  goblin->shootTargeting = model::ShootTargeting::ONLY_PC;
  state.world.activeMap.characters[1].x = 2;
  state.world.activeMap.characters[2].x = 1;
  decision = game::chooseSeekAndShootCombatAction(
      state.world, state.mapInstances, state.player, *goblin, database);
  ok = assertTrue(decision.choice == game::EnemyShootChoice::Shoot &&
                      decision.targetChId == "leader",
                  "only the main PC is shot when both are in range") &&
       ok;

  database.addAbilityTemplate(makeBow(1, model::AbilityCostType::ABILITY_COST_AMMO, 1));
  state.world.activeMap.characters[1].x = 0;
  state.world.activeMap.characters[1].y = 4;
  state.world.activeMap.characters[2].x = 1;
  state.world.activeMap.characters[2].y = 0;
  decision = game::chooseSeekAndShootCombatAction(
      state.world, state.mapInstances, state.player, *goblin, database);
  ok = assertTrue(decision.choice == game::EnemyShootChoice::Move && decision.dx == 0 &&
                      decision.dy == 1,
                  "only-PC approach ignores the closer ally") &&
       ok;

  goblin->shootTargeting = model::ShootTargeting::RANDOM;
  state.world.activeMap.characters[2].x = 6;
  decision = game::chooseSeekAndShootCombatAction(
      state.world, state.mapInstances, state.player, *goblin, database);
  ok = assertTrue(decision.choice == game::EnemyShootChoice::Move,
                  "out of range takes a step toward the party") &&
       ok;

  goblin->ammoId.clear();
  decision = game::chooseSeekAndShootCombatAction(
      state.world, state.mapInstances, state.player, *goblin, database);
  ok = assertTrue(decision.choice == game::EnemyShootChoice::Unavailable,
                  "missing ammo falls back from shoot") &&
       ok;

  goblin->ammoId = "arrows";
  for (size_t i = 0; i < goblin->inventory.size(); ++i) {
    if (goblin->inventory[i].id == "arrows") {
      goblin->inventory[i].quantity = 0;
    }
  }
  database.addAbilityTemplate(makeBow(1, model::AbilityCostType::ABILITY_COST_AMMO, 1));
  decision = game::chooseSeekAndShootCombatAction(
      state.world, state.mapInstances, state.player, *goblin, database);
  ok = assertTrue(decision.choice == game::EnemyShootChoice::Unavailable,
                  "short ammo stack falls back from shoot") &&
       ok;

  {
    auto seeded = model::CharacterTemplate{};
    seeded.name = "archer";
    seeded.type = model::CharacterTemplateType::ENEMY;
    seeded.shootTargeting = model::ShootTargeting::LOWEST_HEALTH;
    model::CharacterTemplateStartingItem bowItem;
    bowItem.itemName = "LongbowOak";
    bowItem.quantity = 1;
    bowItem.equipSlot = model::CharacterStartingEquipSlot::WEAPON0;
    model::CharacterTemplateStartingItem ammoItem;
    ammoItem.itemName = "ArrowsIron";
    ammoItem.quantity = 10;
    ammoItem.equipSlot = model::CharacterStartingEquipSlot::AMMO;
    seeded.startingInventory.pushBack(bowItem);
    seeded.startingInventory.pushBack(ammoItem);
    auto spawned = model::CharacterInstance{};
    model::applyCharacterTemplateToInstance(spawned, seeded);
    ok = assertTrue(spawned.inventorySeeded && spawned.inventory.size() == 2 &&
                        !spawned.weapon0Id.empty() && spawned.ammoId == spawned.inventory[1].id &&
                        spawned.inventory[1].quantity == 10,
                    "template inventory is equipped once") &&
         ok;
    spawned.inventory[1].quantity = 3;
    model::applyCharacterTemplateToInstance(spawned, seeded);
    ok = assertEqual(spawned.inventory[1].quantity, 3, "reapply does not refill ammo") && ok;
    ok = assertTrue(spawned.shootTargeting == model::ShootTargeting::LOWEST_HEALTH,
                    "shoot targeting is copied") &&
         ok;
  }

  for (size_t i = 0; i < goblin->inventory.size(); ++i) {
    if (goblin->inventory[i].id == "arrows") {
      goblin->inventory[i].quantity = 2;
    }
  }
  goblin->ammoId = "arrows";
  goblin->currentAp = 4;
  database.addAbilityTemplate(makeBow(4, model::AbilityCostType::ABILITY_COST_AMMO, 1));
  state.world.combat.active = true;
  state.world.activeMap.characters[1].x = 1;
  state.world.activeMap.characters[1].y = 0;
  state.world.activeMap.characters[1].currentHp = 20;
  state.world.activeMap.characters[1].maxHp = 30;
  state.world.activeMap.characters[1].hpInitialized = true;

  state::StateManager stateManager;
  stateManager.getState() = state;
  model::SpellTargetInfo shot;
  shot.targetCharacterId = "leader";
  shot.tileX = 1;
  shot.tileY = 0;
  stateManager.enqueueAction(state::makeAction<state::actions::PerformShoot>("goblin", shot), 0);
  pumpActions(stateManager);

  auto* after = game::findCharacterById(stateManager.getState().world.activeMap, "goblin");
  ok = assertTrue(after != nullptr, "enemy remains on the map") && ok;
  if (after != nullptr) {
    ok = assertEqual(after->currentAp, 1, "enemy shot spends 3 AP") && ok;
    auto arrowsLeft = -1;
    for (size_t i = 0; i < after->inventory.size(); ++i) {
      if (after->inventory[i].id == "arrows") {
        arrowsLeft = after->inventory[i].quantity;
      }
    }
    ok = assertEqual(arrowsLeft, 1, "enemy shot spends one arrow from instance inventory") &&
         ok;
    ok = assertTrue(stateManager.getState().player.party[0].inventory.empty(),
                    "enemy ammo is not taken from a party sheet") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestEnemyShoot assertions failed" << LOG_ENDL;
    return 1;
  }
  LOG(INFO) << "TestEnemyShoot completed successfully" << LOG_ENDL;
  return 0;
}
