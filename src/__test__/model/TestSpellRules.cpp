#include <functional>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <string_view>
#include <cassert>
import carcer.game.combat;
import sdl2w;
import bmin.string_interop;
#include "macros.h"

#define TEST_NAME "TestSpellRules"

namespace {

void addHealSelfAbility(db::Database& database) {
  model::AbilityTemplate ability;
  ability.name = "HEAL_SELF";
  ability.type = model::AbilityType::ABILITY_SPELL;
  ability.targetSelect.targetType = model::TargetSelectType::TARGET_SELF;
  ability.apCost = 1;
  ability.costType = model::AbilityCostType::ABILITY_COST_MANA;
  ability.costValue = 4;
  model::AbilityRestore restore;
  restore.restoreWhich = model::CurrentStatEnum::CURRENT_STAT_HP;
  restore.restoreDice = {model::Dice::D0};
  restore.restoreBonus = 10;
  restore.restoreStat = model::StatsEnum::STAT_STR;
  restore.restoreStatMult = 0;
  ability.restores.pushBack(restore);
  database.addAbilityTemplate(ability);
}

void addSpell(db::Database& database,
              const bmin::String& name,
              const bmin::DynArray<model::SpellRuneRequirement>& requiredRunes) {
  model::SpellTemplate spell;
  spell.name = name;
  spell.abilityName = "HEAL_SELF";
  spell.requiredRunes = requiredRunes;
  database.addSpellTemplate(spell);
}

model::SpellEquipContext equipCtx(const model::CharacterPlayer& character,
                                  bool playerControlled,
                                  const db::Database& database) {
  return {.character = character,
          .playerControlled = playerControlled,
          .database = database};
}

model::SpellCastContext castCtx(const model::CharacterPlayer& character,
                                bool playerControlled,
                                const db::Database& database) {
  return {.character = character,
          .playerControlled = playerControlled,
          .database = database};
}

} // namespace

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  LOG(INFO) << "Starting " << TEST_NAME << LOG_ENDL;

  db::Database database;
  addHealSelfAbility(database);
  addSpell(database,
           "SPELL_HEAL_SELF",
           {model::SpellRuneRequirement{model::RuneType::REGROWTH, 1}});
  addSpell(database,
           "SPELL_DOUBLE_HEAT",
           {model::SpellRuneRequirement{model::RuneType::HEAT, 2}});
  addSpell(database, "SPELL_NO_RUNES", {});
  model::CharacterPlayer character;
  character.currentMp = 10;
  // Owned rune pool lives on the character (not bag inventory).
  character.availableRunes = {
      {.type = model::RuneType::REGROWTH, .count = 1},
      {.type = model::RuneType::HEAT, .count = 2},
  };
  // Known/ready intentionally empty — spell is "unknown" to the character.
  character.knownSpells = {};
  character.readySpells = {};
  character.equippedRunes = {};

  // canMemorizeSpell: always requires equipped type tallies (no playerControlled bypass).
  assert(!model::canMemorizeSpell(character, "SPELL_HEAL_SELF", database));
  assert(!model::canMemorizeSpell(character, "SPELL_DOUBLE_HEAT", database));
  assert(model::canMemorizeSpell(character, "SPELL_NO_RUNES", database));
  assert(!model::canMemorizeSpell(character, "SPELL_DOES_NOT_EXIST", database));

  character.equippedRunes = {model::RuneType::REGROWTH};
  assert(model::canMemorizeSpell(character, "SPELL_HEAL_SELF", database));
  assert(!model::canMemorizeSpell(character, "SPELL_DOUBLE_HEAT", database));

  // Type tallies: one HEAT does not satisfy {HEAT, 2}; two do.
  character.equippedRunes = {model::RuneType::HEAT};
  assert(!model::characterHasRequiredRunesEquipped(
      character, *database.findSpellTemplate("SPELL_DOUBLE_HEAT"), database));
  assert(!model::canMemorizeSpell(character, "SPELL_DOUBLE_HEAT", database));
  character.equippedRunes = {model::RuneType::HEAT, model::RuneType::HEAT};
  assert(model::characterHasRequiredRunesEquipped(
      character, *database.findSpellTemplate("SPELL_DOUBLE_HEAT"), database));
  assert(model::canMemorizeSpell(character, "SPELL_DOUBLE_HEAT", database));

  // Reset equipped for playerControlled cast/equip checks.
  character.equippedRunes = {};

  // playerControlled true: can equip a spell the character does not know.
  assert(model::canEquipSpell(equipCtx(character, true, database), "SPELL_HEAL_SELF"));
  // playerControlled true: can cast without runes / ready when MP is ok.
  assert(model::canCastSpell(castCtx(character, true, database), "SPELL_HEAL_SELF"));
  assert(model::canCastSpell(castCtx(character, true, database), "SPELL_DOUBLE_HEAT"));
  assert(model::canCastSpell(castCtx(character, true, database), "SPELL_NO_RUNES"));

  // playerControlled false: cannot equip unknown (not in knownSpells).
  assert(!model::canEquipSpell(equipCtx(character, false, database), "SPELL_HEAL_SELF"));

  // playerControlled false: known but missing required runes → not Ready, cannot cast.
  character.knownSpells = {bmin::String("SPELL_HEAL_SELF"),
                           bmin::String("SPELL_DOUBLE_HEAT")};
  character.equippedRunes = {};
  assert(!model::characterHasSpellReady(character, "SPELL_HEAL_SELF", database));
  assert(!model::canCastSpell(castCtx(character, false, database), "SPELL_HEAL_SELF"));

  // playerControlled false: known + required rune equipped → Ready, can cast.
  character.equippedRunes = {model::RuneType::REGROWTH};
  assert(model::characterHasSpellReady(character, "SPELL_HEAL_SELF", database));
  assert(model::canCastSpell(castCtx(character, false, database), "SPELL_HEAL_SELF"));

  // playerControlled false: tally gate on cast (need two HEAT).
  character.equippedRunes = {model::RuneType::HEAT};
  assert(!model::characterHasSpellReady(character, "SPELL_DOUBLE_HEAT", database));
  assert(!model::canCastSpell(castCtx(character, false, database), "SPELL_DOUBLE_HEAT"));
  character.equippedRunes = {model::RuneType::HEAT, model::RuneType::HEAT};
  assert(model::characterHasSpellReady(character, "SPELL_DOUBLE_HEAT", database));
  assert(model::canCastSpell(castCtx(character, false, database), "SPELL_DOUBLE_HEAT"));

  // MP fail even when playerControlled (bypass does not skip MP).
  character.currentMp = 3;
  assert(!model::canCastSpell(castCtx(character, true, database), "SPELL_HEAL_SELF"));
  character.currentMp = 4;
  assert(model::canCastSpell(castCtx(character, true, database), "SPELL_HEAL_SELF"));

  // Unknown spell name (missing template) fails for equip and cast.
  assert(!model::canEquipSpell(equipCtx(character, true, database), "SPELL_DOES_NOT_EXIST"));
  assert(!model::canEquipSpell(equipCtx(character, false, database), "SPELL_DOES_NOT_EXIST"));
  assert(!model::canCastSpell(castCtx(character, true, database), "SPELL_DOES_NOT_EXIST"));
  assert(!model::canCastSpell(castCtx(character, false, database), "SPELL_DOES_NOT_EXIST"));

  // Knowing the spell allows non-player equip (still no rune gate on equip).
  assert(model::canEquipSpell(equipCtx(character, false, database), "SPELL_HEAL_SELF"));

  // Empty requiredRunes → Ready whenever known.
  character.knownSpells.pushBack(bmin::String("SPELL_NO_RUNES"));
  character.equippedRunes = {};
  assert(model::characterHasSpellReady(character, "SPELL_NO_RUNES", database));
  assert(!model::characterHasSpellReady(character, "SPELL_HEAL_SELF", database));

  // Legacy prepare/unprepare still mutates readySpells list (UI Ready is derived).
  character.readySpells = {};
  assert(model::characterPlayerPrepareSpell(
             character, "SPELL_HEAL_SELF", true, database) ==
         model::SpellReadyResult::PREPARED);
  assert(model::characterPlayerPrepareSpell(
             character, "SPELL_HEAL_SELF", true, database) ==
         model::SpellReadyResult::ALREADY_READY);
  assert(model::characterPlayerUnprepareSpell(character, "SPELL_HEAL_SELF") ==
         model::SpellReadyResult::UNPREPARED);
  assert(model::characterPlayerUnprepareSpell(character, "SPELL_HEAL_SELF") ==
         model::SpellReadyResult::NOT_READY);
  assert(model::characterPlayerPrepareSpell(
             character, "SPELL_DOES_NOT_EXIST", true, database) ==
         model::SpellReadyResult::CANNOT_EQUIP);

  // Rune slot equip / unequip / toggle (dense equippedRunes + availableRunes capacity).
  character.equippedRunes = {};
  assert(model::characterPlayerEquipRuneType(character, model::RuneType::REGROWTH) ==
         model::EquipRuneResult::EQUIPPED);
  assert(character.equippedRunes.size() == 1);
  assert(character.equippedRunes[0] == model::RuneType::REGROWTH);
  // Capacity exhausted for REGROWTH (qty 1) → cannot equip a second.
  assert(model::characterPlayerEquipRuneType(character, model::RuneType::REGROWTH) ==
         model::EquipRuneResult::NO_RUNE_AVAILABLE);

  assert(model::characterPlayerToggleManaSlotRune(character, 0) ==
         model::EquipRuneResult::UNEQUIPPED);
  assert(character.equippedRunes.size() == 0);
  // Empty-slot toggle appends first equippable type in enum order (HEAT).
  assert(model::characterPlayerToggleManaSlotRune(character, 0) ==
         model::EquipRuneResult::EQUIPPED);
  assert(character.equippedRunes.size() == 1);
  assert(character.equippedRunes[0] == model::RuneType::HEAT);

  // Available HEAT qty 2 allows two equipped HEAT via type equip.
  character.equippedRunes = {};
  assert(model::characterPlayerEquipRuneType(character, model::RuneType::HEAT) ==
         model::EquipRuneResult::EQUIPPED);
  assert(model::characterPlayerEquipRuneType(character, model::RuneType::HEAT) ==
         model::EquipRuneResult::EQUIPPED);
  assert(character.equippedRunes.size() == 2);
  assert(character.equippedRunes[0] == model::RuneType::HEAT);
  assert(character.equippedRunes[1] == model::RuneType::HEAT);

  // Equipping out of order still groups same types together (enum sort).
  character.equippedRunes = {};
  assert(model::characterPlayerEquipRuneType(character, model::RuneType::REGROWTH) ==
         model::EquipRuneResult::EQUIPPED);
  assert(model::characterPlayerEquipRuneType(character, model::RuneType::HEAT) ==
         model::EquipRuneResult::EQUIPPED);
  assert(model::characterPlayerEquipRuneType(character, model::RuneType::HEAT) ==
         model::EquipRuneResult::EQUIPPED);
  assert(character.equippedRunes.size() == 3);
  assert(character.equippedRunes[0] == model::RuneType::HEAT);
  assert(character.equippedRunes[1] == model::RuneType::HEAT);
  assert(character.equippedRunes[2] == model::RuneType::REGROWTH);

  // Resume capacity / toggle checks from two HEAT equipped.
  character.equippedRunes = {model::RuneType::HEAT, model::RuneType::HEAT};
  // No remaining HEAT capacity.
  assert(model::characterPlayerEquipRuneType(character, model::RuneType::HEAT) ==
         model::EquipRuneResult::NO_RUNE_AVAILABLE);
  // Gap toggle (slotIndex > size) is invalid.
  assert(model::characterPlayerToggleManaSlotRune(character, 4) ==
         model::EquipRuneResult::INVALID_SLOT);
  // Unequip first slot and compact.
  assert(model::characterPlayerToggleManaSlotRune(character, 0) ==
         model::EquipRuneResult::UNEQUIPPED);
  assert(character.equippedRunes.size() == 1);
  assert(character.equippedRunes[0] == model::RuneType::HEAT);
  // With HEAT capacity remaining, first empty appends HEAT (enum order).
  assert(model::characterPlayerToggleManaSlotRune(character, 1) ==
         model::EquipRuneResult::EQUIPPED);
  assert(character.equippedRunes.size() == 2);
  assert(character.equippedRunes[0] == model::RuneType::HEAT);
  assert(character.equippedRunes[1] == model::RuneType::HEAT);
  // Exhaust HEAT capacity again, then first empty appends REGROWTH.
  assert(model::characterPlayerToggleManaSlotRune(character, 2) ==
         model::EquipRuneResult::EQUIPPED);
  assert(character.equippedRunes.size() == 3);
  assert(character.equippedRunes[2] == model::RuneType::REGROWTH);

  // castSpell: fail cleanly without spending MP when canCastSpell is false.
  character.currentMp = 3;
  auto failedCast =
      model::castSpell(character, "SPELL_HEAL_SELF", "", true, database);
  assert(failedCast.result == model::CastSpellResult::CANNOT_CAST);
  assert(character.currentMp == 3);

  // castSpell: success spends linked ability mana cost and resolves HEAL_SELF (+10 HP).
  character.currentMp = 4;
  auto cast = model::castSpell(character, "SPELL_HEAL_SELF", "", true, database);
  assert(cast.result == model::CastSpellResult::CAST);
  assert(character.currentMp == 0);
  assert(cast.effects.hpDelta == 10);
  assert(cast.effects.casterApCost == 1);
  assert(cast.effects.targetCharacterId == character.instanceId);

  // Missing linked ability fails without spending MP.
  model::SpellTemplate orphanSpell;
  orphanSpell.name = "SPELL_ORPHAN";
  orphanSpell.abilityName = "ABILITY_DOES_NOT_EXIST";
  database.addSpellTemplate(orphanSpell);
  character.currentMp = 5;
  auto missingAbility =
      model::castSpell(character, "SPELL_ORPHAN", "", true, database);
  assert(missingAbility.result == model::CastSpellResult::MISSING_ABILITY);
  assert(character.currentMp == 5);

  LOG(INFO) << "Finished " << TEST_NAME << LOG_ENDL;
  return 0;
}
