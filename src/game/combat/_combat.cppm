module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <string_view>
#include <string>

export module carcer.game.combat;
export import carcer.model.instances;
export import carcer.model.templates;
export import carcer.db;
export import carcer.game.map;
export import carcer.state;
export import bmin.containers;
import bmin.string_interop;

export {

// --- from game/combat/CombatRunner.h ---

namespace game {

struct CombatRunner {
  const model::World* world;

  CombatRunner(const model::World* world);
  ~CombatRunner();

  void startCombat();
  void startTurn();
  void endTurn();
  void endCombat();
};

void onNewCombatRound(state::State& state);

} // namespace game

// --- from game/combat/Damage.h ---
namespace game {

struct CalculatedAbilityDamageResult {
  bool didHit = true;
  int damage = 0;
  int damageReduced = 0;
};

CalculatedAbilityDamageResult calculateAttackDamage(
    const model::AbilityAttack& attack,
    const model::CharacterInstance& attacker,
    const model::CharacterInstance& target);

CalculatedAbilityDamageResult calculateAbilityDamage(
    const model::AbilityDamage& abilityDamage,
    const model::CharacterInstance& attacker,
    const model::CharacterInstance& victim);

} // namespace game

// --- from game/combat/EnemyBehavior.h ---
namespace game {

bool canEnemySpotPartyAvatar(model::World& world,
                             const model::Player& player,
                             const model::CharacterInstance& enemy);

void updateEnemySpotting(model::World& world, const model::Player& player);

/**
 * Choose one step (dx, dy) for SEEK_AND_MELEE toward (targetX, targetY).
 * Prefer a reachable neighbor that reduces Chebyshev distance. Returns false if none.
 * database is used only for walkability / pathfinding.
 */
bool chooseSeekStepToward(model::ActiveMap& activeMap,
                          const model::CharacterInstance& actor,
                          int targetX,
                          int targetY,
                          const db::Database& database,
                          int& outDx,
                          int& outDy);

/**
 * Combat SEEK_AND_MELEE: if adjacent to a hostile, move into their tile;
 * else step toward the nearest living party member on the map.
 * database is used only for walkability / pathfinding.
 */
bool chooseSeekAndMeleeCombatAction(model::World& world,
                                    const model::Player& player,
                                    const model::CharacterInstance& actor,
                                    const db::Database& database,
                                    int& outDx,
                                    int& outDy);

/**
 * Enqueues timed town enemy AI (seek / melee with combat swing timing).
 * Requires StateManager; sets world.resolvingTownEnemyAi until the sequence ends.
 */
// void runTownEnemyAiAfterPlayerMove(state::State& state, const db::Database& database);

} // namespace game

// --- from game/combat/projectileHelpers.h ---
namespace game {

bmin::String getProjectileFacingSuffix(int dx, int dy);
int getProjectileTravelDurationMs(model::ProjectilePath path);

} // namespace game

// --- from game/combat/SpellRules.h ---
// #include "model/instances/CharacterInstance.h"
// #include "model/instances/CharacterPlayer.h"
// #include "model/instances/Player.h"
// #include "model/instances/World.h"
// #include "model/stats/CharacterStats.h"
// #include "model/templates/AbilityTypes.h"
// #include "model/templates/Spells.h"
// #include <string_view>

// class Database;
// }

namespace model {
int spellAbilityManaCost(const SpellTemplate& spell, const db::Database& database);
}

// struct SpellEquipContext {
//   const CharacterPlayer& character;
//   bool playerControlled = false;
//   const db::Database& database;
// };

// struct SpellCastContext {
//   const CharacterPlayer& character;
//   bool playerControlled = false;
//   const db::Database& database;
// };

// /**
//  * Generic combat/map caster: MP + stats for any map character.
//  * partySheet is set only for party members (rune/known gates).
//  */
// struct SpellCasterRef {
//   const bmin::String* id = nullptr;
//   int* currentMp = nullptr;
//   const CharacterStats* stats = nullptr;
//   CharacterPlayer* partySheet = nullptr;
// };

// bool characterKnowsSpell(const CharacterPlayer& character, std::string_view spellName);

// /**
//  * Tallies RuneType occurrences in character.equippedRunes against spell.requiredRunes.
//  * Database is unused (no item-name lookup); kept for call-site compatibility.
//  */
// bool characterHasRequiredRunesEquipped(const CharacterPlayer& character,
//                                        const SpellTemplate& spell,
//                                        const db::Database& database);

// /**
//  * Spell is Ready when the character knows it and equipped runes meet requiredRunes.
//  * (Ready is derived — not a separate prepared list.)
//  */
// bool characterHasSpellReady(const CharacterPlayer& character,
//                             std::string_view spellName,
//                             const db::Database& database);

// /**
//  * Gate for adding a spell to knownSpells.
//  * false if missing template or required rune type tallies are not met from
//  equippedRunes.
//  * No playerControlled bypass — memorize always requires runes (starting known lists at
//  * character load remain authorial and are not gated through this helper).
//  */
// bool canMemorizeSpell(const CharacterPlayer& character,
//                       std::string_view spellName,
//                       const db::Database& database);

// /** false if unknown spell; if !playerControlled && !knows → false; else true. No rune
// gate. */ bool canEquipSpell(const SpellEquipContext& context, std::string_view
// spellName);

// /**
//  * Mana cost for casting a spell comes from the linked ability
//  * (`ABILITY_COST_MANA` → `costValue`; otherwise 0). Missing ability → 0.
//  */
// int spellAbilityManaCost(const SpellTemplate& spell, const db::Database& database);

// /**
//  * false if missing template / insufficient MP (always check MP from linked ability).
//  * if !playerControlled: require known + equipped rune requirements (Ready).
//  * if playerControlled: bypass know/ready/rune gates; still check MP + template exists.
//  */
// bool canCastSpell(const SpellCastContext& context, std::string_view spellName);

// /**
//  * Generic caster gate: always checks MP.
//  * If partySheet != nullptr, also requires known + equipped runes (Ready).
//  * If partySheet == nullptr (NPC), mana only.
//  */
// bool canCastSpell(const SpellCasterRef& caster,
//                   std::string_view spellName,
//                   const db::Database& database);

// /**
//  * Resolve a map character into a SpellCasterRef.
//  * Party members use CharacterPlayer MP/stats/runes; NPCs use map MP + template stats
//  * (written into npcStatsStorage). Returns false if the caster cannot be resolved
//  * (party id missing sheet, or NPC missing template).
//  */
// bool resolveCombatSpellCaster(Player& player,
//                               CharacterInstance& mapCharacter,
//                               const db::Database& database,
//                               CharacterStats& npcStatsStorage,
//                               SpellCasterRef& out);

// enum class SpellReadyResult {
//   PREPARED,
//   UNPREPARED,
//   ALREADY_READY,
//   NOT_READY,
//   CANNOT_EQUIP,
// };

// /** Move spell into readySpells when canEquipSpell allows. */
// SpellReadyResult characterPlayerPrepareSpell(CharacterPlayer& character,
//                                              std::string_view spellName,
//                                              bool playerControlled,
//                                              const db::Database& database);

// /** Remove spell from readySpells (no-op if not ready). */
// SpellReadyResult characterPlayerUnprepareSpell(CharacterPlayer& character,
//                                                std::string_view spellName);

// enum class CastSpellResult {
//   CAST,
//   CANNOT_CAST,
//   MISSING_ABILITY,
// };

// /** Resolved linked-ability effects for the combat/UI apply step. */
// struct CastSpellResolvedEffects {
//   bmin::String targetCharacterId;
//   int hpDelta = 0;
//   int mpDelta = 0;
//   int apDelta = 0;
//   int casterApCost = 0;
//   AbilityDepiction depiction;
// };

// struct CastSpellOutcome {
//   CastSpellResult result = CastSpellResult::CANNOT_CAST;
//   CastSpellResolvedEffects effects;
// };

// /**
//  * Resolve spell → ability, call canCastSpell, spend linked ability mana cost on
//  * success, and compute linked ability restore/attack deltas. Does not mutate
//  * target HP/AP on the map — callers apply those (e.g. ModifyHP / ModifyAP).
//  * Status effects are ignored for v1. Empty targetCharacterId (or TARGET_SELF)
//  * resolves to caster id.
//  */
// CastSpellOutcome castSpell(const SpellCasterRef& caster,
//                            std::string_view spellName,
//                            std::string_view targetCharacterId,
//                            const db::Database& database);

// /** Party/UI convenience: playerControlled true bypasses rune gates (mana still
// checked). */ CastSpellOutcome castSpell(CharacterPlayer& caster,
//                            std::string_view spellName,
//                            std::string_view targetCharacterId,
//                            bool playerControlled,
//                            const db::Database& database);

// /**
//  * Combat cast from a map character. Party sheet (if any) supplies runes + party MP;
//  * NPCs check mana only on the map instance.
//  */
// CastSpellOutcome castSpell(Player& player,
//                            CharacterInstance& mapCaster,
//                            std::string_view spellName,
//                            std::string_view targetCharacterId,
//                            const db::Database& database);

// struct CombatZoneCastHit {
//   bmin::String characterId;
//   int hpDelta = 0;
// };

// struct CombatZoneCastOutcome {
//   CastSpellResult result = CastSpellResult::CANNOT_CAST;
//   int casterApCost = 0;
//   AbilityDepiction depiction;
//   int zoneW = 1;
//   int zoneH = 1;
//   int originX = 0;
//   int originY = 0;
//   bmin::DynArray<CombatZoneCastHit> hits;
// };

// /**
//  * Combat TARGET_ZONE cast for a generic SpellCasterRef.
//  * Party sheet → mana + runes; no party sheet → mana only.
//  */
// CombatZoneCastOutcome castCombatZoneSpell(const SpellCasterRef& caster,
//                                           std::string_view spellName,
//                                           int originX,
//                                           int originY,
//                                           const World& world,
//                                           const db::Database& database);

// /**
//  * Combat TARGET_ZONE cast from a map character (preferred combat entry point).
//  */
// CombatZoneCastOutcome castCombatZoneSpell(Player& player,
//                                           CharacterInstance& mapCaster,
//                                           std::string_view spellName,
//                                           int originX,
//                                           int originY,
//                                           const World& world,
//                                           const db::Database& database);

// /**
//  * Test/UI convenience overload. When requireReadyRunes is true, treats caster as a
//  * party sheet (rune gates); when false, mana only.
//  */
// CombatZoneCastOutcome castCombatZoneSpell(CharacterPlayer& caster,
//                                           std::string_view spellName,
//                                           int originX,
//                                           int originY,
//                                           const World& world,
//                                           const db::Database& database,
//                                           bool requireReadyRunes = true);

// /** Travel duration for a projectile path over Chebyshev tile distance (min 1). */
// int projectileTravelDurationMs(ProjectilePath path, int chebyshevDistance);

// } // namespace model

// --- from game/diceHelpers.h ---
namespace game {

int getNumDiceSides(model::Dice dice);
int rollDice(model::Dice dice);
int rollDiceList(const bmin::DynArray<model::Dice>& diceList);

} // namespace game

} // export

namespace game {

void onNewCombatRound(state::State& state) {
  model::resetAllCombatAp(state.world, model::COMBAT_STARTING_AP);
  advanceWorldMovementTicks(state, TILE_FIELD_MOVES_PER_COMBAT_ROUND);
}

} // namespace game
