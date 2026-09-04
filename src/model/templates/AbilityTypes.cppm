module;
#include <cstddef>
#include <cstdint>
#include <utility>
#include <optional>
#include <stdexcept>

export module carcer.model.templates.AbilityTypes;
export import bmin.containers;
import bmin.string_interop;
import sdl2w;
#include "macros.h"

export {

// --- from model/templates/AbilityTypes.h ---
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
  ATTACK_CLASS_AUTO_HIT,
};

enum class DamageType {
  DAMAGE_TYPE_EDGED,
  DAMAGE_TYPE_BASHING,
  DAMAGE_TYPE_PIERCING,
  DAMAGE_TYPE_HEAT,
  DAMAGE_TYPE_FREEZE,
  DAMAGE_TYPE_STATIC,
  DAMAGE_TYPE_NECROTIC,
  DAMAGE_TYPE_EPHEMERAL,
  DAMAGE_TYPE_TRUE,
};

enum class ProjectilePath {
  PROJECTILE_PATH_SHORT,
  PROJECTILE_PATH_MEDIUM,
  PROJECTILE_PATH_TALL,
  PROJECTILE_PATH_NONE,
};

enum class ProjectileType {
  PROJECTILE_NONE,
  DOT_RED,
  DOT_WHITE,
  DOT_BLUE,
  DOT_BIG_YELLOW,
  COMET_BLUE,
  COMET_RED,
  COMET_GREEN,
  ARROW_FIRE,
  ARROW_NORMAL,
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
  DamageType attackType = DamageType::DAMAGE_TYPE_EDGED;
  int mod = 0;
};

struct AbilitySave {
  StatsEnum saveStat = StatsEnum::STAT_STR;
  int saveBase = 0;
  StatsEnum saveAgainst = StatsEnum::STAT_STR;
  int saveAgainstBase = 0;
};

struct AbilityAttackDmg {
  bmin::DynArray<Dice> dmgDice;
  int dmgBonus = 0;
  StatsEnum dmgStat = StatsEnum::STAT_STR;
  float dmgStatMult = 0.f;
  int attackBonus = 0;
};

struct AbilityAttack {
  AttackClass attackClass = AttackClass::ATTACK_CLASS_MELEE;
  DamageType damageType = DamageType::DAMAGE_TYPE_EDGED;
  std::optional<AbilityAttackDmg> dmg;
  std::optional<AbilitySave> save;
};

struct AbilityStatus {
  bmin::String statusEffect;
  std::optional<AbilitySave> save;
  std::optional<int> baseDuration;
  std::optional<int> durationBonus;
};

struct AbilityRestore {
  CurrentStatEnum restoreWhich = CurrentStatEnum::CURRENT_STAT_HP;
  bmin::DynArray<Dice> restoreDice;
  int restoreBonus = 0;
  StatsEnum restoreStat = StatsEnum::STAT_STR;
  int restoreStatMult = 0;
};

struct AbilityDamage {
  DamageType damageType = DamageType::DAMAGE_TYPE_EDGED;
  bmin::DynArray<Dice> dmgDice;
  int dmgBonus = 0;
  StatsEnum dmgStat = StatsEnum::STAT_STR;
  float dmgStatMult = 0.f;
};

struct AbilityDepiction {
  bmin::String dmgAnim;
  ProjectileType projectileType = ProjectileType::PROJECTILE_NONE;
  ProjectilePath projectilePath = ProjectilePath::PROJECTILE_PATH_NONE;
  bmin::String startSound;
  bmin::String dmgSound;
};

StatusEffectCondition statusEffectConditionFromString(const bmin::String& value);
bmin::String statusEffectConditionToString(StatusEffectCondition value);

AbilityType abilityTypeFromString(const bmin::String& value);
bmin::String abilityTypeToString(AbilityType value);

TargetSelectType targetSelectTypeFromString(const bmin::String& value);
bmin::String targetSelectTypeToString(TargetSelectType value);

TargetAllegianceSelectType targetAllegianceSelectTypeFromString(const bmin::String& value);
bmin::String targetAllegianceSelectTypeToString(TargetAllegianceSelectType value);

Dice diceFromString(const bmin::String& value);
bmin::String diceToString(Dice value);

StatsEnum statsEnumFromString(const bmin::String& value);
bmin::String statsEnumToString(StatsEnum value);

StatusEventType statusEventTypeFromString(const bmin::String& value);
bmin::String statusEventTypeToString(StatusEventType value);

CurrentStatEnum currentStatEnumFromString(const bmin::String& value);
bmin::String currentStatEnumToString(CurrentStatEnum value);

StatusActionTargetType statusActionTargetTypeFromString(const bmin::String& value);
bmin::String statusActionTargetTypeToString(StatusActionTargetType value);

AbilityCostType abilityCostTypeFromString(const bmin::String& value);
bmin::String abilityCostTypeToString(AbilityCostType value);

AttackClass attackClassFromString(const bmin::String& value);
bmin::String attackClassToString(AttackClass value);

DamageType damageTypeFromString(const bmin::String& value);
bmin::String damageTypeToString(DamageType value);

ProjectilePath projectilePathFromString(const bmin::String& value);
bmin::String projectilePathToString(ProjectilePath value);

bool projectileTypeHasFacing(ProjectileType value);
bmin::String projectileTypeToAnimBase(ProjectileType value);
ProjectileType projectileTypeFromString(const bmin::String& value);
ProjectileType projectileTypeFromAnimName(const bmin::String& animName);
bmin::String projectileTypeToString(ProjectileType value);

} // namespace model

} // export

namespace model {

StatusEffectCondition statusEffectConditionFromString(const bmin::String& value) {
  if (value == "CONDITION_ALWAYS") {
    return StatusEffectCondition::CONDITION_ALWAYS;
  }
  if (value == "CONDITION_TARGET_HP_BELOW_HALF") {
    return StatusEffectCondition::CONDITION_TARGET_HP_BELOW_HALF;
  }
  if (value == "CONDITION_SELF_HP_BELOW_HALF") {
    return StatusEffectCondition::CONDITION_SELF_HP_BELOW_HALF;
  }
  if (value == "CONDITION_AT_LEAST_ENEMY_IN_RANGE_OF_TARGET") {
    return StatusEffectCondition::CONDITION_AT_LEAST_ENEMY_IN_RANGE_OF_TARGET;
  }
  if (value == "CONDITION_HAS_NOT_MOVED_SINCE_LAST_ROUND") {
    return StatusEffectCondition::CONDITION_HAS_NOT_MOVED_SINCE_LAST_ROUND;
  }
  if (value == "CONDITION_ATTACK_MISSED") {
    return StatusEffectCondition::CONDITION_ATTACK_MISSED;
  }
  if (value == "CONDITION_FIRST_TIME_ATTACKED") {
    return StatusEffectCondition::CONDITION_FIRST_TIME_ATTACKED;
  }
  throw std::runtime_error(("Invalid StatusEffectCondition: " + value).cStr());
}

bmin::String statusEffectConditionToString(StatusEffectCondition value) {
  switch (value) {
  case StatusEffectCondition::CONDITION_ALWAYS:
    return "CONDITION_ALWAYS";
  case StatusEffectCondition::CONDITION_TARGET_HP_BELOW_HALF:
    return "CONDITION_TARGET_HP_BELOW_HALF";
  case StatusEffectCondition::CONDITION_SELF_HP_BELOW_HALF:
    return "CONDITION_SELF_HP_BELOW_HALF";
  case StatusEffectCondition::CONDITION_AT_LEAST_ENEMY_IN_RANGE_OF_TARGET:
    return "CONDITION_AT_LEAST_ENEMY_IN_RANGE_OF_TARGET";
  case StatusEffectCondition::CONDITION_HAS_NOT_MOVED_SINCE_LAST_ROUND:
    return "CONDITION_HAS_NOT_MOVED_SINCE_LAST_ROUND";
  case StatusEffectCondition::CONDITION_ATTACK_MISSED:
    return "CONDITION_ATTACK_MISSED";
  case StatusEffectCondition::CONDITION_FIRST_TIME_ATTACKED:
    return "CONDITION_FIRST_TIME_ATTACKED";
  }
  throw std::runtime_error("Unknown StatusEffectCondition");
}

AbilityType abilityTypeFromString(const bmin::String& value) {
  if (value == "ABILITY_ATTACK") {
    return AbilityType::ABILITY_ATTACK;
  }
  if (value == "ABILITY_SPELL") {
    return AbilityType::ABILITY_SPELL;
  }
  if (value == "ABILITY_SKILL") {
    return AbilityType::ABILITY_SKILL;
  }
  if (value == "ABILITY_SUB_ATTACK") {
    return AbilityType::ABILITY_SUB_ATTACK;
  }
  throw std::runtime_error(("Invalid AbilityType: " + value).cStr());
}

bmin::String abilityTypeToString(AbilityType value) {
  switch (value) {
  case AbilityType::ABILITY_ATTACK:
    return "ABILITY_ATTACK";
  case AbilityType::ABILITY_SPELL:
    return "ABILITY_SPELL";
  case AbilityType::ABILITY_SKILL:
    return "ABILITY_SKILL";
  case AbilityType::ABILITY_SUB_ATTACK:
    return "ABILITY_SUB_ATTACK";
  }
  throw std::runtime_error("Unknown AbilityType");
}

TargetSelectType targetSelectTypeFromString(const bmin::String& value) {
  if (value == "TARGET_SELF") {
    return TargetSelectType::TARGET_SELF;
  }
  if (value == "TARGET_UNIT" || value == "TARGET_MOVE") {
    return TargetSelectType::TARGET_UNIT;
  }
  if (value == "TARGET_ZONE") {
    return TargetSelectType::TARGET_ZONE;
  }
  if (value == "TARGET_ALL_IN_RANGE") {
    return TargetSelectType::TARGET_ALL_IN_RANGE;
  }
  throw std::runtime_error(("Invalid TargetSelectType: " + value).cStr());
}

bmin::String targetSelectTypeToString(TargetSelectType value) {
  switch (value) {
  case TargetSelectType::TARGET_SELF:
    return "TARGET_SELF";
  case TargetSelectType::TARGET_UNIT:
    return "TARGET_UNIT";
  case TargetSelectType::TARGET_ZONE:
    return "TARGET_ZONE";
  case TargetSelectType::TARGET_ALL_IN_RANGE:
    return "TARGET_ALL_IN_RANGE";
  }
  throw std::runtime_error("Unknown TargetSelectType");
}

TargetAllegianceSelectType
targetAllegianceSelectTypeFromString(const bmin::String& value) {
  if (value == "TARGET_ALLEGIANCE_SAME") {
    return TargetAllegianceSelectType::TARGET_ALLEGIANCE_SAME;
  }
  if (value == "TARGET_ALLEGIANCE_SAME_AND_SELF") {
    return TargetAllegianceSelectType::TARGET_ALLEGIANCE_SAME_AND_SELF;
  }
  if (value == "TARGET_ALLEGIANCE_OTHER") {
    return TargetAllegianceSelectType::TARGET_ALLEGIANCE_OTHER;
  }
  if (value == "TARGET_ALLEGIANCE_ALL") {
    return TargetAllegianceSelectType::TARGET_ALLEGIANCE_ALL;
  }
  if (value == "TARGET_ALLEGIANCE_ALL_AND_SELF") {
    return TargetAllegianceSelectType::TARGET_ALLEGIANCE_ALL_AND_SELF;
  }
  throw std::runtime_error(("Invalid TargetAllegianceSelectType: " + value).cStr());
}

bmin::String targetAllegianceSelectTypeToString(TargetAllegianceSelectType value) {
  switch (value) {
  case TargetAllegianceSelectType::TARGET_ALLEGIANCE_SAME:
    return "TARGET_ALLEGIANCE_SAME";
  case TargetAllegianceSelectType::TARGET_ALLEGIANCE_SAME_AND_SELF:
    return "TARGET_ALLEGIANCE_SAME_AND_SELF";
  case TargetAllegianceSelectType::TARGET_ALLEGIANCE_OTHER:
    return "TARGET_ALLEGIANCE_OTHER";
  case TargetAllegianceSelectType::TARGET_ALLEGIANCE_ALL:
    return "TARGET_ALLEGIANCE_ALL";
  case TargetAllegianceSelectType::TARGET_ALLEGIANCE_ALL_AND_SELF:
    return "TARGET_ALLEGIANCE_ALL_AND_SELF";
  }
  throw std::runtime_error("Unknown TargetAllegianceSelectType");
}

Dice diceFromString(const bmin::String& value) {
  if (value == "D0") {
    return Dice::D0;
  }
  if (value == "D2") {
    return Dice::D2;
  }
  if (value == "D4") {
    return Dice::D4;
  }
  if (value == "D6") {
    return Dice::D6;
  }
  if (value == "D8") {
    return Dice::D8;
  }
  if (value == "D10") {
    return Dice::D10;
  }
  if (value == "D12") {
    return Dice::D12;
  }
  if (value == "D20") {
    return Dice::D20;
  }
  if (value == "D100") {
    return Dice::D100;
  }
  throw std::runtime_error(("Invalid Dice: " + value).cStr());
}

bmin::String diceToString(Dice value) {
  switch (value) {
  case Dice::D0:
    return "D0";
  case Dice::D2:
    return "D2";
  case Dice::D4:
    return "D4";
  case Dice::D6:
    return "D6";
  case Dice::D8:
    return "D8";
  case Dice::D10:
    return "D10";
  case Dice::D12:
    return "D12";
  case Dice::D20:
    return "D20";
  case Dice::D100:
    return "D100";
  }
  throw std::runtime_error("Unknown Dice");
}

StatsEnum statsEnumFromString(const bmin::String& value) {
  if (value == "STAT_STR") {
    return StatsEnum::STAT_STR;
  }
  if (value == "STAT_MND") {
    return StatsEnum::STAT_MND;
  }
  if (value == "STAT_CON") {
    return StatsEnum::STAT_CON;
  }
  if (value == "STAT_AGI") {
    return StatsEnum::STAT_AGI;
  }
  if (value == "STAT_LCK") {
    return StatsEnum::STAT_LCK;
  }
  throw std::runtime_error(("Invalid StatsEnum: " + value).cStr());
}

bmin::String statsEnumToString(StatsEnum value) {
  switch (value) {
  case StatsEnum::STAT_STR:
    return "STAT_STR";
  case StatsEnum::STAT_MND:
    return "STAT_MND";
  case StatsEnum::STAT_CON:
    return "STAT_CON";
  case StatsEnum::STAT_AGI:
    return "STAT_AGI";
  case StatsEnum::STAT_LCK:
    return "STAT_LCK";
  }
  throw std::runtime_error("Unknown StatsEnum");
}

StatusEventType statusEventTypeFromString(const bmin::String& value) {
  if (value == "STATUS_EVENT_ON_APPLIED") {
    return StatusEventType::STATUS_EVENT_ON_APPLIED;
  }
  if (value == "STATUS_EVENT_ON_ATTACK") {
    return StatusEventType::STATUS_EVENT_ON_ATTACK;
  }
  if (value == "STATUS_EVENT_ON_ATTACKED_MELEE") {
    return StatusEventType::STATUS_EVENT_ON_ATTACKED_MELEE;
  }
  if (value == "STATUS_EVENT_ON_ATTACKED_RANGE") {
    return StatusEventType::STATUS_EVENT_ON_ATTACKED_RANGE;
  }
  if (value == "STATUS_EVENT_ON_ATTACKED_MAGIC") {
    return StatusEventType::STATUS_EVENT_ON_ATTACKED_MAGIC;
  }
  if (value == "STATUS_EVENT_ON_MOVE") {
    return StatusEventType::STATUS_EVENT_ON_MOVE;
  }
  if (value == "STATUS_EVENT_ON_TURN_START") {
    return StatusEventType::STATUS_EVENT_ON_TURN_START;
  }
  if (value == "STATUS_EVENT_ON_TURN_END") {
    return StatusEventType::STATUS_EVENT_ON_TURN_END;
  }
  if (value == "STATUS_EVENT_ON_ROUND_START") {
    return StatusEventType::STATUS_EVENT_ON_ROUND_START;
  }
  throw std::runtime_error(("Invalid StatusEventType: " + value).cStr());
}

bmin::String statusEventTypeToString(StatusEventType value) {
  switch (value) {
  case StatusEventType::STATUS_EVENT_ON_APPLIED:
    return "STATUS_EVENT_ON_APPLIED";
  case StatusEventType::STATUS_EVENT_ON_ATTACK:
    return "STATUS_EVENT_ON_ATTACK";
  case StatusEventType::STATUS_EVENT_ON_ATTACKED_MELEE:
    return "STATUS_EVENT_ON_ATTACKED_MELEE";
  case StatusEventType::STATUS_EVENT_ON_ATTACKED_RANGE:
    return "STATUS_EVENT_ON_ATTACKED_RANGE";
  case StatusEventType::STATUS_EVENT_ON_ATTACKED_MAGIC:
    return "STATUS_EVENT_ON_ATTACKED_MAGIC";
  case StatusEventType::STATUS_EVENT_ON_MOVE:
    return "STATUS_EVENT_ON_MOVE";
  case StatusEventType::STATUS_EVENT_ON_TURN_START:
    return "STATUS_EVENT_ON_TURN_START";
  case StatusEventType::STATUS_EVENT_ON_TURN_END:
    return "STATUS_EVENT_ON_TURN_END";
  case StatusEventType::STATUS_EVENT_ON_ROUND_START:
    return "STATUS_EVENT_ON_ROUND_START";
  }
  throw std::runtime_error("Unknown StatusEventType");
}

CurrentStatEnum currentStatEnumFromString(const bmin::String& value) {
  if (value == "CURRENT_STAT_HP") {
    return CurrentStatEnum::CURRENT_STAT_HP;
  }
  if (value == "CURRENT_STAT_AP") {
    return CurrentStatEnum::CURRENT_STAT_AP;
  }
  if (value == "CURRENT_STAT_MANA") {
    return CurrentStatEnum::CURRENT_STAT_MANA;
  }
  if (value == "CURRENT_STAT_AC") {
    return CurrentStatEnum::CURRENT_STAT_AC;
  }
  throw std::runtime_error(("Invalid CurrentStatEnum: " + value).cStr());
}

bmin::String currentStatEnumToString(CurrentStatEnum value) {
  switch (value) {
  case CurrentStatEnum::CURRENT_STAT_HP:
    return "CURRENT_STAT_HP";
  case CurrentStatEnum::CURRENT_STAT_AP:
    return "CURRENT_STAT_AP";
  case CurrentStatEnum::CURRENT_STAT_MANA:
    return "CURRENT_STAT_MANA";
  case CurrentStatEnum::CURRENT_STAT_AC:
    return "CURRENT_STAT_AC";
  }
  throw std::runtime_error("Unknown CurrentStatEnum");
}

StatusActionTargetType statusActionTargetTypeFromString(const bmin::String& value) {
  if (value == "STATUS_ACTION_TARGET_SELF") {
    return StatusActionTargetType::STATUS_ACTION_TARGET_SELF;
  }
  if (value == "STATUS_ACTION_TARGET_ATTACKER_LOCATION") {
    return StatusActionTargetType::STATUS_ACTION_TARGET_ATTACKER_LOCATION;
  }
  if (value == "STATUS_ACTION_TARGET_LAST_LOCATION") {
    return StatusActionTargetType::STATUS_ACTION_TARGET_LAST_LOCATION;
  }
  throw std::runtime_error(("Invalid StatusActionTargetType: " + value).cStr());
}

bmin::String statusActionTargetTypeToString(StatusActionTargetType value) {
  switch (value) {
  case StatusActionTargetType::STATUS_ACTION_TARGET_SELF:
    return "STATUS_ACTION_TARGET_SELF";
  case StatusActionTargetType::STATUS_ACTION_TARGET_ATTACKER_LOCATION:
    return "STATUS_ACTION_TARGET_ATTACKER_LOCATION";
  case StatusActionTargetType::STATUS_ACTION_TARGET_LAST_LOCATION:
    return "STATUS_ACTION_TARGET_LAST_LOCATION";
  }
  throw std::runtime_error("Unknown StatusActionTargetType");
}

AbilityCostType abilityCostTypeFromString(const bmin::String& value) {
  if (value == "ABILITY_COST_NONE") {
    return AbilityCostType::ABILITY_COST_NONE;
  }
  if (value == "ABILITY_COST_MANA") {
    return AbilityCostType::ABILITY_COST_MANA;
  }
  if (value == "ABILITY_COST_COOLDOWN") {
    return AbilityCostType::ABILITY_COST_COOLDOWN;
  }
  if (value == "ABILITY_COST_HP") {
    return AbilityCostType::ABILITY_COST_HP;
  }
  throw std::runtime_error(("Invalid AbilityCostType: " + value).cStr());
}

bmin::String abilityCostTypeToString(AbilityCostType value) {
  switch (value) {
  case AbilityCostType::ABILITY_COST_NONE:
    return "ABILITY_COST_NONE";
  case AbilityCostType::ABILITY_COST_MANA:
    return "ABILITY_COST_MANA";
  case AbilityCostType::ABILITY_COST_COOLDOWN:
    return "ABILITY_COST_COOLDOWN";
  case AbilityCostType::ABILITY_COST_HP:
    return "ABILITY_COST_HP";
  }
  throw std::runtime_error("Unknown AbilityCostType");
}

AttackClass attackClassFromString(const bmin::String& value) {
  if (value == "ATTACK_CLASS_MELEE") {
    return AttackClass::ATTACK_CLASS_MELEE;
  }
  if (value == "ATTACK_CLASS_RANGED") {
    return AttackClass::ATTACK_CLASS_RANGED;
  }
  if (value == "ATTACK_CLASS_MAGIC") {
    return AttackClass::ATTACK_CLASS_MAGIC;
  }
  if (value == "ATTACK_CLASS_AUTO_HIT") {
    return AttackClass::ATTACK_CLASS_AUTO_HIT;
  }
  throw std::runtime_error(("Invalid AttackClass: " + value).cStr());
}

bmin::String attackClassToString(AttackClass value) {
  switch (value) {
  case AttackClass::ATTACK_CLASS_MELEE:
    return "ATTACK_CLASS_MELEE";
  case AttackClass::ATTACK_CLASS_RANGED:
    return "ATTACK_CLASS_RANGED";
  case AttackClass::ATTACK_CLASS_MAGIC:
    return "ATTACK_CLASS_MAGIC";
  case AttackClass::ATTACK_CLASS_AUTO_HIT:
    return "ATTACK_CLASS_AUTO_HIT";
  }
  throw std::runtime_error("Unknown AttackClass");
}

DamageType damageTypeFromString(const bmin::String& value) {
  if (value == "DAMAGE_TYPE_EDGED") {
    return DamageType::DAMAGE_TYPE_EDGED;
  }
  if (value == "DAMAGE_TYPE_BASHING") {
    return DamageType::DAMAGE_TYPE_BASHING;
  }
  if (value == "DAMAGE_TYPE_PIERCING") {
    return DamageType::DAMAGE_TYPE_PIERCING;
  }
  if (value == "DAMAGE_TYPE_HEAT") {
    return DamageType::DAMAGE_TYPE_HEAT;
  }
  if (value == "DAMAGE_TYPE_FREEZE") {
    return DamageType::DAMAGE_TYPE_FREEZE;
  }
  if (value == "DAMAGE_TYPE_STATIC") {
    return DamageType::DAMAGE_TYPE_STATIC;
  }
  if (value == "DAMAGE_TYPE_NECROTIC") {
    return DamageType::DAMAGE_TYPE_NECROTIC;
  }
  if (value == "DAMAGE_TYPE_EPHEMERAL") {
    return DamageType::DAMAGE_TYPE_EPHEMERAL;
  }
  if (value == "DAMAGE_TYPE_TRUE") {
    return DamageType::DAMAGE_TYPE_TRUE;
  }
  throw std::runtime_error(("Invalid DamageType: " + value).cStr());
}

bmin::String damageTypeToString(DamageType value) {
  switch (value) {
  case DamageType::DAMAGE_TYPE_EDGED:
    return "DAMAGE_TYPE_EDGED";
  case DamageType::DAMAGE_TYPE_BASHING:
    return "DAMAGE_TYPE_BASHING";
  case DamageType::DAMAGE_TYPE_PIERCING:
    return "DAMAGE_TYPE_PIERCING";
  case DamageType::DAMAGE_TYPE_HEAT:
    return "DAMAGE_TYPE_HEAT";
  case DamageType::DAMAGE_TYPE_FREEZE:
    return "DAMAGE_TYPE_FREEZE";
  case DamageType::DAMAGE_TYPE_STATIC:
    return "DAMAGE_TYPE_STATIC";
  case DamageType::DAMAGE_TYPE_NECROTIC:
    return "DAMAGE_TYPE_NECROTIC";
  case DamageType::DAMAGE_TYPE_EPHEMERAL:
    return "DAMAGE_TYPE_EPHEMERAL";
  case DamageType::DAMAGE_TYPE_TRUE:
    return "DAMAGE_TYPE_TRUE";
  }
  throw std::runtime_error("Unknown DamageType");
}

ProjectilePath projectilePathFromString(const bmin::String& value) {
  if (value == "PROJECTILE_PATH_SHORT") {
    return ProjectilePath::PROJECTILE_PATH_SHORT;
  }
  if (value == "PROJECTILE_PATH_MEDIUM") {
    return ProjectilePath::PROJECTILE_PATH_MEDIUM;
  }
  if (value == "PROJECTILE_PATH_TALL") {
    return ProjectilePath::PROJECTILE_PATH_TALL;
  }
  if (value == "PROJECTILE_PATH_NONE") {
    return ProjectilePath::PROJECTILE_PATH_NONE;
  }
  throw std::runtime_error(("Invalid ProjectilePath: " + value).cStr());
}

bmin::String projectilePathToString(ProjectilePath value) {
  switch (value) {
  case ProjectilePath::PROJECTILE_PATH_SHORT:
    return "PROJECTILE_PATH_SHORT";
  case ProjectilePath::PROJECTILE_PATH_MEDIUM:
    return "PROJECTILE_PATH_MEDIUM";
  case ProjectilePath::PROJECTILE_PATH_TALL:
    return "PROJECTILE_PATH_TALL";
  case ProjectilePath::PROJECTILE_PATH_NONE:
    return "PROJECTILE_PATH_NONE";
  }
  throw std::runtime_error("Unknown ProjectilePath");
}

bool projectileTypeHasFacing(ProjectileType value) {
  switch (value) {
  case ProjectileType::COMET_BLUE:
  case ProjectileType::COMET_RED:
  case ProjectileType::COMET_GREEN:
  case ProjectileType::ARROW_FIRE:
  case ProjectileType::ARROW_NORMAL:
    return true;
  case ProjectileType::PROJECTILE_NONE:
  case ProjectileType::DOT_RED:
  case ProjectileType::DOT_WHITE:
  case ProjectileType::DOT_BLUE:
  case ProjectileType::DOT_BIG_YELLOW:
    return false;
  }
  throw std::runtime_error("Unknown ProjectileType");
}

bmin::String projectileTypeToAnimBase(ProjectileType value) {
  switch (value) {
  case ProjectileType::PROJECTILE_NONE:
    return "";
  case ProjectileType::DOT_RED:
    return "dot_red";
  case ProjectileType::DOT_WHITE:
    return "dot_white";
  case ProjectileType::DOT_BLUE:
    return "dot_blue";
  case ProjectileType::DOT_BIG_YELLOW:
    return "dot_big_yellow";
  case ProjectileType::COMET_BLUE:
    return "comet_blue";
  case ProjectileType::COMET_RED:
    return "comet_red";
  case ProjectileType::COMET_GREEN:
    return "comet_green";
  case ProjectileType::ARROW_FIRE:
    return "arrow_fire";
  case ProjectileType::ARROW_NORMAL:
    return "arrow_normal";
  }
  throw std::runtime_error("Unknown ProjectileType");
}

ProjectileType projectileTypeFromString(const bmin::String& value) {
  if (value == "PROJECTILE_NONE") {
    return ProjectileType::PROJECTILE_NONE;
  }
  if (value == "DOT_RED") {
    return ProjectileType::DOT_RED;
  }
  if (value == "DOT_WHITE") {
    return ProjectileType::DOT_WHITE;
  }
  if (value == "DOT_BLUE") {
    return ProjectileType::DOT_BLUE;
  }
  if (value == "DOT_BIG_YELLOW") {
    return ProjectileType::DOT_BIG_YELLOW;
  }
  if (value == "COMET_BLUE") {
    return ProjectileType::COMET_BLUE;
  }
  if (value == "COMET_RED") {
    return ProjectileType::COMET_RED;
  }
  if (value == "COMET_GREEN") {
    return ProjectileType::COMET_GREEN;
  }
  if (value == "ARROW_FIRE") {
    return ProjectileType::ARROW_FIRE;
  }
  if (value == "ARROW_NORMAL") {
    return ProjectileType::ARROW_NORMAL;
  }
  throw std::runtime_error(("Invalid ProjectileType: " + value).cStr());
}

ProjectileType projectileTypeFromAnimName(const bmin::String& animName) {
  if (animName.empty()) {
    return ProjectileType::PROJECTILE_NONE;
  }

  struct ProjectileAnimBase {
    const char* base;
    ProjectileType type;
  };

  static const ProjectileAnimBase bases[] = {
      {"dot_red", ProjectileType::DOT_RED},
      {"dot_white", ProjectileType::DOT_WHITE},
      {"dot_blue", ProjectileType::DOT_BLUE},
      {"dot_big_yellow", ProjectileType::DOT_BIG_YELLOW},
      {"comet_blue", ProjectileType::COMET_BLUE},
      {"comet_red", ProjectileType::COMET_RED},
      {"comet_green", ProjectileType::COMET_GREEN},
      {"arrow_fire", ProjectileType::ARROW_FIRE},
      {"arrow_normal", ProjectileType::ARROW_NORMAL},
  };

  for (const auto& entry : bases) {
    const bmin::String base = entry.base;
    if (animName == base || animName.startsWith((base + "_").cStr())) {
      return entry.type;
    }
  }

  throw std::runtime_error(("Invalid projectile anim name: " + animName).cStr());
}

bmin::String projectileTypeToString(ProjectileType value) {
  switch (value) {
  case ProjectileType::PROJECTILE_NONE:
    return "PROJECTILE_NONE";
  case ProjectileType::DOT_RED:
    return "DOT_RED";
  case ProjectileType::DOT_WHITE:
    return "DOT_WHITE";
  case ProjectileType::DOT_BLUE:
    return "DOT_BLUE";
  case ProjectileType::DOT_BIG_YELLOW:
    return "DOT_BIG_YELLOW";
  case ProjectileType::COMET_BLUE:
    return "COMET_BLUE";
  case ProjectileType::COMET_RED:
    return "COMET_RED";
  case ProjectileType::COMET_GREEN:
    return "COMET_GREEN";
  case ProjectileType::ARROW_FIRE:
    return "ARROW_FIRE";
  case ProjectileType::ARROW_NORMAL:
    return "ARROW_NORMAL";
  }
  throw std::runtime_error("Unknown ProjectileType");
}

} // namespace model
