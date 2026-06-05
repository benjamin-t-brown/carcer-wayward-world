#pragma once

#include <optional>
#include <string>
#include <vector>

namespace model {

enum class StatusEffectCondition {
  CONDITION_ALWAYS,
  CONDITION_TARGET_HP_BELOW_HALF,
  CONDITION_SELF_HP_BELOW_HALF,
  CONDITION_AT_LEAST_ENEMY_IN_RANGE_OF_TARGET,
  CONDITION_HAS_NOT_MOVED_SINCE_LAST_ROUND,
  CONDITION_ATTACK_MISSED,
  CONDITION_FIRST_TIME_ATTACKED,
};

enum class AbilityType {
  ABILITY_ATTACK,
  ABILITY_SPELL,
  ABILITY_SKILL,
  ABILITY_SUB_ATTACK,
};

enum class TargetSelectType {
  TARGET_SELF,
  TARGET_UNIT,
  TARGET_ZONE,
  TARGET_ALL_IN_RANGE,
};

enum class TargetAllegianceSelectType {
  TARGET_ALLEGIANCE_SAME,
  TARGET_ALLEGIANCE_SAME_AND_SELF,
  TARGET_ALLEGIANCE_OTHER,
  TARGET_ALLEGIANCE_ALL,
  TARGET_ALLEGIANCE_ALL_AND_SELF,
};

enum class Dice { D0, D2, D4, D6, D8, D10, D12, D20, D100 };

enum class StatsEnum { STAT_STR, STAT_MND, STAT_CON, STAT_AGI, STAT_LCK };

enum class StatusEventType {
  STATUS_EVENT_ON_APPLIED,
  STATUS_EVENT_ON_ATTACK,
  STATUS_EVENT_ON_ATTACKED_MELEE,
  STATUS_EVENT_ON_ATTACKED_RANGE,
  STATUS_EVENT_ON_ATTACKED_MAGIC,
  STATUS_EVENT_ON_MOVE,
  STATUS_EVENT_ON_TURN_START,
  STATUS_EVENT_ON_TURN_END,
  STATUS_EVENT_ON_ROUND_START,
};

enum class CurrentStatEnum {
  CURRENT_STAT_HP,
  CURRENT_STAT_AP,
  CURRENT_STAT_MANA,
  CURRENT_STAT_AC,
};

enum class StatusActionTargetType {
  STATUS_ACTION_TARGET_SELF,
  STATUS_ACTION_TARGET_ATTACKER_LOCATION,
  STATUS_ACTION_TARGET_LAST_LOCATION,
};

enum class AbilityCostType {
  ABILITY_COST_NONE,
  ABILITY_COST_MANA,
  ABILITY_COST_COOLDOWN,
  ABILITY_COST_HP,
};

enum class AttackClass {
  ATTACK_CLASS_MELEE,
  ATTACK_CLASS_RANGED,
  ATTACK_CLASS_MAGIC,
};

enum class ProjectilePath {
  PROJECTILE_PATH_SHORT,
  PROJECTILE_PATH_MEDIUM,
  PROJECTILE_PATH_TALL,
  PROJECTILE_PATH_NONE,
};

struct TargetSelectInfoPoint {
  int x = 0;
  int y = 0;
};

struct TargetSelectInfo {
  TargetSelectType targetType = TargetSelectType::TARGET_SELF;
  TargetAllegianceSelectType allegianceSelectType =
      TargetAllegianceSelectType::TARGET_ALLEGIANCE_SAME;
  int numTargetableUnits = 1;
  TargetSelectInfoPoint zoneSize;
  int range = 0;
};

struct Stats {
  int STR = 0;
  int MND = 0;
  int CON = 0;
  int AGI = 0;
  int LCK = 0;
};

struct CurrentStats {
  int HP = 0;
  int AP = 0;
  int MANA = 0;
  int AC = 0;
};

struct Resistance {
  AttackClass attackType = AttackClass::ATTACK_CLASS_MELEE;
  int mod = 0;
};

struct AbilitySave {
  StatsEnum saveStat = StatsEnum::STAT_STR;
  int saveBase = 0;
  StatsEnum saveAgainst = StatsEnum::STAT_STR;
  int saveAgainstBase = 0;
};

struct AbilityAttackDmg {
  std::vector<Dice> dmgDice;
  int dmgBonus = 0;
  StatsEnum dmgStat = StatsEnum::STAT_STR;
  int dmgStatMult = 0;
  int attackBonus = 0;
};

struct AbilityAttack {
  AttackClass attackClass = AttackClass::ATTACK_CLASS_MELEE;
  std::optional<AbilityAttackDmg> dmg;
  std::optional<AbilitySave> save;
};

struct AbilityStatus {
  std::string statusEffect;
  std::optional<AbilitySave> save;
};

struct AbilityRestore {
  CurrentStatEnum restoreWhich = CurrentStatEnum::CURRENT_STAT_HP;
  std::vector<Dice> restoreDice;
  int restoreBonus = 0;
  StatsEnum restoreStat = StatsEnum::STAT_STR;
  int restoreStatMult = 0;
};

struct AbilityDepiction {
  std::string dmgAnim;
  bool projectileHasFacing = false;
  std::string projectileAnim;
  ProjectilePath projectilePath = ProjectilePath::PROJECTILE_PATH_NONE;
  std::string startSound;
  std::string dmgSound;
};

StatusEffectCondition statusEffectConditionFromString(const std::string& value);
std::string statusEffectConditionToString(StatusEffectCondition value);

AbilityType abilityTypeFromString(const std::string& value);
std::string abilityTypeToString(AbilityType value);

TargetSelectType targetSelectTypeFromString(const std::string& value);
std::string targetSelectTypeToString(TargetSelectType value);

TargetAllegianceSelectType targetAllegianceSelectTypeFromString(const std::string& value);
std::string targetAllegianceSelectTypeToString(TargetAllegianceSelectType value);

Dice diceFromString(const std::string& value);
std::string diceToString(Dice value);

StatsEnum statsEnumFromString(const std::string& value);
std::string statsEnumToString(StatsEnum value);

StatusEventType statusEventTypeFromString(const std::string& value);
std::string statusEventTypeToString(StatusEventType value);

CurrentStatEnum currentStatEnumFromString(const std::string& value);
std::string currentStatEnumToString(CurrentStatEnum value);

StatusActionTargetType statusActionTargetTypeFromString(const std::string& value);
std::string statusActionTargetTypeToString(StatusActionTargetType value);

AbilityCostType abilityCostTypeFromString(const std::string& value);
std::string abilityCostTypeToString(AbilityCostType value);

AttackClass attackClassFromString(const std::string& value);
std::string attackClassToString(AttackClass value);

ProjectilePath projectilePathFromString(const std::string& value);
std::string projectilePathToString(ProjectilePath value);

} // namespace model
