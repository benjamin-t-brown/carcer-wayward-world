#include "model/SpellRules.h"

#include "bmin/StringInterop.h"
#include "db/Database.h"
#include "model/templates/Abilities.h"
#include "model/stats/CharacterStats.h"
#include <cmath>
#include <cstdlib>

namespace model {

namespace {

int diceSides(Dice dice) {
  switch (dice) {
  case Dice::D0:
    return 0;
  case Dice::D2:
    return 2;
  case Dice::D4:
    return 4;
  case Dice::D6:
    return 6;
  case Dice::D8:
    return 8;
  case Dice::D10:
    return 10;
  case Dice::D12:
    return 12;
  case Dice::D20:
    return 20;
  case Dice::D100:
    return 100;
  }
  return 0;
}

int rollDice(Dice dice) {
  const auto sides = diceSides(dice);
  if (sides <= 0) {
    return 0;
  }
  return 1 + (std::rand() % sides);
}

int rollDiceList(const bmin::DynArray<Dice>& diceList) {
  auto total = 0;
  for (const auto& dice : diceList) {
    total += rollDice(dice);
  }
  return total;
}

int statValue(const CharacterStats& stats, StatsEnum which) {
  switch (which) {
  case StatsEnum::STAT_STR:
    return stats.generic.str;
  case StatsEnum::STAT_MND:
    return stats.generic.mnd;
  case StatsEnum::STAT_CON:
    return stats.generic.con;
  case StatsEnum::STAT_AGI:
    return stats.generic.agi;
  case StatsEnum::STAT_LCK:
    return stats.generic.lck;
  }
  return 0;
}

int computeRestoreAmount(const AbilityRestore& restore, const CharacterStats& stats) {
  return rollDiceList(restore.restoreDice) + restore.restoreBonus +
         (statValue(stats, restore.restoreStat) * restore.restoreStatMult);
}

int computeAttackDamage(const AbilityAttack& attack, const CharacterStats& stats) {
  if (!attack.dmg.has_value()) {
    return 0;
  }
  const auto& dmg = *attack.dmg;
  const float total =
      static_cast<float>(rollDiceList(dmg.dmgDice) + dmg.dmgBonus) +
      static_cast<float>(statValue(stats, dmg.dmgStat)) * dmg.dmgStatMult;
  return static_cast<int>(std::lround(total));
}

} // namespace

bool characterKnowsSpell(const CharacterPlayer& character, std::string_view spellName) {
  for (const auto& known : character.knownSpells) {
    if (known == spellName) {
      return true;
    }
  }
  return false;
}

bool characterHasRequiredRunesEquipped(const CharacterPlayer& character,
                                       const SpellTemplate& spell,
                                       const db::Database& database) {
  (void)database;
  int equippedCounts[kRuneTypeCount] = {};
  for (const auto& runeType : character.equippedRunes) {
    equippedCounts[runeTypeIndex(runeType)]++;
  }

  for (const auto& requiredRune : spell.requiredRunes) {
    if (equippedCounts[runeTypeIndex(requiredRune.type)] < requiredRune.count) {
      return false;
    }
  }
  return true;
}

bool characterHasSpellReady(const CharacterPlayer& character,
                            std::string_view spellName,
                            const db::Database& database) {
  if (!characterKnowsSpell(character, spellName)) {
    return false;
  }
  const auto* spell = database.findSpellTemplate(spellName);
  if (spell == nullptr) {
    return false;
  }
  return characterHasRequiredRunesEquipped(character, *spell, database);
}

bool canMemorizeSpell(const CharacterPlayer& character,
                      std::string_view spellName,
                      const db::Database& database) {
  const auto* spell = database.findSpellTemplate(spellName);
  if (spell == nullptr) {
    return false;
  }
  return characterHasRequiredRunesEquipped(character, *spell, database);
}

bool canEquipSpell(const SpellEquipContext& context, std::string_view spellName) {
  if (context.database.findSpellTemplate(spellName) == nullptr) {
    return false;
  }
  if (!context.playerControlled && !characterKnowsSpell(context.character, spellName)) {
    return false;
  }
  return true;
}

int spellAbilityManaCost(const SpellTemplate& spell, const db::Database& database) {
  const auto* ability = database.findAbilityTemplate(bmin::toStringView(spell.abilityName));
  if (ability == nullptr) {
    return 0;
  }
  if (ability->costType != AbilityCostType::ABILITY_COST_MANA) {
    return 0;
  }
  return ability->costValue;
}

bool canCastSpell(const SpellCastContext& context, std::string_view spellName) {
  const auto* spell = context.database.findSpellTemplate(spellName);
  if (spell == nullptr) {
    return false;
  }
  if (context.character.currentMp < spellAbilityManaCost(*spell, context.database)) {
    return false;
  }
  if (!context.playerControlled) {
    // Ready = known + equipped rune requirements met.
    if (!characterHasSpellReady(context.character, spellName, context.database)) {
      return false;
    }
  }
  return true;
}

SpellReadyResult characterPlayerPrepareSpell(CharacterPlayer& character,
                                             std::string_view spellName,
                                             bool playerControlled,
                                             const db::Database& database) {
  SpellEquipContext context{.character = character,
                            .playerControlled = playerControlled,
                            .database = database};
  if (!canEquipSpell(context, spellName)) {
    return SpellReadyResult::CANNOT_EQUIP;
  }
  for (const auto& ready : character.readySpells) {
    if (ready == spellName) {
      return SpellReadyResult::ALREADY_READY;
    }
  }
  character.readySpells.pushBack(bmin::String(spellName.data(), spellName.size()));
  return SpellReadyResult::PREPARED;
}

SpellReadyResult characterPlayerUnprepareSpell(CharacterPlayer& character,
                                               std::string_view spellName) {
  for (size_t i = 0; i < character.readySpells.size(); ++i) {
    if (character.readySpells[i] == spellName) {
      character.readySpells.erase(i);
      return SpellReadyResult::UNPREPARED;
    }
  }
  return SpellReadyResult::NOT_READY;
}

CastSpellOutcome castSpell(CharacterPlayer& caster,
                           std::string_view spellName,
                           std::string_view targetCharacterId,
                           bool playerControlled,
                           const db::Database& database) {
  CastSpellOutcome outcome;

  SpellCastContext castContext{.character = caster,
                               .playerControlled = playerControlled,
                               .database = database};
  if (!canCastSpell(castContext, spellName)) {
    outcome.result = CastSpellResult::CANNOT_CAST;
    return outcome;
  }

  const auto* spell = database.findSpellTemplate(spellName);
  if (spell == nullptr) {
    outcome.result = CastSpellResult::CANNOT_CAST;
    return outcome;
  }

  const auto* ability = database.findAbilityTemplate(bmin::toStringView(spell->abilityName));
  if (ability == nullptr) {
    outcome.result = CastSpellResult::MISSING_ABILITY;
    return outcome;
  }

  caster.currentMp -= spellAbilityManaCost(*spell, database);

  auto resolvedTarget = bmin::String(targetCharacterId.data(), targetCharacterId.size());
  if (ability->targetSelect.targetType == TargetSelectType::TARGET_SELF ||
      resolvedTarget.empty()) {
    resolvedTarget = caster.instanceId;
  }

  outcome.effects.targetCharacterId = resolvedTarget;
  outcome.effects.casterApCost = ability->apCost;
  outcome.effects.depiction = ability->depiction;

  for (const auto& restore : ability->restores) {
    const auto amount = computeRestoreAmount(restore, caster.stats);
    switch (restore.restoreWhich) {
    case CurrentStatEnum::CURRENT_STAT_HP:
      outcome.effects.hpDelta += amount;
      break;
    case CurrentStatEnum::CURRENT_STAT_MANA:
      outcome.effects.mpDelta += amount;
      break;
    case CurrentStatEnum::CURRENT_STAT_AP:
      outcome.effects.apDelta += amount;
      break;
    case CurrentStatEnum::CURRENT_STAT_AC:
      break;
    }
  }

  for (const auto& attack : ability->attacks) {
    outcome.effects.hpDelta -= computeAttackDamage(attack, caster.stats);
  }

  outcome.result = CastSpellResult::CAST;
  return outcome;
}

} // namespace model
