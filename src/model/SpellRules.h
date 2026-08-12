#pragma once

#include "model/instances/CharacterPlayer.h"
#include "model/templates/AbilityTypes.h"
#include "model/templates/Spells.h"
#include <string_view>

namespace db {
class Database;
}

namespace model {

struct SpellEquipContext {
  const CharacterPlayer& character;
  bool playerControlled = false;
  const db::Database& database;
};

struct SpellCastContext {
  const CharacterPlayer& character;
  bool playerControlled = false;
  const db::Database& database;
};

bool characterKnowsSpell(const CharacterPlayer& character, std::string_view spellName);

/**
 * Tallies RuneType occurrences in character.equippedRunes against spell.requiredRunes.
 * Database is unused (no item-name lookup); kept for call-site compatibility.
 */
bool characterHasRequiredRunesEquipped(const CharacterPlayer& character,
                                       const SpellTemplate& spell,
                                       const db::Database& database);

/**
 * Spell is Ready when the character knows it and equipped runes meet requiredRunes.
 * (Ready is derived — not a separate prepared list.)
 */
bool characterHasSpellReady(const CharacterPlayer& character,
                            std::string_view spellName,
                            const db::Database& database);

/**
 * Gate for adding a spell to knownSpells.
 * false if missing template or required rune type tallies are not met from equippedRunes.
 * No playerControlled bypass — memorize always requires runes (starting known lists at
 * character load remain authorial and are not gated through this helper).
 */
bool canMemorizeSpell(const CharacterPlayer& character,
                      std::string_view spellName,
                      const db::Database& database);

/** false if unknown spell; if !playerControlled && !knows → false; else true. No rune gate. */
bool canEquipSpell(const SpellEquipContext& context, std::string_view spellName);

/**
 * Mana cost for casting a spell comes from the linked ability
 * (`ABILITY_COST_MANA` → `costValue`; otherwise 0). Missing ability → 0.
 */
int spellAbilityManaCost(const SpellTemplate& spell, const db::Database& database);

/**
 * false if missing template / insufficient MP (always check MP from linked ability).
 * if !playerControlled: require known + equipped rune requirements (Ready).
 * if playerControlled: bypass know/ready/rune gates; still check MP + template exists.
 */
bool canCastSpell(const SpellCastContext& context, std::string_view spellName);

enum class SpellReadyResult {
  PREPARED,
  UNPREPARED,
  ALREADY_READY,
  NOT_READY,
  CANNOT_EQUIP,
};

/** Move spell into readySpells when canEquipSpell allows. */
SpellReadyResult characterPlayerPrepareSpell(CharacterPlayer& character,
                                             std::string_view spellName,
                                             bool playerControlled,
                                             const db::Database& database);

/** Remove spell from readySpells (no-op if not ready). */
SpellReadyResult characterPlayerUnprepareSpell(CharacterPlayer& character,
                                               std::string_view spellName);

enum class CastSpellResult {
  CAST,
  CANNOT_CAST,
  MISSING_ABILITY,
};

/** Resolved linked-ability effects for the combat/UI apply step. */
struct CastSpellResolvedEffects {
  bmin::String targetCharacterId;
  int hpDelta = 0;
  int mpDelta = 0;
  int apDelta = 0;
  int casterApCost = 0;
  AbilityDepiction depiction;
};

struct CastSpellOutcome {
  CastSpellResult result = CastSpellResult::CANNOT_CAST;
  CastSpellResolvedEffects effects;
};

/**
 * Resolve spell → ability, call canCastSpell, spend linked ability mana cost on
 * success, and compute linked ability restore/attack deltas. Does not mutate
 * target HP/AP on the map — callers apply those (e.g. ModifyHP / ModifyAP).
 * Status effects are ignored for v1. Empty targetCharacterId (or TARGET_SELF)
 * resolves to caster.instanceId.
 */
CastSpellOutcome castSpell(CharacterPlayer& caster,
                           std::string_view spellName,
                           std::string_view targetCharacterId,
                           bool playerControlled,
                           const db::Database& database);

} // namespace model
