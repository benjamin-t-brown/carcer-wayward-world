#include "db/Database.h"
#include "game/combat/AbilityOverrideMerge.h"
#include "model/templates/Abilities.hpp"
#include "model/templates/AbilityTypes.h"
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

model::AbilityAttack makeAttack(const model::AbilityAttackDmg& dmg) {
  auto attack = model::AbilityAttack{};
  attack.attackClass = model::AttackClass::ATTACK_CLASS_MELEE;
  attack.damageType = model::DamageType::DAMAGE_TYPE_EDGED;
  attack.dmg = dmg;
  return attack;
}

bool dmgEquals(const model::AbilityAttackDmg& actual,
               const model::AbilityAttackDmg& expected,
               const char* label) {
  auto ok = true;
  ok = assertEqual(static_cast<int>(actual.dmgDice.size()),
                   static_cast<int>(expected.dmgDice.size()),
                   label) &&
       ok;
  if (!actual.dmgDice.empty() && !expected.dmgDice.empty()) {
    ok = assertTrue(actual.dmgDice[0] == expected.dmgDice[0], label) && ok;
  }
  ok = assertEqual(actual.dmgBonus, expected.dmgBonus, label) && ok;
  ok = assertTrue(actual.dmgStat == expected.dmgStat, label) && ok;
  if (actual.dmgStatMult != expected.dmgStatMult) {
    LOG(ERROR) << label << " dmgStatMult mismatch" << LOG_ENDL;
    ok = false;
  }
  ok = assertEqual(actual.attackBonus, expected.attackBonus, label) && ok;
  return ok;
}

} // namespace

int main(int /*argc*/, char** /*argv*/) {
  LOG(INFO) << "Starting TestAbilityOverrideMerge" << LOG_ENDL;
  auto ok = true;

  const auto base0 = makeDmg(model::Dice::D4, 0, 0.5f, 0);
  const auto base1 = makeDmg(model::Dice::D8, 2, 1.f, 3);
  const auto override0 = makeDmg(model::Dice::D6, 1, 1.f, 1);
  const auto extraOverride = makeDmg(model::Dice::D12, 9, 2.f, 9);

  db::Database database;
  auto stored = model::AbilityTemplate{};
  stored.name = "TEST_MERGE_ABILITY";
  stored.attacks.pushBack(makeAttack(base0));
  stored.attacks.pushBack(makeAttack(base1));
  database.addAbilityTemplate(stored);

  auto resolved = *database.findAbilityTemplate("TEST_MERGE_ABILITY");
  auto overrides = bmin::DynArray<model::AbilityAttackDmg>{};
  overrides.pushBack(override0);
  overrides.pushBack(extraOverride);
  overrides.pushBack(extraOverride);
  game::mergeAbilityAttackDmgOverrides(resolved, overrides);

  ok = assertEqual(static_cast<int>(resolved.attacks.size()), 2, "attacks size unchanged") &&
       ok;
  ok = assertTrue(resolved.attacks[0].dmg.has_value(), "attack0 dmg present") && ok;
  ok = assertTrue(resolved.attacks[1].dmg.has_value(), "attack1 dmg present") && ok;
  if (resolved.attacks[0].dmg.has_value()) {
    ok = dmgEquals(*resolved.attacks[0].dmg, override0, "whole-object replace index 0") &&
         ok;
  }
  if (resolved.attacks[1].dmg.has_value()) {
    ok = dmgEquals(*resolved.attacks[1].dmg, extraOverride, "replace present index 1") &&
         ok;
  }

  const auto* dbAbility = database.findAbilityTemplate("TEST_MERGE_ABILITY");
  ok = assertTrue(dbAbility != nullptr, "db ability still present") && ok;
  if (dbAbility != nullptr && dbAbility->attacks.size() >= 2 &&
      dbAbility->attacks[0].dmg.has_value() && dbAbility->attacks[1].dmg.has_value()) {
    ok = dmgEquals(*dbAbility->attacks[0].dmg, base0, "db attack0 unchanged") && ok;
    ok = dmgEquals(*dbAbility->attacks[1].dmg, base1, "db attack1 unchanged") && ok;
  }

  {
    auto copy = *database.findAbilityTemplate("TEST_MERGE_ABILITY");
    auto pad = model::AbilityAttackDmg{};
    auto padOverrides = bmin::DynArray<model::AbilityAttackDmg>{};
    padOverrides.pushBack(pad);
    game::mergeAbilityAttackDmgOverrides(copy, padOverrides);
    ok = assertTrue(copy.attacks[0].dmg.has_value(), "pad leaves dmg") && ok;
    if (copy.attacks[0].dmg.has_value()) {
      ok = dmgEquals(*copy.attacks[0].dmg, base0, "legacy pad skipped") && ok;
      ok = assertTrue(copy.attacks[0].dmg->dmgStatMult == 0.5f,
                      "pad did not fill editor dmgStatMult=1") &&
           ok;
      ok = assertTrue(!copy.attacks[0].dmg->dmgDice.empty() &&
                          copy.attacks[0].dmg->dmgDice[0] == model::Dice::D4,
                      "pad did not fill editor D6") &&
           ok;
    }
  }

  {
    auto copy = *database.findAbilityTemplate("TEST_MERGE_ABILITY");
    auto onlyFirst = bmin::DynArray<model::AbilityAttackDmg>{};
    onlyFirst.pushBack(override0);
    game::mergeAbilityAttackDmgOverrides(copy, onlyFirst);
    ok = assertTrue(copy.attacks[1].dmg.has_value(), "missing override keeps base") && ok;
    if (copy.attacks[1].dmg.has_value()) {
      ok = dmgEquals(*copy.attacks[1].dmg, base1, "index 1 unchanged without override") &&
           ok;
    }
  }

  if (!ok) {
    LOG(ERROR) << "TestAbilityOverrideMerge assertions failed" << LOG_ENDL;
    return 1;
  }
  LOG(INFO) << "TestAbilityOverrideMerge completed successfully" << LOG_ENDL;
  return 0;
}
