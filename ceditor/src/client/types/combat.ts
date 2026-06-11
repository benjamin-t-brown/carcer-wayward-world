// Combat ability / status effect schema (mirrors model/templates/CombatTypes.h)

export type StatusEffectCondition =
  | 'CONDITION_ALWAYS'
  | 'CONDITION_TARGET_HP_BELOW_HALF'
  | 'CONDITION_SELF_HP_BELOW_HALF'
  | 'CONDITION_IS_EVEN_ROUND'
  | 'CONDITION_IS_ODD_ROUND'
  | 'CONDITION_AT_LEAST_ENEMY_IN_RANGE_OF_TARGET'
  | 'CONDITION_HAS_NOT_MOVED_SINCE_LAST_ROUND'
  | 'CONDITION_HAS_NEARBY_PEASANT'
  | 'CONDITION_ATTACK_MISSED'
  | 'CONDITION_FIRST_TIME_ATTACKED';

export type AbilityType =
  | 'ABILITY_ATTACK'
  | 'ABILITY_SPELL'
  | 'ABILITY_SKILL'
  | 'ABILITY_SUB_ATTACK';

export type TargetSelectType =
  | 'TARGET_SELF'
  | 'TARGET_UNIT'
  | 'TARGET_ZONE'
  | 'TARGET_ALL_IN_RANGE';

export type TargetAllegianceSelectType =
  | 'TARGET_ALLEGIANCE_SAME'
  | 'TARGET_ALLEGIANCE_SAME_AND_SELF'
  | 'TARGET_ALLEGIANCE_OTHER'
  | 'TARGET_ALLEGIANCE_ALL'
  | 'TARGET_ALLEGIANCE_ALL_AND_SELF';

export type Dice =
  | 'D0'
  | 'D2'
  | 'D4'
  | 'D6'
  | 'D8'
  | 'D10'
  | 'D12'
  | 'D20'
  | 'D100';

export type StatsEnum =
  | 'STAT_STR'
  | 'STAT_MND'
  | 'STAT_CON'
  | 'STAT_AGI'
  | 'STAT_LCK';

export type StatusEventType =
  | 'STATUS_EVENT_ON_APPLIED'
  | 'STATUS_EVENT_ON_ATTACK'
  | 'STATUS_EVENT_ON_ATTACKED_MELEE'
  | 'STATUS_EVENT_ON_ATTACKED_RANGE'
  | 'STATUS_EVENT_ON_ATTACKED_MAGIC'
  | 'STATUS_EVENT_ON_MOVE'
  | 'STATUS_EVENT_ON_TURN_START'
  | 'STATUS_EVENT_ON_TURN_END'
  | 'STATUS_EVENT_ON_ROUND_START';

export type CurrentStatEnum =
  | 'CURRENT_STAT_HP'
  | 'CURRENT_STAT_AP'
  | 'CURRENT_STAT_MANA'
  | 'CURRENT_STAT_AC';

export type StatusActionTargetType =
  | 'STATUS_ACTION_TARGET_SELF'
  | 'STATUS_ACTION_TARGET_ATTACKER_LOCATION'
  | 'STATUS_ACTION_TARGET_LAST_LOCATION';

export type AbilityCostType =
  | 'ABILITY_COST_NONE'
  | 'ABILITY_COST_MANA'
  | 'ABILITY_COST_COOLDOWN'
  | 'ABILITY_COST_HP';

export type AttackClass =
  | 'ATTACK_CLASS_MELEE'
  | 'ATTACK_CLASS_RANGED'
  | 'ATTACK_CLASS_MAGIC'
  | 'ATTACK_CLASS_AUTO_HIT';

export type ProjectilePath =
  | 'PROJECTILE_PATH_SHORT'
  | 'PROJECTILE_PATH_MEDIUM'
  | 'PROJECTILE_PATH_TALL'
  | 'PROJECTILE_PATH_NONE';

export type DamageType =
  | 'DAMAGE_TYPE_EDGED'
  | 'DAMAGE_TYPE_BASHING'
  | 'DAMAGE_TYPE_PIERCING'
  | 'DAMAGE_TYPE_HEAT'
  | 'DAMAGE_TYPE_FREEZE'
  | 'DAMAGE_TYPE_STATIC'
  | 'DAMAGE_TYPE_NECROTIC'
  | 'DAMAGE_TYPE_EPHEMERAL'
  | 'DAMAGE_TYPE_TRUE';

export interface TargetSelectInfoPoint {
  x: number;
  y: number;
}

export interface TargetSelectInfo {
  targetType: TargetSelectType;
  allegianceSelectType: TargetAllegianceSelectType;
  numTargetableUnits: number;
  zoneSize: TargetSelectInfoPoint;
  range: number;
}

export interface Stats {
  STR?: number;
  MND?: number;
  CON?: number;
  AGI?: number;
  LCK?: number;
}

export interface CurrentStats {
  HP?: number;
  AP?: number;
  MANA?: number;
  AC?: number;
}

export interface Resistance {
  attackType: DamageType;
  mod: number;
}

export interface StatusEffectEvent {
  type: StatusEventType;
  condition: StatusEffectCondition;
}

export interface AbilityDepiction {
  dmgAnim: string;
  projectileHasFacing?: boolean;
  projectileAnim: string;
  projectilePath: ProjectilePath;
  startSound: string;
  dmgSound: string;
}

export interface AbilitySave {
  saveStat: StatsEnum;
  saveBase: number;
  saveAgainst: StatsEnum;
  saveAgainstBase: number;
}

export interface AbilityAttackDmg {
  dmgDice: Dice[];
  dmgBonus: number;
  dmgStat: StatsEnum;
  dmgStatMult: number;
  attackBonus: number;
}

export function createDefaultAbilityAttackDmg(): AbilityAttackDmg {
  return {
    dmgDice: ['D6'],
    dmgBonus: 0,
    dmgStat: 'STAT_STR',
    dmgStatMult: 1,
    attackBonus: 0,
  };
}

/** Fills missing dmg fields; keeps dice from base when present. */
export function mergeAbilityAttackDmg(
  base?: AbilityAttackDmg,
): AbilityAttackDmg {
  const defaults = createDefaultAbilityAttackDmg();
  if (!base) {
    return defaults;
  }
  return {
    ...defaults,
    ...base,
    dmgDice: base.dmgDice?.length ? [...base.dmgDice] : [...defaults.dmgDice],
  };
}

export interface AbilityAttack {
  attackClass: AttackClass;
  dmg?: AbilityAttackDmg;
  save?: AbilitySave;
}

export interface AbilityStatus {
  statusEffect: string;
  save?: AbilitySave;
  /** Overrides template baseDuration for this application. */
  baseDuration?: number;
  /** Flat turns added after base. */
  durationBonus?: number;
}

export interface AbilityRestore {
  restoreWhich: CurrentStatEnum;
  restoreDice: Dice[];
  restoreBonus: number;
  restoreStat: StatsEnum;
  restoreStatMult: number;
}

export function createDefaultAbilityRestore(): AbilityRestore {
  return {
    restoreWhich: 'CURRENT_STAT_HP',
    restoreDice: ['D6'],
    restoreBonus: 0,
    restoreStat: 'STAT_MND',
    restoreStatMult: 1,
  };
}

/** Fills missing restore fields; keeps dice from base when present. */
export function mergeAbilityRestore(base?: AbilityRestore): AbilityRestore {
  const defaults = createDefaultAbilityRestore();
  if (!base) {
    return defaults;
  }
  return {
    ...defaults,
    ...base,
    restoreDice: base.restoreDice?.length
      ? [...base.restoreDice]
      : [...defaults.restoreDice],
  };
}

export interface StatusEffectDurationScale {
  durationStat: StatsEnum;
  durationStatMult: number;
}

export function createDefaultStatusEffectDurationScale(): StatusEffectDurationScale {
  return {
    durationStat: 'STAT_MND',
    durationStatMult: 1,
  };
}

export interface StatusEffectAction {
  statusActionTargetType: StatusActionTargetType;
  abilityName: string;
  events: StatusEffectEvent[];
}

export interface AbilityTemplate {
  name: string;
  label: string;
  description: string;
  icon: string;
  type: AbilityType;
  targetSelect: TargetSelectInfo;
  apCost: number;
  costType: AbilityCostType;
  costValue: number;
  depiction: AbilityDepiction;
  attacks?: AbilityAttack[];
  statuses?: AbilityStatus[];
  restores?: AbilityRestore[];
}

export interface StatusEffectTemplate {
  name: string;
  description: string;
  baseDuration: number;
  durationScale?: StatusEffectDurationScale;
  applyBonuses?: Stats;
  applyCurrentStatChange?: CurrentStats;
  applyResistances?: Resistance[];
  actions?: StatusEffectAction[];
}

export const STATUS_EFFECT_CONDITIONS: StatusEffectCondition[] = [
  'CONDITION_ALWAYS',
  'CONDITION_TARGET_HP_BELOW_HALF',
  'CONDITION_SELF_HP_BELOW_HALF',
  'CONDITION_IS_EVEN_ROUND',
  'CONDITION_IS_ODD_ROUND',
  'CONDITION_AT_LEAST_ENEMY_IN_RANGE_OF_TARGET',
  'CONDITION_HAS_NOT_MOVED_SINCE_LAST_ROUND',
  'CONDITION_HAS_NEARBY_PEASANT',
  'CONDITION_ATTACK_MISSED',
  'CONDITION_FIRST_TIME_ATTACKED',
];

export const ABILITY_TYPES: AbilityType[] = [
  'ABILITY_ATTACK',
  'ABILITY_SPELL',
  'ABILITY_SKILL',
  'ABILITY_SUB_ATTACK',
];

export const TARGET_SELECT_TYPES: TargetSelectType[] = [
  'TARGET_SELF',
  'TARGET_UNIT',
  'TARGET_ZONE',
  'TARGET_ALL_IN_RANGE',
];

export const TARGET_ALLEGIANCE_TYPES: TargetAllegianceSelectType[] = [
  'TARGET_ALLEGIANCE_SAME',
  'TARGET_ALLEGIANCE_SAME_AND_SELF',
  'TARGET_ALLEGIANCE_OTHER',
  'TARGET_ALLEGIANCE_ALL',
  'TARGET_ALLEGIANCE_ALL_AND_SELF',
];

export const DICE_TYPES: Dice[] = [
  'D0',
  'D2',
  'D4',
  'D6',
  'D8',
  'D10',
  'D12',
  'D20',
  'D100',
];

export const STATS_ENUMS: StatsEnum[] = [
  'STAT_STR',
  'STAT_MND',
  'STAT_CON',
  'STAT_AGI',
  'STAT_LCK',
];

export const STATUS_EVENT_TYPES: StatusEventType[] = [
  'STATUS_EVENT_ON_APPLIED',
  'STATUS_EVENT_ON_ATTACK',
  'STATUS_EVENT_ON_ATTACKED_MELEE',
  'STATUS_EVENT_ON_ATTACKED_RANGE',
  'STATUS_EVENT_ON_ATTACKED_MAGIC',
  'STATUS_EVENT_ON_MOVE',
  'STATUS_EVENT_ON_TURN_START',
  'STATUS_EVENT_ON_TURN_END',
  'STATUS_EVENT_ON_ROUND_START',
];

export const CURRENT_STAT_ENUMS: CurrentStatEnum[] = [
  'CURRENT_STAT_HP',
  'CURRENT_STAT_AP',
  'CURRENT_STAT_MANA',
  'CURRENT_STAT_AC',
];

export const STATUS_ACTION_TARGET_TYPES: StatusActionTargetType[] = [
  'STATUS_ACTION_TARGET_SELF',
  'STATUS_ACTION_TARGET_ATTACKER_LOCATION',
  'STATUS_ACTION_TARGET_LAST_LOCATION',
];

export const ABILITY_COST_TYPES: AbilityCostType[] = [
  'ABILITY_COST_NONE',
  'ABILITY_COST_MANA',
  'ABILITY_COST_COOLDOWN',
  'ABILITY_COST_HP',
];

export const ATTACK_CLASSES: AttackClass[] = [
  'ATTACK_CLASS_MELEE',
  'ATTACK_CLASS_RANGED',
  'ATTACK_CLASS_MAGIC',
  'ATTACK_CLASS_AUTO_HIT',
];

export function attackClassUsesAttackBonus(attackClass: AttackClass): boolean {
  return (
    attackClass !== 'ATTACK_CLASS_MAGIC' &&
    attackClass !== 'ATTACK_CLASS_AUTO_HIT'
  );
}

export const PROJECTILE_PATHS: ProjectilePath[] = [
  'PROJECTILE_PATH_SHORT',
  'PROJECTILE_PATH_MEDIUM',
  'PROJECTILE_PATH_TALL',
  'PROJECTILE_PATH_NONE',
];

export const DAMAGE_TYPES: DamageType[] = [
  'DAMAGE_TYPE_EDGED',
  'DAMAGE_TYPE_BASHING',
  'DAMAGE_TYPE_PIERCING',
  'DAMAGE_TYPE_HEAT',
  'DAMAGE_TYPE_FREEZE',
  'DAMAGE_TYPE_STATIC',
  'DAMAGE_TYPE_NECROTIC',
  'DAMAGE_TYPE_EPHEMERAL',
  'DAMAGE_TYPE_TRUE',
];

export function createDefaultTargetSelectInfo(): TargetSelectInfo {
  return {
    targetType: 'TARGET_UNIT',
    allegianceSelectType: 'TARGET_ALLEGIANCE_OTHER',
    numTargetableUnits: 1,
    zoneSize: { x: 1, y: 1 },
    range: 1,
  };
}

export function createDefaultAbilityDepiction(): AbilityDepiction {
  return {
    dmgAnim: '',
    projectileHasFacing: false,
    projectileAnim: '',
    projectilePath: 'PROJECTILE_PATH_NONE',
    startSound: '',
    dmgSound: '',
  };
}

export function abilityDepictionHasProjectile(
  depiction: AbilityDepiction,
): boolean {
  return depiction.projectilePath !== 'PROJECTILE_PATH_NONE';
}

export const ABILITY_PROJECTILE_PATHS: ProjectilePath[] = [
  'PROJECTILE_PATH_SHORT',
  'PROJECTILE_PATH_MEDIUM',
  'PROJECTILE_PATH_TALL',
];

export function setAbilityDepictionHasProjectile(
  depiction: AbilityDepiction,
  hasProjectile: boolean,
): AbilityDepiction {
  if (!hasProjectile) {
    return {
      ...depiction,
      projectileHasFacing: false,
      projectileAnim: '',
      projectilePath: 'PROJECTILE_PATH_NONE',
    };
  }
  if (depiction.projectilePath === 'PROJECTILE_PATH_NONE') {
    return { ...depiction, projectilePath: 'PROJECTILE_PATH_SHORT' };
  }
  return depiction;
}

/** Clear depiction anim/sound fields that are not defined in loaded SDL2W assets. */
export function sanitizeAbilityDepiction(
  depiction: AbilityDepiction,
  animationMap: Record<string, unknown>,
  soundMap: Record<string, unknown>,
): AbilityDepiction {
  return {
    ...depiction,
    dmgAnim:
      depiction.dmgAnim && depiction.dmgAnim in animationMap
        ? depiction.dmgAnim
        : '',
    projectileAnim:
      depiction.projectileAnim && depiction.projectileAnim in animationMap
        ? depiction.projectileAnim
        : '',
    startSound:
      depiction.startSound && depiction.startSound in soundMap
        ? depiction.startSound
        : '',
    dmgSound:
      depiction.dmgSound && depiction.dmgSound in soundMap
        ? depiction.dmgSound
        : '',
  };
}

export function sanitizeTargetSelect(
  targetSelect: TargetSelectInfo,
): TargetSelectInfo {
  const targetType = targetSelect.targetType as string;
  if (
    targetType === 'TARGET_MOVE' ||
    !TARGET_SELECT_TYPES.includes(targetSelect.targetType)
  ) {
    return { ...targetSelect, targetType: 'TARGET_UNIT' };
  }
  return targetSelect;
}

export function sanitizeAbilityTemplates(
  abilities: AbilityTemplate[],
  animationMap: Record<string, unknown>,
  soundMap: Record<string, unknown>,
): AbilityTemplate[] {
  return abilities.map((ability) => ({
    ...ability,
    targetSelect: sanitizeTargetSelect(ability.targetSelect),
    depiction: sanitizeAbilityDepiction(
      ability.depiction,
      animationMap,
      soundMap,
    ),
  }));
}

export function createDefaultAbilityTemplate(): AbilityTemplate {
  return {
    name: '',
    label: '',
    description: '',
    icon: '',
    type: 'ABILITY_ATTACK',
    targetSelect: createDefaultTargetSelectInfo(),
    apCost: 1,
    costType: 'ABILITY_COST_NONE',
    costValue: 0,
    depiction: createDefaultAbilityDepiction(),
    attacks: [],
    statuses: [],
    restores: [],
  };
}

export function createDefaultStatusEffectTemplate(): StatusEffectTemplate {
  return {
    name: '',
    description: '',
    baseDuration: 1,
    applyResistances: [],
    actions: [],
  };
}
