#pragma once

#include "model/stats/CharacterStats.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace model {

struct CharacterInstance;
struct CharacterPlayer;

enum class CharacterTemplateType {
  TOWNSPERSON,
  TOWNSPERSON_STATIC,
  ENEMY,
  ENEMY_STATIC,
};

enum class CharacterTemplateBehaviorName {
  MOVE_RANDOMLY,
  IMMOBILE,
  IMMOBILE_UNTIL_ENEMY_SPOTTED,
  SEEK_MARKER,
  MOVE_LEFT_RIGHT,
  MOVE_UP_DOWN,
};

enum class CombatBehaviorName {
  SEEK_AND_MELEE,
  SPELLCASTER,
};

enum class CombatSpellAiSelection {
  FIRST,
  RANDOM,
};

enum class CombatSpellAiZoneTargeting {
  DEFAULT,
  /** Center on a random hostile whose tile is in range. */
  RANDOM_IN_RANGE,
  ONLY_PC,
};

enum class CombatSpellAiConditionType {
  ALWAYS,
  SELF_HP_BELOW_PERCENT,
  SELF_HP_ABOVE_PERCENT,
  ZONE_HOSTILE_COUNT_AT_LEAST,
  HAS_STATUS,
  LACKS_STATUS,
  OWN_TURN_AT_MOST,
};

struct CombatSpellAiCondition {
  CombatSpellAiConditionType type = CombatSpellAiConditionType::ALWAYS;
  int value = 0;
  bmin::String status;
};

struct CombatSpellAiRule {
  bmin::String spell;
  int weight = 1;
  CombatSpellAiZoneTargeting zoneTargeting = CombatSpellAiZoneTargeting::DEFAULT;
  bmin::DynArray<CombatSpellAiCondition> conditions;
};

struct CombatSpellAiTurnMode {
  CombatBehaviorName behavior = CombatBehaviorName::SPELLCASTER;
  int weight = 1;
};

struct CharacterTemplateCombatSpellAi {
  CombatSpellAiSelection selection = CombatSpellAiSelection::FIRST;
  CombatSpellAiZoneTargeting zoneTargeting = CombatSpellAiZoneTargeting::DEFAULT;
  int skipChance = 0;
  bmin::DynArray<CombatSpellAiTurnMode> turnModes;
  bmin::DynArray<bmin::String> spells;
  bmin::DynArray<CombatSpellAiRule> rules;
};

CombatSpellAiSelection combatSpellAiSelectionFromString(const bmin::String& value);
bmin::String getStringFromCombatSpellAiSelection(CombatSpellAiSelection selection);
CombatSpellAiZoneTargeting combatSpellAiZoneTargetingFromString(const bmin::String& value);
bmin::String getStringFromCombatSpellAiZoneTargeting(CombatSpellAiZoneTargeting targeting);
CombatSpellAiConditionType combatSpellAiConditionTypeFromString(const bmin::String& value);
bmin::String getStringFromCombatSpellAiConditionType(CombatSpellAiConditionType type);

/** Sprite frame layout on the character's spritesheet. */
enum class CharacterSpriteLayout {
  /** Idle + attack at spriteOffset / spriteOffset+1 (default). */
  Default,
  /** Weapon-combo layout (regular + reserved, singles, duals). */
  WeaponCombos,
};

CharacterSpriteLayout characterSpriteLayoutFromString(const bmin::String& value);
bmin::String getStringFromCharacterSpriteLayout(CharacterSpriteLayout layout);

struct CharacterTemplateTalk {
  bmin::String talkName;
  bmin::String portraitName;
};

struct CharacterTemplateBehavior {
  bmin::String behaviorName;
};

struct CharacterTemplateCombat {
  int hp = 0;
  int mp = 0;
  bmin::DynArray<bmin::String> dropTables;
  /** Optional unarmed melee ability name; empty = use MELEE_ATTACK_DEFAULT. */
  bmin::String defaultAbility;
};

struct CharacterTemplateCombatBehavior {
  CombatBehaviorName town = CombatBehaviorName::SEEK_AND_MELEE;
  CombatBehaviorName combat = CombatBehaviorName::SEEK_AND_MELEE;
};

struct CharacterTemplateSound {
  bmin::String deathSoundName;
};

struct CharacterTemplateStatus {
  bmin::String status;
};

struct CharacterTemplateVision {
  int radius = 0;
};

struct CharacterTemplate {
  CharacterTemplateType type;
  bmin::String name;
  bmin::String label;
  bmin::String spritesheetName;
  bmin::String spriteOffset;
  CharacterSpriteLayout spriteLayout = CharacterSpriteLayout::Default;
  CharacterTemplateTalk talk;
  CharacterTemplateBehavior behavior;
  CharacterStats stats;
  CharacterTemplateCombat combat;
  CharacterTemplateCombatBehavior combatBehavior;
  CharacterTemplateSound sound;
  bmin::DynArray<CharacterTemplateStatus> statuses;
  CharacterTemplateVision vision;
  CharacterTemplateCombatSpellAi combatSpellAi;
  /** SpellTemplate.name values applied when constructing a CharacterPlayer. */
  bmin::DynArray<bmin::String> startingKnownSpells;
  /** Optional prepared subset; only names also in known after apply are kept. */
  bmin::DynArray<bmin::String> startingReadySpells;
};

bmin::String characterGetSprite(const CharacterTemplate& character);
bmin::String characterGetSpriteAtIndexOffset(const CharacterTemplate& characterTemplate,
                                             int indexOffset);

/** Copy AI/faction fields from template onto a map character instance. */
void applyCharacterTemplateToInstance(CharacterInstance& character,
                                      const CharacterTemplate& characterTemplate);

/** Copy starting known/ready spell lists from template onto a party member. */
void applyCharacterTemplateStartingSpells(CharacterPlayer& character,
                                          const CharacterTemplate& characterTemplate);

} // namespace model
