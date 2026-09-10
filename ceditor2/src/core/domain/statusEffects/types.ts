import type { JsonObject } from '../../database/types.js';

export const STAT_TYPES = [
  'STAT_STR',
  'STAT_MND',
  'STAT_CON',
  'STAT_AGI',
  'STAT_LCK',
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

export const STATUS_EVENT_TYPES = [
  'STATUS_EVENT_ON_APPLIED',
  'STATUS_EVENT_ON_ATTACK',
  'STATUS_EVENT_ON_ATTACKED_MELEE',
  'STATUS_EVENT_ON_ATTACKED_RANGE',
  'STATUS_EVENT_ON_ATTACKED_MAGIC',
  'STATUS_EVENT_ON_MOVE',
  'STATUS_EVENT_ON_TURN_START',
  'STATUS_EVENT_ON_TURN_END',
  'STATUS_EVENT_ON_ROUND_START',
] as const;

/** Values accepted by the current C++ statusEffectConditionFromString loader. */
export const STATUS_EFFECT_CONDITIONS = [
  'CONDITION_ALWAYS',
  'CONDITION_TARGET_HP_BELOW_HALF',
  'CONDITION_SELF_HP_BELOW_HALF',
  'CONDITION_AT_LEAST_ENEMY_IN_RANGE_OF_TARGET',
  'CONDITION_HAS_NOT_MOVED_SINCE_LAST_ROUND',
  'CONDITION_ATTACK_MISSED',
  'CONDITION_FIRST_TIME_ATTACKED',
] as const;

export const STATUS_ACTION_TARGET_TYPES = [
  'STATUS_ACTION_TARGET_SELF',
  'STATUS_ACTION_TARGET_ATTACKER_LOCATION',
  'STATUS_ACTION_TARGET_LAST_LOCATION',
] as const;

export type StatType = (typeof STAT_TYPES)[number];
export type DamageType = (typeof DAMAGE_TYPES)[number];
export type StatusEventType = (typeof STATUS_EVENT_TYPES)[number];
export type StatusEffectCondition = (typeof STATUS_EFFECT_CONDITIONS)[number];
export type StatusActionTargetType =
  (typeof STATUS_ACTION_TARGET_TYPES)[number];

export type StatusEffectStats = JsonObject & {
  STR?: number;
  MND?: number;
  CON?: number;
  AGI?: number;
  LCK?: number;
};

export type StatusEffectCurrentStats = JsonObject & {
  HP?: number;
  AP?: number;
  MANA?: number;
  AC?: number;
};

export type StatusEffectResistance = JsonObject & {
  attackType: DamageType;
  mod: number;
};

export type StatusEffectDurationScale = JsonObject & {
  durationStat: StatType;
  durationStatMult: number;
};

export type StatusEffectEvent = JsonObject & {
  type: StatusEventType;
  condition: StatusEffectCondition;
};

export type StatusEffectAction = JsonObject & {
  statusActionTargetType: StatusActionTargetType;
  abilityName: string;
  events: StatusEffectEvent[];
};

/**
 * The editable JSON record. Intersecting each shape with JsonObject keeps known
 * fields typed while retaining any newer fields ceditor2 does not understand.
 */
export type StatusEffectRecord = JsonObject & {
  name: string;
  description: string;
  baseDuration: number;
  durationScale?: StatusEffectDurationScale;
  applyBonuses?: StatusEffectStats;
  applyCurrentStatChange?: StatusEffectCurrentStats;
  applyResistances?: StatusEffectResistance[];
  actions?: StatusEffectAction[];
};
