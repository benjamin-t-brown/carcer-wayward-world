// Mirrors src/model/templates/Spells.hpp SpellTemplate + RuneTypes.h

/** Matches model::RuneType in RuneTypes.h */
export type RuneType =
  | 'HEAT'
  | 'ENTROPY'
  | 'REGROWTH'
  | 'DISPLACE'
  | 'EXPAND'
  | 'ATTACH'
  | 'TRANSFORM'
  | 'COMPACT';

export const RUNE_TYPES: RuneType[] = [
  'HEAT',
  'ENTROPY',
  'REGROWTH',
  'DISPLACE',
  'EXPAND',
  'ATTACH',
  'TRANSFORM',
  'COMPACT',
];

const RUNE_TYPE_SET = new Set<string>(RUNE_TYPES);

export function isRuneType(value: unknown): value is RuneType {
  return typeof value === 'string' && RUNE_TYPE_SET.has(value);
}

/** Matches model::runeTypeIndex */
export function runeTypeIndex(type: RuneType): number {
  return RUNE_TYPES.indexOf(type);
}

/** Matches model::runeTypeToSpriteName → runes_0 … runes_7 */
export function runeTypeToSpriteName(type: RuneType): string {
  return `runes_${runeTypeIndex(type)}`;
}

/** Matches model::SpellRuneRequirement in Spells.hpp */
export interface SpellRuneRequirement {
  type: RuneType;
  count: number;
}

export interface SpellTemplate {
  name: string;
  label: string;
  description: string;
  icon: string;
  abilityName: string;
  requiredRunes: SpellRuneRequirement[];
}

export function createDefaultSpellTemplate(): SpellTemplate {
  return {
    name: '',
    label: '',
    description: '',
    icon: 'runes_0',
    abilityName: '',
    requiredRunes: [],
  };
}

function sanitizeRuneCount(count: unknown): number | null {
  const n = typeof count === 'number' ? count : Number(count);
  if (!Number.isFinite(n) || n <= 0) {
    return null;
  }
  return Math.floor(n);
}

function sanitizeRequiredRunes(requiredRunes: unknown): SpellRuneRequirement[] {
  if (!Array.isArray(requiredRunes)) {
    return [];
  }

  const seen = new Set<RuneType>();
  const result: SpellRuneRequirement[] = [];

  for (const entry of requiredRunes) {
    if (!entry || typeof entry !== 'object') {
      continue;
    }
    const raw = entry as { type?: unknown; count?: unknown };
    if (!isRuneType(raw.type)) {
      continue;
    }
    if (seen.has(raw.type)) {
      continue;
    }
    const count = sanitizeRuneCount(raw.count);
    if (count === null) {
      continue;
    }
    seen.add(raw.type);
    result.push({ type: raw.type, count });
  }

  return result;
}

/** Soft validation: spells with a non-empty abilityName must reference a known ability. */
export function validateSpellAbilityRefs(
  spells: SpellTemplate[],
  abilityNames: Iterable<string>,
): string[] {
  const known = new Set(
    [...abilityNames].map((name) => name.trim()).filter(Boolean),
  );
  const errors: string[] = [];

  for (const spell of spells) {
    const spellId = spell.name?.trim() || spell.label?.trim() || '(unnamed spell)';
    const abilityName = spell.abilityName?.trim() ?? '';
    if (!abilityName) {
      continue;
    }
    if (!known.has(abilityName)) {
      errors.push(
        `${spellId}: ability "${abilityName}" does not exist`,
      );
    }
  }

  return errors;
}

export function sanitizeSpellTemplates(
  spells: SpellTemplate[],
): SpellTemplate[] {
  return spells.map((spell) => {
    const defaults = createDefaultSpellTemplate();
    return {
      name: typeof spell?.name === 'string' ? spell.name : defaults.name,
      label: typeof spell?.label === 'string' ? spell.label : defaults.label,
      description:
        typeof spell?.description === 'string'
          ? spell.description
          : defaults.description,
      icon: typeof spell?.icon === 'string' ? spell.icon : defaults.icon,
      abilityName:
        typeof spell?.abilityName === 'string'
          ? spell.abilityName
          : defaults.abilityName,
      requiredRunes: sanitizeRequiredRunes(spell?.requiredRunes),
    };
  });
}
