#include "db/Database.h"
#include "game/combat/ShootRules.h"
#include "game/map/Camera.h"
#include "model/Combat.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "model/templates/Abilities.hpp"
#include "model/templates/AbilityTypes.h"
#include "model/templates/Items.h"
#include "sdl2w/Logger.h"
#include "state/DatabaseInterface.h"
#include "state/StateManager.h"
#include "actions/combat/PerformShoot.hpp"
#include "bmin/String.h"

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

model::AbilityTemplate makeShootAbility(const bmin::String& name,
                                        model::AbilityCostType costType,
                                        int costValue,
                                        int apCost) {
  auto ability = model::AbilityTemplate{};
  ability.name = name;
  ability.apCost = apCost;
  ability.costType = costType;
  ability.costValue = costValue;
  ability.targetSelect.targetType = model::TargetSelectType::TARGET_UNIT;
  ability.targetSelect.allegianceSelectType =
      model::TargetAllegianceSelectType::TARGET_ALLEGIANCE_OTHER;
  ability.targetSelect.range = 1;
  ability.targetSelect.zoneSize = {1, 1};
  ability.depiction.dmgAnim = "splash_attack";
  ability.depiction.dmgSound = "hit_arrow";
  ability.depiction.startSound = "shoot_bow";
  ability.depiction.projectileType = model::ProjectileType::ARROW_NORMAL;
  ability.depiction.projectilePath = model::ProjectilePath::PROJECTILE_PATH_SHORT;
  auto attack = model::AbilityAttack{};
  attack.attackClass = model::AttackClass::ATTACK_CLASS_AUTO_HIT;
  attack.damageType = model::DamageType::DAMAGE_TYPE_PIERCING;
  auto dmg = model::AbilityAttackDmg{};
  dmg.dmgDice = {model::Dice::D0};
  dmg.dmgBonus = 1;
  dmg.dmgStat = model::StatsEnum::STAT_STR;
  dmg.dmgStatMult = 0.f;
  attack.dmg = dmg;
  ability.attacks.pushBack(attack);
  return ability;
}

void addRangedItem(db::Database& database,
                   const bmin::String& name,
                   const bmin::String& abilityName,
                   int dmgBonus) {
  auto item = model::ItemTemplate{};
  item.name = name;
  item.itemType = model::ItemType::WEAPON_RANGED;
  auto weapon = model::ItemWeaponConfig{};
  weapon.abilityName = abilityName;
  if (dmgBonus != 0) {
    auto dmg = model::AbilityAttackDmg{};
    dmg.dmgDice = {model::Dice::D0};
    dmg.dmgBonus = dmgBonus;
    dmg.dmgStat = model::StatsEnum::STAT_STR;
    dmg.dmgStatMult = 0.f;
    weapon.dmgOverrides.pushBack(dmg);
  }
  item.weapon = weapon;
  database.addItemTemplate(item);
}

void addMeleeItem(db::Database& database, const bmin::String& name) {
  auto item = model::ItemTemplate{};
  item.name = name;
  item.itemType = model::ItemType::WEAPON_MELEE;
  auto weapon = model::ItemWeaponConfig{};
  weapon.abilityName = "MELEE_ATTACK_METAL_KNIFE";
  item.weapon = weapon;
  database.addItemTemplate(item);
}

model::CharacterInventoryItem makeInv(const bmin::String& id,
                                      const bmin::String& itemName,
                                      int quantity) {
  return {.itemName = itemName, .id = id, .quantity = quantity};
}

void pumpActions(state::StateManager& stateManager, int maxMs, int& maxParticles, int& maxProjectiles) {
  for (int elapsed = 0; elapsed < maxMs; elapsed += 50) {
    stateManager.update(50);
    const auto particles =
        static_cast<int>(stateManager.getState().world.activeMap.damageParticles.size());
    const auto projectiles =
        static_cast<int>(stateManager.getState().world.activeMap.projectiles.size());
    if (particles > maxParticles) {
      maxParticles = particles;
    }
    if (projectiles > maxProjectiles) {
      maxProjectiles = projectiles;
    }
    const auto& actions = stateManager.getActionData();
    if (actions.sequentialActions.empty() && actions.sequentialActionsNext.empty() &&
        actions.insertActions.empty()) {
      return;
    }
  }
}

model::CharacterInstance* findOnMap(model::ActiveMap& activeMap, const bmin::String& id) {
  for (auto& character : activeMap.characters) {
    if (character.id == id) {
      return &character;
    }
  }
  return nullptr;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestShootRules" << LOG_ENDL;
  auto ok = true;

  db::Database database;
  database.addAbilityTemplate(
      makeShootAbility("SHOOT_AMMO", model::AbilityCostType::ABILITY_COST_AMMO, 1, 9));
  database.addAbilityTemplate(
      makeShootAbility("SHOOT_OFF", model::AbilityCostType::ABILITY_COST_NONE, 0, 0));
  addRangedItem(database, "Longbow", "SHOOT_AMMO", 4);
  addRangedItem(database, "Shortbow", "SHOOT_OFF", 0);
  addMeleeItem(database, "Dagger");

  model::Player player;
  model::CharacterPlayer member;
  member.instanceId = "hero";
  member.currentHp = 8;
  member.currentMp = 3;
  member.inventory = {
      makeInv("bow-main", "Longbow", 1),
      makeInv("bow-off", "Shortbow", 1),
      makeInv("dagger1", "Dagger", 1),
      makeInv("arrows1", "ArrowsIron", 2),
  };
  player.party.pushBack(std::move(member));

  auto shooter = model::CharacterInstance{};
  shooter.id = "hero";
  shooter.x = 0;
  shooter.y = 0;
  shooter.currentAp = 4;

  {
    player.party[0].equipment.weapon0Id = "dagger1";
    player.party[0].equipment.weapon1Id.clear();
    const auto resolved = game::resolveEquippedShootAbility(player, shooter, database);
    ok = assertTrue(!resolved.has_value(), "dagger does not grant shoot") && ok;
  }

  {
    player.party[0].equipment.weapon0Id = "dagger1";
    player.party[0].equipment.weapon1Id = "bow-off";
    const auto resolved = game::resolveEquippedShootAbility(player, shooter, database);
    ok = assertTrue(resolved.has_value() && resolved->ability.name == "SHOOT_OFF",
                    "off-hand bow is used when main hand is melee") &&
         ok;
  }

  {
    player.party[0].equipment.weapon0Id = "bow-main";
    player.party[0].equipment.weapon1Id = "bow-off";
    const auto resolved = game::resolveEquippedShootAbility(player, shooter, database);
    ok = assertTrue(resolved.has_value() && resolved->ability.name == "SHOOT_AMMO",
                    "main-hand bow wins over off-hand bow") &&
         ok;
    ok = assertTrue(resolved.has_value() && resolved->ability.attacks[0].dmg.has_value() &&
                        resolved->ability.attacks[0].dmg->dmgBonus == 4,
                    "weapon damage override applied") &&
         ok;
  }

  player.party[0].equipment.weapon0Id = "bow-main";
  player.party[0].equipment.weapon1Id.clear();
  player.party[0].equipment.ammoId.clear();
  shooter.currentAp = 4;
  ok = assertTrue(!game::canStartShoot(player, shooter, database),
                  "ammo cost with empty ammo slot blocks shoot") &&
       ok;

  player.party[0].equipment.ammoId = "arrows1";
  player.party[0].inventory[3].quantity = 1;
  database.addAbilityTemplate(
      makeShootAbility("SHOOT_AMMO", model::AbilityCostType::ABILITY_COST_AMMO, 2, 9));
  ok = assertTrue(!game::canStartShoot(player, shooter, database),
                  "short ammo stack blocks shoot") &&
       ok;

  database.addAbilityTemplate(
      makeShootAbility("SHOOT_AMMO", model::AbilityCostType::ABILITY_COST_AMMO, 1, 9));
  player.party[0].inventory[3].quantity = 1;
  {
    const auto resolved = game::resolveEquippedShootAbility(player, shooter, database);
    ok = assertTrue(resolved.has_value(), "ammo bow still resolves") && ok;
    if (resolved.has_value()) {
      auto spent = game::spendShootAuxCost(player, shooter, resolved->ability);
      ok = assertTrue(spent.ok, "last arrow is payable") && ok;
      ok = assertTrue(player.party[0].equipment.ammoId.empty(),
                      "empty ammo stack clears the ammo slot") &&
           ok;
      ok = assertEqual(static_cast<int>(player.party[0].inventory.size()),
                       3,
                       "empty ammo stack leaves inventory") &&
           ok;
    }
  }

  player.party[0].inventory.pushBack(makeInv("arrows1", "ArrowsIron", 2));
  player.party[0].equipment.ammoId = "arrows1";
  shooter.currentAp = 1;
  ok = assertTrue(game::canStartShoot(player, shooter, database),
                  "1 AP can start a shot and go negative") &&
       ok;
  shooter.currentAp = 0;
  ok = assertTrue(!game::canStartShoot(player, shooter, database),
                  "0 AP cannot start a shot") &&
       ok;
  shooter.currentAp = 4;
  ok = assertTrue(game::canStartShoot(player, shooter, database),
                  "4 AP and one arrow per shot can start") &&
       ok;

  database.addAbilityTemplate(
      makeShootAbility("SHOOT_AMMO", model::AbilityCostType::ABILITY_COST_HP, 5, 0));
  player.party[0].currentHp = 4;
  ok = assertTrue(!game::canStartShoot(player, shooter, database),
                  "HP cost above current HP blocks shoot") &&
       ok;
  player.party[0].currentHp = 5;
  ok = assertTrue(game::canStartShoot(player, shooter, database),
                  "HP cost equal to current HP can be paid") &&
       ok;

  database.addAbilityTemplate(
      makeShootAbility("SHOOT_AMMO", model::AbilityCostType::ABILITY_COST_MANA, 4, 0));
  player.party[0].currentMp = 3;
  ok = assertTrue(!game::canStartShoot(player, shooter, database),
                  "mana cost above current MP blocks shoot") &&
       ok;
  player.party[0].currentMp = 4;
  {
    const auto resolved = game::resolveEquippedShootAbility(player, shooter, database);
    ok = assertTrue(resolved.has_value() && game::canStartShoot(player, shooter, database),
                    "mana cost equal to current MP can be paid") &&
         ok;
    if (resolved.has_value()) {
      auto spent = game::spendShootAuxCost(player, shooter, resolved->ability);
      ok = assertTrue(spent.ok && spent.hpCost == 0, "mana spend is not HP") && ok;
      ok = assertEqual(player.party[0].currentMp, 0, "mana spend deducts MP") && ok;
    }
  }

  database.addAbilityTemplate(
      makeShootAbility("SHOOT_AMMO", model::AbilityCostType::ABILITY_COST_NONE, 0, 0));
  player.party[0].equipment.ammoId.clear();
  shooter.currentAp = 3;
  ok = assertTrue(game::canStartShoot(player, shooter, database),
                  "none cost does not require ammo") &&
       ok;

  database.addAbilityTemplate(
      makeShootAbility("SHOOT_AMMO", model::AbilityCostType::ABILITY_COST_COOLDOWN, 1, 0));
  const auto* cooldownAbility = database.findAbilityTemplate("SHOOT_AMMO");
  ok = assertTrue(cooldownAbility != nullptr &&
                      !game::canPayShootAuxCost(player, shooter, *cooldownAbility),
                  "cooldown cannot be paid") &&
       ok;
  ok = assertTrue(!game::canStartShoot(player, shooter, database),
                  "cooldown blocks shoot") &&
       ok;

  database.addAbilityTemplate(
      makeShootAbility("SHOOT_AMMO", model::AbilityCostType::ABILITY_COST_AMMO, 1, 9));
  player.party[0].currentMp = 4;
  player.party[0].currentHp = 10;
  player.party[0].equipment.ammoId = "arrows1";
  for (size_t i = 0; i < player.party[0].inventory.size(); ++i) {
    if (player.party[0].inventory[i].id == "arrows1") {
      player.party[0].inventory[i].quantity = 2;
    }
  }
  shooter.currentAp = 4;

  state::DatabaseInterface::setDatabase(&database);
  state::StateManager stateManager;
  auto& state = stateManager.getState();
  state.player = player;
  state.world.combat.active = true;
  state.world.camera.viewW = 100;
  state.world.camera.viewH = 80;
  state.world.camera.cameraMode = model::CameraMode::Follow;
  state.world.activeMap.characters.pushBack(shooter);
  auto enemy = model::CharacterInstance{};
  enemy.id = "goblin";
  enemy.type = model::CharacterTemplateType::ENEMY;
  enemy.x = 1;
  enemy.y = 0;
  enemy.currentHp = 20;
  enemy.maxHp = 20;
  enemy.hpInitialized = true;
  state.world.activeMap.characters.pushBack(enemy);

  model::SpellTargetInfo target;
  target.tileX = 1;
  target.tileY = 0;
  stateManager.enqueueAction(state::makeAction<state::actions::PerformShoot>("hero", target),
                             0);
  auto maxParticles = 0;
  auto maxProjectiles = 0;
  pumpActions(stateManager, 4000, maxParticles, maxProjectiles);

  auto* shotShooter = findOnMap(state.world.activeMap, "hero");
  auto* shotEnemy = findOnMap(state.world.activeMap, "goblin");
  ok = assertTrue(shotShooter != nullptr && shotEnemy != nullptr, "shot characters remain") &&
       ok;
  if (shotShooter != nullptr && shotEnemy != nullptr) {
    ok = assertEqual(shotShooter->currentAp, 1, "shot spends 3 AP, not the ability apCost") &&
         ok;
    ok = assertEqual(shotEnemy->currentHp, 16, "hit applies overridden damage") && ok;
    auto arrowQuantity = -1;
    for (size_t i = 0; i < state.player.party[0].inventory.size(); ++i) {
      if (state.player.party[0].inventory[i].id == "arrows1") {
        arrowQuantity = state.player.party[0].inventory[i].quantity;
      }
    }
    ok = assertEqual(arrowQuantity, 1, "shot consumes one arrow") && ok;
    ok = assertTrue(state.player.party[0].equipment.ammoId == "arrows1",
                    "remaining arrows stay equipped") &&
         ok;
  }
  ok = assertTrue(maxProjectiles >= 1, "shot spawns a projectile") && ok;
  ok = assertTrue(maxParticles >= 1, "hit spawns a damage splat") && ok;
  const auto returnCam = game::computeCameraFollow(0, 0, 100, 80);
  const auto hitCam = game::computeCameraFollow(1, 0, 100, 80);
  ok = assertTrue(state.world.camera.cameraMode == model::CameraMode::Follow,
                  "AP remaining restores follow after the hit pan") &&
       ok;
  ok = assertEqual(state.world.camera.camX, returnCam.camX, "AP remaining camera returns x") &&
       ok;
  ok = assertEqual(state.world.camera.camY, returnCam.camY, "AP remaining camera returns y") &&
       ok;

  stateManager.enqueueAction(state::makeAction<state::actions::PerformShoot>("hero", target),
                             0);
  pumpActions(stateManager, 4000, maxParticles, maxProjectiles);
  shotShooter = findOnMap(state.world.activeMap, "hero");
  ok = assertTrue(shotShooter != nullptr && shotShooter->currentAp == 1 - model::COMBAT_SHOOT_COST,
                  "second shot from 1 AP goes negative") &&
       ok;
  ok = assertTrue(state.world.camera.cameraMode == model::CameraMode::Controlled &&
                      state.world.camera.camX == hitCam.camX &&
                      state.world.camera.camY == hitCam.camY,
                  "spent shot leaves the camera on the target") &&
       ok;

  if (!ok) {
    LOG(ERROR) << "TestShootRules assertions failed" << LOG_ENDL;
    return 1;
  }
  LOG(INFO) << "TestShootRules completed successfully" << LOG_ENDL;
  return 0;
}
