#include "db/Database.h"
#include "game/combat/Damage.h"
#include "game/combat/MeleeAttackResolve.h"
#include "game/combat/SpellRules.h"
#include "model/instances/CharacterInstance.hpp"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "model/stats/CharacterStats.h"
#include "model/templates/Abilities.hpp"
#include "model/templates/AbilityTypes.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Items.h"
#include "sdl2w/Logger.h"
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

model::AbilityAttackDmg makeDmg(model::Dice dice,
                                int bonus,
                                float mult,
                                int attackBonus) {
  auto dmg = model::AbilityAttackDmg{};
  dmg.dmgDice = {dice};
  dmg.dmgBonus = bonus;
  dmg.dmgStat = model::StatsEnum::STAT_STR;
  dmg.dmgStatMult = mult;
  dmg.attackBonus = attackBonus;
  return dmg;
}

model::AbilityTemplate makeMeleeAbility(const bmin::String& name,
                                        const model::AbilityAttackDmg& dmg) {
  auto ability = model::AbilityTemplate{};
  ability.name = name;
  auto attack = model::AbilityAttack{};
  attack.attackClass = model::AttackClass::ATTACK_CLASS_MELEE;
  attack.damageType = model::DamageType::DAMAGE_TYPE_EDGED;
  attack.dmg = dmg;
  ability.attacks.pushBack(attack);
  ability.depiction.dmgAnim = "splash_attack";
  ability.depiction.dmgSound = "hit_punch3";
  return ability;
}

void addMeleeItem(db::Database& database,
                  const bmin::String& name,
                  const bmin::String& abilityName,
                  const bmin::DynArray<model::AbilityAttackDmg>& dmgOverrides,
                  model::ItemType itemType = model::ItemType::WEAPON_MELEE) {
  auto item = model::ItemTemplate{};
  item.name = name;
  item.itemType = itemType;
  auto weapon = model::ItemWeaponConfig{};
  weapon.abilityName = abilityName;
  weapon.dmgOverrides = dmgOverrides;
  item.weapon = weapon;
  database.addItemTemplate(item);
}

model::CharacterInventoryItem makeInv(const bmin::String& id, const bmin::String& itemName) {
  return {.itemName = itemName, .id = id, .quantity = 1};
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestMeleeAttackResolve" << LOG_ENDL;
  auto ok = true;

  ok = assertTrue(game::meleeAttackHits(10, 0, 10), "d20 10 vs AC 10 hits") && ok;
  ok = assertTrue(!game::meleeAttackHits(9, 0, 10), "d20 9 vs AC 10 misses") && ok;
  ok = assertTrue(game::meleeAttackHits(9, 1, 10), "d20 9 +1 vs AC 10 hits") && ok;
  ok = assertTrue(game::meleeAttackHits(20, 0, 99), "nat 20 hits") && ok;
  ok = assertTrue(!game::meleeAttackHits(1, 99, 10), "nat 1 misses") && ok;
  ok = assertTrue(game::rollMeleeAttackHit(
                      model::AttackClass::ATTACK_CLASS_AUTO_HIT, 0, 99, false),
                  "AUTO_HIT always hits") &&
       ok;

  {
    auto stats = model::CharacterStats{};
    stats.trainable.weapon.unarmed = 3;
    stats.trainable.weapon.edged = 4;
    stats.trainable.weapon.blunt = 5;
    stats.trainable.weapon.pole = 6;
    stats.trainable.weapon.range = 7;
    auto attack = model::AbilityAttack{};
    attack.attackClass = model::AttackClass::ATTACK_CLASS_MELEE;
    attack.damageType = model::DamageType::DAMAGE_TYPE_EDGED;
    ok = assertEqual(game::weaponMasteryBonus(stats, attack, true),
                     3,
                     "unarmed uses unarmed mastery") &&
         ok;
    ok = assertEqual(game::weaponMasteryBonus(stats, attack, false),
                     4,
                     "edged melee uses edged") &&
         ok;
    attack.damageType = model::DamageType::DAMAGE_TYPE_BASHING;
    ok = assertEqual(game::weaponMasteryBonus(stats, attack, false),
                     5,
                     "bashing uses blunt") &&
         ok;
    attack.damageType = model::DamageType::DAMAGE_TYPE_PIERCING;
    ok = assertEqual(game::weaponMasteryBonus(stats, attack, false),
                     6,
                     "piercing uses pole") &&
         ok;
    attack.attackClass = model::AttackClass::ATTACK_CLASS_RANGED;
    attack.damageType = model::DamageType::DAMAGE_TYPE_PIERCING;
    ok = assertEqual(game::weaponMasteryBonus(stats, attack, false),
                     7,
                     "ranged class uses range") &&
         ok;
    ok = assertEqual(game::meleeTargetArmorClass(stats), 10, "default AC 10") && ok;
  }

  {
    auto attack = model::AbilityAttack{};
    auto stats = model::CharacterStats{};
    stats.generic.str = 9;
    ok = assertEqual(game::calculateAttackDamage(attack, stats).damage,
                     0,
                     "missing dmg yields 0") &&
         ok;

    attack.dmg = makeDmg(model::Dice::D0, 3, 1.f, 0);
    ok = assertEqual(game::calculateAttackDamage(attack, stats).damage,
                     12,
                     "D0+3+STR*1") &&
         ok;
    ok = assertTrue(game::calculateAttackDamage(attack, stats).damage != 10,
                    "damage is not stub 10") &&
         ok;
  }

  db::Database database;
  database.addAbilityTemplate(
      makeMeleeAbility(game::kMeleeAttackDefaultName, makeDmg(model::Dice::D4, 0, 1.f, 0)));
  database.addAbilityTemplate(
      makeMeleeAbility("MELEE_ATTACK_METAL_KNIFE", makeDmg(model::Dice::D4, 0, 1.f, 0)));
  database.addAbilityTemplate(
      makeMeleeAbility("MELEE_ATTACK_METAL_SWORD", makeDmg(model::Dice::D6, 0, 1.f, 0)));

  auto knifeOverrides = bmin::DynArray<model::AbilityAttackDmg>{};
  knifeOverrides.pushBack(makeDmg(model::Dice::D4, 0, 1.f, 1));
  addMeleeItem(database, "DaggerBronze", "MELEE_ATTACK_METAL_KNIFE", knifeOverrides);

  auto swordOverrides = bmin::DynArray<model::AbilityAttackDmg>{};
  swordOverrides.pushBack(makeDmg(model::Dice::D6, 2, 1.f, 1));
  addMeleeItem(database, "SwordBronze", "MELEE_ATTACK_METAL_SWORD", swordOverrides);

  auto twoHandOverrides = bmin::DynArray<model::AbilityAttackDmg>{};
  twoHandOverrides.pushBack(makeDmg(model::Dice::D10, 0, 1.f, 0));
  addMeleeItem(database,
               "GreatSword",
               "MELEE_ATTACK_METAL_SWORD",
               twoHandOverrides,
               model::ItemType::WEAPON_MELEE_2H);

  auto emptyNameOverrides = bmin::DynArray<model::AbilityAttackDmg>{};
  emptyNameOverrides.pushBack(makeDmg(model::Dice::D20, 9, 1.f, 9));
  addMeleeItem(database, "EmptyNameDagger", "", emptyNameOverrides);

  auto missingAbilityOverrides = bmin::DynArray<model::AbilityAttackDmg>{};
  missingAbilityOverrides.pushBack(makeDmg(model::Dice::D12, 8, 1.f, 8));
  addMeleeItem(database, "MissingAbilityDagger", "NO_SUCH_ABILITY", missingAbilityOverrides);

  auto bow = model::ItemTemplate{};
  bow.name = "ShortBow";
  bow.itemType = model::ItemType::WEAPON_RANGED;
  auto bowWeapon = model::ItemWeaponConfig{};
  bowWeapon.abilityName = "RANGED_ATTACK_BOW";
  bow.weapon = bowWeapon;
  database.addItemTemplate(bow);

  auto noWeapon = model::ItemTemplate{};
  noWeapon.name = "BareMelee";
  noWeapon.itemType = model::ItemType::WEAPON_MELEE;
  database.addItemTemplate(noWeapon);

  auto enemyTemplate = model::CharacterTemplate{};
  enemyTemplate.name = "slime";
  enemyTemplate.type = model::CharacterTemplateType::ENEMY;
  enemyTemplate.stats.generic.str = 4;
  enemyTemplate.combat.hp = 20;
  database.addCharacterTemplate(enemyTemplate);

  model::Player player;
  model::CharacterPlayer member;
  member.instanceId = "ally-1";
  member.stats.generic.str = 9;
  member.inventory = {
      makeInv("dagger1", "DaggerBronze"),
      makeInv("sword1", "SwordBronze"),
      makeInv("bow1", "ShortBow"),
      makeInv("empty1", "EmptyNameDagger"),
      makeInv("missing1", "MissingAbilityDagger"),
      makeInv("bare1", "BareMelee"),
      makeInv("great1", "GreatSword"),
      makeInv("unknown1", "NoSuchItem"),
  };
  player.party.pushBack(std::move(member));

  auto ally = model::CharacterInstance{};
  ally.id = "ally-1";
  ally.templateName = "hero";

  auto enemy = model::CharacterInstance{};
  enemy.id = "enemy-1";
  enemy.templateName = "slime";
  enemy.type = model::CharacterTemplateType::ENEMY;

  {
    const auto resolved = game::resolveMeleeAttackAbilities(player, enemy, database);
    ok = assertEqual(static_cast<int>(resolved.size()), 1, "enemy unarmed size") && ok;
    if (!resolved.empty()) {
      ok = assertTrue(resolved[0].ability.name == game::kMeleeAttackDefaultName,
                      "enemy uses default") &&
           ok;
      ok = assertTrue(!resolved[0].offHand, "enemy not off-hand") && ok;
      ok = assertTrue(resolved[0].unarmed, "enemy unarmed") && ok;
    }
  }

  {
    player.party[0].equipment.weapon0Id.clear();
    player.party[0].equipment.weapon1Id.clear();
    const auto resolved = game::resolveMeleeAttackAbilities(player, ally, database);
    ok = assertEqual(static_cast<int>(resolved.size()), 1, "empty hands size") && ok;
    if (!resolved.empty()) {
      ok = assertTrue(resolved[0].ability.name == game::kMeleeAttackDefaultName,
                      "empty hands default") &&
           ok;
      ok = assertTrue(resolved[0].unarmed, "empty hands unarmed") && ok;
    }
  }

  {
    player.party[0].equipment.weapon0Id = "stale-id";
    player.party[0].equipment.weapon1Id.clear();
    const auto resolved = game::resolveMeleeAttackAbilities(player, ally, database);
    ok = assertEqual(static_cast<int>(resolved.size()), 1, "stale id size") && ok;
    if (!resolved.empty()) {
      ok = assertTrue(resolved[0].ability.name == game::kMeleeAttackDefaultName,
                      "stale id default") &&
           ok;
    }
  }

  {
    player.party[0].equipment.weapon0Id = "unknown1";
    player.party[0].equipment.weapon1Id.clear();
    const auto resolved = game::resolveMeleeAttackAbilities(player, ally, database);
    ok = assertEqual(static_cast<int>(resolved.size()), 1, "unknown template size") &&
         ok;
    if (!resolved.empty()) {
      ok = assertTrue(resolved[0].ability.name == game::kMeleeAttackDefaultName,
                      "unknown template default") &&
           ok;
    }
  }

  {
    player.party[0].equipment.weapon0Id = "bare1";
    player.party[0].equipment.weapon1Id.clear();
    const auto resolved = game::resolveMeleeAttackAbilities(player, ally, database);
    ok = assertEqual(static_cast<int>(resolved.size()), 1, "nullopt weapon size") && ok;
    if (!resolved.empty()) {
      ok = assertTrue(resolved[0].ability.name == game::kMeleeAttackDefaultName,
                      "nullopt weapon default") &&
           ok;
    }
  }

  {
    player.party[0].equipment.weapon0Id = "bow1";
    player.party[0].equipment.weapon1Id.clear();
    const auto resolved = game::resolveMeleeAttackAbilities(player, ally, database);
    ok = assertEqual(static_cast<int>(resolved.size()), 1, "ranged only size") && ok;
    if (!resolved.empty()) {
      ok = assertTrue(resolved[0].ability.name == game::kMeleeAttackDefaultName,
                      "ranged skipped to default") &&
           ok;
    }
  }

  {
    player.party[0].equipment.weapon0Id = "dagger1";
    player.party[0].equipment.weapon1Id.clear();
    const auto resolved = game::resolveMeleeAttackAbilities(player, ally, database);
    ok = assertEqual(static_cast<int>(resolved.size()), 1, "one melee size") && ok;
    if (!resolved.empty()) {
      ok = assertTrue(resolved[0].ability.name == "MELEE_ATTACK_METAL_KNIFE",
                      "dagger ability") &&
           ok;
      ok = assertTrue(!resolved[0].offHand, "single weapon not off-hand") && ok;
      ok = assertTrue(!resolved[0].unarmed, "dagger is not unarmed") && ok;
      ok = assertTrue(resolved[0].ability.attacks[0].dmg.has_value() &&
                          resolved[0].ability.attacks[0].dmg->attackBonus == 1,
                      "dagger override applied") &&
           ok;
      const auto* dbKnife = database.findAbilityTemplate("MELEE_ATTACK_METAL_KNIFE");
      ok = assertTrue(dbKnife != nullptr && dbKnife->attacks[0].dmg.has_value() &&
                          dbKnife->attacks[0].dmg->attackBonus == 0,
                      "named ability db unchanged") &&
           ok;
    }
  }

  {
    player.party[0].equipment.weapon0Id = "dagger1";
    player.party[0].equipment.weapon1Id = "sword1";
    const auto resolved = game::resolveMeleeAttackAbilities(player, ally, database);
    ok = assertEqual(static_cast<int>(resolved.size()), 2, "dual-wield size") && ok;
    if (resolved.size() >= 2) {
      ok = assertTrue(resolved[0].ability.name == "MELEE_ATTACK_METAL_KNIFE",
                      "main dagger") &&
           ok;
      ok = assertTrue(!resolved[0].offHand, "main not off-hand") && ok;
      ok = assertTrue(resolved[1].ability.name == "MELEE_ATTACK_METAL_SWORD",
                      "off sword") &&
           ok;
      ok = assertTrue(resolved[1].offHand, "second is off-hand") && ok;
      ok = assertTrue(resolved[1].ability.attacks[0].dmg.has_value() &&
                          resolved[1].ability.attacks[0].dmg->dmgBonus == 2,
                      "sword override applied") &&
           ok;
    }
  }

  {
    player.party[0].equipment.weapon0Id = "bow1";
    player.party[0].equipment.weapon1Id = "dagger1";
    const auto resolved = game::resolveMeleeAttackAbilities(player, ally, database);
    ok = assertEqual(static_cast<int>(resolved.size()), 1, "bow+dagger size") && ok;
    if (!resolved.empty()) {
      ok = assertTrue(resolved[0].ability.name == "MELEE_ATTACK_METAL_KNIFE",
                      "off-slot dagger still full chance hand") &&
           ok;
      ok = assertTrue(!resolved[0].offHand, "lone melee not off-hand") && ok;
    }
  }

  {
    player.party[0].equipment.weapon0Id = "great1";
    player.party[0].equipment.weapon1Id.clear();
    const auto resolved = game::resolveMeleeAttackAbilities(player, ally, database);
    ok = assertEqual(static_cast<int>(resolved.size()), 1, "2H size") && ok;
    if (!resolved.empty()) {
      ok = assertTrue(resolved[0].ability.name == "MELEE_ATTACK_METAL_SWORD",
                      "2H uses named ability") &&
           ok;
      ok = assertTrue(!resolved[0].offHand, "2H not off-hand") && ok;
    }
  }

  {
    player.party[0].equipment.weapon0Id = "empty1";
    player.party[0].equipment.weapon1Id.clear();
    const auto resolved = game::resolveMeleeAttackAbilities(player, ally, database);
    ok = assertEqual(static_cast<int>(resolved.size()), 1, "empty abilityName size") &&
         ok;
    if (!resolved.empty()) {
      ok = assertTrue(resolved[0].ability.name == game::kMeleeAttackDefaultName,
                      "empty abilityName uses default") &&
           ok;
      ok = assertTrue(resolved[0].ability.attacks[0].dmg.has_value() &&
                          resolved[0].ability.attacks[0].dmg->attackBonus == 0,
                      "empty name ignores leftover overrides") &&
           ok;
    }
  }

  {
    player.party[0].equipment.weapon0Id = "missing1";
    player.party[0].equipment.weapon1Id.clear();
    const auto resolved = game::resolveMeleeAttackAbilities(player, ally, database);
    ok = assertEqual(static_cast<int>(resolved.size()), 1, "missing named ability size") &&
         ok;
    if (!resolved.empty()) {
      ok = assertTrue(resolved[0].ability.name == game::kMeleeAttackDefaultName,
                      "missing named ability uses default") &&
           ok;
      ok = assertTrue(resolved[0].ability.attacks[0].dmg.has_value() &&
                          resolved[0].ability.attacks[0].dmg->attackBonus == 0,
                      "missing name ignores leftover overrides") &&
           ok;
    }
  }

  {
    auto npcStats = model::CharacterStats{};
    auto caster = model::SpellCasterRef{};
    ok = assertTrue(model::resolveCombatSpellCaster(
                        player, ally, database, npcStats, caster) &&
                        caster.stats != nullptr && caster.stats->generic.str == 9,
                    "party STR from sheet") &&
         ok;
    auto attack = model::AbilityAttack{};
    attack.dmg = makeDmg(model::Dice::D0, 0, 1.f, 0);
    ok = assertEqual(game::calculateAttackDamage(attack, *caster.stats).damage,
                     9,
                     "party sheet STR damage") &&
         ok;
    ok = assertEqual(ally.stats.generic.str, 0, "instance stats still 0") && ok;
  }

  {
    auto npcStats = model::CharacterStats{};
    auto caster = model::SpellCasterRef{};
    ok = assertTrue(model::resolveCombatSpellCaster(
                        player, enemy, database, npcStats, caster) &&
                        caster.stats != nullptr && caster.stats->generic.str == 4,
                    "enemy STR from template") &&
         ok;
    auto attack = model::AbilityAttack{};
    attack.dmg = makeDmg(model::Dice::D0, 0, 1.f, 0);
    ok = assertEqual(game::calculateAttackDamage(attack, *caster.stats).damage,
                     4,
                     "enemy template STR damage") &&
         ok;
    ok = assertEqual(enemy.stats.generic.str, 0, "enemy instance stats unused") && ok;
  }

  {
    db::Database emptyDb;
    player.party[0].equipment.weapon0Id.clear();
    player.party[0].equipment.weapon1Id.clear();
    const auto resolved = game::resolveMeleeAttackAbilities(player, ally, emptyDb);
    ok = assertEqual(static_cast<int>(resolved.size()),
                     0,
                     "missing default skips hand") &&
         ok;
  }

  if (!ok) {
    LOG(ERROR) << "TestMeleeAttackResolve assertions failed" << LOG_ENDL;
    return 1;
  }
  LOG(INFO) << "TestMeleeAttackResolve completed successfully" << LOG_ENDL;
  return 0;
}
