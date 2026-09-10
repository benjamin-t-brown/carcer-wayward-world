import type { JsonObject } from '../../database/types.js';

/** Values accepted by model::runeTypeFromString. */
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

export type RuneType = (typeof RUNE_TYPES)[number];

export type SpellRuneRequirement = JsonObject & {
  type: RuneType;
  count: number;
};

/** Known fields are typed while JsonObject retains forward-compatible fields. */
export type SpellRecord = JsonObject & {
  name: string;
  label: string;
  description: string;
  icon: string;
  abilityName: string;
  requiredRunes: SpellRuneRequirement[];
};
