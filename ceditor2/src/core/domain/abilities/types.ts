import type { JsonObject } from '../../database/types.js';

export const ABILITY_TYPES = [
  'ABILITY_ATTACK',
  'ABILITY_SPELL',
  'ABILITY_SKILL',
  'ABILITY_SUB_ATTACK',
] as const;

export const TARGET_SELECT_TYPES = [
  'TARGET_SELF',
  'TARGET_UNIT',
  'TARGET_ZONE',
  'TARGET_ALL_IN_RANGE',
] as const;

export const TARGET_ALLEGIANCE_TYPES = [
  'TARGET_ALLEGIANCE_SAME',
  'TARGET_ALLEGIANCE_SAME_AND_SELF',
  'TARGET_ALLEGIANCE_OTHER',
  'TARGET_ALLEGIANCE_ALL',
  'TARGET_ALLEGIANCE_ALL_AND_SELF',
] as const;

export const ABILITY_COST_TYPES = [
  'ABILITY_COST_NONE',
  'ABILITY_COST_MANA',
  'ABILITY_COST_COOLDOWN',
  'ABILITY_COST_HP',
] as const;

export const STAT_TYPES = [
  'STAT_STR',
  'STAT_MND',
  'STAT_CON',
  'STAT_AGI',
  'STAT_LCK',
] as const;

export const CURRENT_STAT_TYPES = [
  'CURRENT_STAT_HP',
  'CURRENT_STAT_AP',
  'CURRENT_STAT_MANA',
  'CURRENT_STAT_AC',
] as const;

export const DICE_TYPES = [
  'D0',
  'D2',
  'D4',
  'D6',
  'D8',
  'D10',
  'D12',
  'D20',
  'D100',
] as const;

export const ATTACK_CLASSES = [
  'ATTACK_CLASS_MELEE',
  'ATTACK_CLASS_RANGED',
  'ATTACK_CLASS_MAGIC',
  'ATTACK_CLASS_AUTO_HIT',
] as const;

export const DAMAGE_TYPES = [
  'DAMAGE_TYPE_EDGED',
  'DAMAGE_TYPE_BASHING',
  'DAMAGE_TYPE_PIERCING',
  'DAMAGE_TYPE_HEAT',
  'DAMAGE_TYPE_FREEZE',
  'DAMAGE_TYPE_STATIC',
  'DAMAGE_TYPE_NECROTIC',
  'DAMAGE_TYPE_EPHEMERAL',
  'DAMAGE_TYPE_TRUE',
] as const;

export const PROJECTILE_TYPES = [
  'PROJECTILE_NONE',
  'DOT_RED',
  'DOT_WHITE',
  'DOT_BLUE',
  'DOT_BIG_YELLOW',
  'COMET_BLUE',
  'COMET_RED',
  'COMET_GREEN',
  'ARROW_FIRE',
  'ARROW_NORMAL',
] as const;

export const PROJECTILE_PATHS = [
  'PROJECTILE_PATH_SHORT',
  'PROJECTILE_PATH_MEDIUM',
  'PROJECTILE_PATH_TALL',
  'PROJECTILE_PATH_NONE',
] as const;

export type AbilityType = (typeof ABILITY_TYPES)[number];
export type TargetSelectType = (typeof TARGET_SELECT_TYPES)[number];
export type TargetAllegianceType = (typeof TARGET_ALLEGIANCE_TYPES)[number];
export type AbilityCostType = (typeof ABILITY_COST_TYPES)[number];
export type StatType = (typeof STAT_TYPES)[number];
export type CurrentStatType = (typeof CURRENT_STAT_TYPES)[number];
export type DieType = (typeof DICE_TYPES)[number];
export type AttackClass = (typeof ATTACK_CLASSES)[number];
export type DamageType = (typeof DAMAGE_TYPES)[number];
export type ProjectileType = (typeof PROJECTILE_TYPES)[number];
export type ProjectilePath = (typeof PROJECTILE_PATHS)[number];

export type AbilityTargetSize = JsonObject & { x: number; y: number };

export type AbilityTargetSelect = JsonObject & {
  targetType: TargetSelectType;
  allegianceSelectType: TargetAllegianceType;
  numTargetableUnits: number;
  zoneSize: AbilityTargetSize;
  range: number;
};

export type AbilityDepiction = JsonObject & {
  dmgAnim: string;
  /** Current persisted field. Older files may contain only projectileAnim. */
  projectileType?: ProjectileType;
  projectileAnim?: string;
  projectilePath: ProjectilePath;
  startSound: string;
  dmgSound: string;
};

export type AbilitySave = JsonObject & {
  saveStat: StatType;
  saveBase: number;
  saveAgainst: StatType;
  saveAgainstBase: number;
};

export type AbilityAttackDamage = JsonObject & {
  dmgDice: DieType[];
  dmgBonus: number;
  dmgStat: StatType;
  dmgStatMult: number;
  attackBonus: number;
};

export type AbilityAttack = JsonObject & {
  attackClass: AttackClass;
  damageType?: DamageType;
  dmg?: AbilityAttackDamage;
  save?: AbilitySave;
};

export type AbilityStatus = JsonObject & {
  statusEffect: string;
  save?: AbilitySave;
  baseDuration?: number;
  durationBonus?: number;
};

export type AbilityRestore = JsonObject & {
  restoreWhich: CurrentStatType;
  restoreDice: DieType[];
  restoreBonus: number;
  restoreStat: StatType;
  restoreStatMult: number;
};

export type AbilityDamage = JsonObject & {
  damageType: DamageType;
  dmgDice: DieType[];
  dmgBonus: number;
  dmgStat: StatType;
  dmgStatMult: number;
};

/** Known fields are typed while JsonObject retains forward-compatible fields. */
export type AbilityRecord = JsonObject & {
  name: string;
  label: string;
  description: string;
  icon: string;
  type: AbilityType;
  targetSelect: AbilityTargetSelect;
  apCost: number;
  costType: AbilityCostType;
  costValue: number;
  depiction: AbilityDepiction;
  attacks?: AbilityAttack[];
  statuses?: AbilityStatus[];
  restores?: AbilityRestore[];
  damages?: AbilityDamage[];
};
