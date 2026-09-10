import type { JsonObject } from '../../database/types.js';
import type {
  AbilityAttackDamage,
  AbilityRestore,
} from '../abilities/index.js';

export const ITEM_TYPES = [
  'WEAPON_MELEE',
  'WEAPON_MELEE_2H',
  'WEAPON_RANGED',
  'WEAPON_AMMO',
  'SHIELD',
  'GARB',
  'PANTS',
  'GLOVES',
  'HAT',
  'SHOES',
  'NECKLACE',
  'POTION',
  'UTILITY',
  'RUNE',
] as const;

export const ITEM_USABILITIES = [
  'NOT_USABLE',
  'USABLE_EVERYWHERE',
  'USABLE_TOWN_ONLY',
  'USABLE_COMBAT_ONLY',
  'USABLE_OUTSIDE_ONLY',
  'USABLE_TOWN_AND_COMBAT',
] as const;

export const RUNE_TYPES = [
  'HEAT',
  'ENTROPY',
  'REGROWTH',
  'DISPLACE',
  'EXPAND',
  'ATTACH',
  'TRANSFORM',
  'COMPACT',
] as const;

export type ItemType = (typeof ITEM_TYPES)[number];
export type KnownItemUsability = (typeof ITEM_USABILITIES)[number];
/** Unknown strings are loader-compatible and therefore remain editable. */
export type ItemUsability = KnownItemUsability | (string & {});
export type RuneType = (typeof RUNE_TYPES)[number];

export type ItemStatusEffectObject = JsonObject & { name: string };
export type ItemStatusEffectRef = string | ItemStatusEffectObject;

export type ItemWeaponConfig = JsonObject & {
  abilityName: string;
  dmgOverrides?: AbilityAttackDamage[];
  /** Older storage shape still accepted by the game loader. */
  attackIndex?: number;
  /** Older storage shape still accepted by the game loader. */
  dmg?: AbilityAttackDamage;
};

export type ItemUseAbilityConfig = JsonObject & {
  abilityName: string;
  dmgOverrides?: AbilityAttackDamage[];
  restoreOverrides?: AbilityRestore[];
};

/** Known fields are typed while JsonObject retains forward-compatible fields. */
export type ItemRecord = JsonObject & {
  itemType: ItemType;
  name: string;
  label: string;
  icon: string;
  description: string;
  weight: number;
  value: number;
  stackable?: boolean;
  indestructable?: boolean;
  itemUsability?: ItemUsability;
  useAbility?: ItemUseAbilityConfig;
  useSpecialEvent?: string;
  statusEffects?: ItemStatusEffectRef[];
  weapon?: ItemWeaponConfig;
  runeType?: RuneType;
};
