// Consolidated asset types for the map and asset editor

import type {
  AbilityAttack,
  AbilityAttackDmg,
  AbilityRestore,
  StatusEffectTemplate,
} from './ability';
import { mergeAbilityAttackDmg, mergeAbilityRestore } from './ability';

// ============================================================================
// Item Templates
// ============================================================================

/** Matches model::ItemType in Items.h */
export type ItemType =
  | 'WEAPON_MELEE'
  | 'WEAPON_MELEE_2H'
  | 'WEAPON_RANGED'
  | 'WEAPON_AMMO'
  | 'SHIELD'
  | 'GARB'
  | 'PANTS'
  | 'GLOVES'
  | 'HAT'
  | 'SHOES'
  | 'NECKLACE'
  | 'POTION'
  | 'UTILITY';

export const ITEM_TYPES: ItemType[] = [
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
];

/** Matches model::ItemUsability in Items.h */
export type ItemUsability =
  | 'NOT_USABLE'
  | 'USABLE_EVERYWHERE'
  | 'USABLE_TOWN_ONLY'
  | 'USABLE_COMBAT_ONLY'
  | 'USABLE_OUTSIDE_ONLY'
  | 'USABLE_TOWN_AND_COMBAT';

export const ITEM_USABILITIES: ItemUsability[] = [
  'NOT_USABLE',
  'USABLE_EVERYWHERE',
  'USABLE_TOWN_ONLY',
  'USABLE_COMBAT_ONLY',
  'USABLE_OUTSIDE_ONLY',
  'USABLE_TOWN_AND_COMBAT',
];

/** Game loader accepts a bare name or `{ name }` object. */
export type ItemStatusEffectRef = string | { name: string };

export function isWeaponItemType(itemType: string): boolean {
  return itemType.startsWith('WEAPON');
}

export interface ItemWeaponConfig {
  abilityName: string;
  /** Per-attack damage overrides; index matches base ability attacks[]. */
  dmgOverrides?: AbilityAttackDmg[];
  /** @deprecated migrated to dmgOverrides */
  attackIndex?: number;
  /** @deprecated migrated to dmgOverrides */
  dmg?: AbilityAttackDmg;
}

export interface ItemUseAbilityConfig {
  abilityName: string;
  /** Per-attack damage overrides; index matches base ability attacks[]. */
  dmgOverrides?: AbilityAttackDmg[];
  /** Per-restore overrides; index matches base ability restores[]. */
  restoreOverrides?: AbilityRestore[];
}

export function isItemUsable(itemUsability?: string): boolean {
  return Boolean(itemUsability && itemUsability !== 'NOT_USABLE');
}

type LegacyItemWeapon = ItemWeaponConfig & { dmgMin?: number; dmgMax?: number };

/** Build dmgOverrides from stored weapon JSON and base ability attacks. */
export function resolveItemWeaponDmgOverrides(
  weapon: ItemWeaponConfig | undefined,
  baseAttacks: AbilityAttack[],
): AbilityAttackDmg[] {
  const count = baseAttacks.length;
  if (count === 0) {
    return [];
  }

  const overrides: (AbilityAttackDmg | undefined)[] = new Array(count);

  if (weapon?.dmgOverrides?.length) {
    weapon.dmgOverrides.forEach((dmg, i) => {
      if (i < count) {
        overrides[i] = mergeAbilityAttackDmg(dmg);
      }
    });
  }

  if (weapon?.dmg) {
    const idx = Math.min(weapon.attackIndex ?? 0, count - 1);
    if (!overrides[idx]) {
      overrides[idx] = mergeAbilityAttackDmg(weapon.dmg);
    }
  }

  for (let i = 0; i < count; i++) {
    if (!overrides[i]) {
      overrides[i] = mergeAbilityAttackDmg(baseAttacks[i]?.dmg);
    }
  }

  return overrides as AbilityAttackDmg[];
}

/** Build restoreOverrides from stored use-ability JSON and base ability restores. */
export function resolveItemUseAbilityRestoreOverrides(
  useAbility: ItemUseAbilityConfig | undefined,
  baseRestores: AbilityRestore[],
): AbilityRestore[] {
  const count = baseRestores.length;
  if (count === 0) {
    return [];
  }

  const overrides: (AbilityRestore | undefined)[] = new Array(count);

  if (useAbility?.restoreOverrides?.length) {
    useAbility.restoreOverrides.forEach((restore, i) => {
      if (i < count) {
        overrides[i] = mergeAbilityRestore(restore);
      }
    });
  }

  for (let i = 0; i < count; i++) {
    if (!overrides[i]) {
      overrides[i] = mergeAbilityRestore(baseRestores[i]);
    }
  }

  return overrides as AbilityRestore[];
}

/** Stored use-ability fields compared when detecting item edits. */
export function useAbilityStorageSnapshot(
  useAbility: ItemUseAbilityConfig | undefined,
): string {
  if (!useAbility) {
    return '';
  }
  return JSON.stringify({
    abilityName: useAbility.abilityName,
    dmgOverrides: useAbility.dmgOverrides,
    restoreOverrides: useAbility.restoreOverrides,
  });
}

export function normalizeItemUseAbilityConfig(
  useAbility: ItemUseAbilityConfig,
  baseAbility: {
    attacks?: AbilityAttack[];
    restores?: AbilityRestore[];
  } = {},
): ItemUseAbilityConfig {
  if (!useAbility.abilityName) {
    return { abilityName: '' };
  }

  const baseAttacks = baseAbility.attacks ?? [];
  const baseRestores = baseAbility.restores ?? [];
  const dmgOverrides = resolveItemWeaponDmgOverrides(useAbility, baseAttacks);
  const restoreOverrides = resolveItemUseAbilityRestoreOverrides(
    useAbility,
    baseRestores,
  );

  return {
    abilityName: useAbility.abilityName,
    dmgOverrides: baseAttacks.length > 0 ? dmgOverrides : useAbility.dmgOverrides,
    restoreOverrides:
      baseRestores.length > 0 ? restoreOverrides : useAbility.restoreOverrides,
  };
}

/** Stored weapon fields compared when detecting item edits. */
export function weaponStorageSnapshot(weapon: ItemWeaponConfig | undefined): string {
  if (!weapon) {
    return '';
  }
  return JSON.stringify({
    abilityName: weapon.abilityName,
    dmgOverrides: weapon.dmgOverrides,
    attackIndex: weapon.attackIndex,
    dmg: weapon.dmg,
  });
}

/** Shift/remove override slots when an ability attack index is deleted. */
export function remapWeaponOverridesAfterAttackDelete(
  weapon: ItemWeaponConfig,
  deletedAttackIndex: number,
): ItemWeaponConfig {
  const overrides: AbilityAttackDmg[] = [];

  if (weapon.dmgOverrides?.length) {
    weapon.dmgOverrides.forEach((dmg, i) => {
      if (i !== deletedAttackIndex) {
        overrides.push(mergeAbilityAttackDmg(dmg));
      }
    });
  } else if (weapon.dmg) {
    const legacyIndex = weapon.attackIndex ?? 0;
    if (legacyIndex !== deletedAttackIndex) {
      overrides.push(mergeAbilityAttackDmg(weapon.dmg));
    }
  }

  return {
    abilityName: weapon.abilityName,
    dmgOverrides: overrides.length > 0 ? overrides : undefined,
  };
}

export function reconcileWeaponAfterAttackDelete(
  weapon: ItemWeaponConfig,
  deletedAttackIndex: number,
  newBaseAttacks: AbilityAttack[],
): ItemWeaponConfig {
  const remapped = remapWeaponOverridesAfterAttackDelete(weapon, deletedAttackIndex);
  return normalizeItemWeaponConfig(remapped, newBaseAttacks);
}

export function reconcileUseAbilityAfterAttackDelete(
  useAbility: ItemUseAbilityConfig,
  deletedAttackIndex: number,
  newBaseAttacks: AbilityAttack[],
  baseRestores: AbilityRestore[] = [],
): ItemUseAbilityConfig {
  const remapped = remapWeaponOverridesAfterAttackDelete(
    useAbility,
    deletedAttackIndex,
  );
  return normalizeItemUseAbilityConfig(remapped, {
    attacks: newBaseAttacks,
    restores: baseRestores,
  });
}

/** A single asset that will change when an ability (or its attack) is deleted. */
export interface AbilityDeleteImpact {
  kind: 'item' | 'statusEffect' | 'map';
  name: string;
  label: string;
  lines: string[];
}

/** @deprecated use AbilityDeleteImpact */
export type WeaponAttackDeleteItemImpact = AbilityDeleteImpact & { kind: 'item' };

export function describeWeaponAttackDeleteImpact(
  weapon: ItemWeaponConfig,
  deletedAttackIndex: number,
  attacksBeforeCount: number,
): string[] {
  const lines: string[] = [];
  const deletedAttackNumber = deletedAttackIndex + 1;

  const hadOverrideAtDeleted =
    Boolean(weapon.dmgOverrides?.[deletedAttackIndex]) ||
    (weapon.dmg !== undefined && (weapon.attackIndex ?? 0) === deletedAttackIndex);

  if (hadOverrideAtDeleted) {
    lines.push(`Remove stored damage override for attack ${deletedAttackNumber}.`);
  }

  const overrideCount = weapon.dmgOverrides?.length ?? 0;
  for (let i = deletedAttackIndex + 1; i < overrideCount; i++) {
    lines.push(`Move attack ${i + 1} damage override → attack ${i}.`);
  }

  const newAttackCount = attacksBeforeCount - 1;
  if (overrideCount > newAttackCount && overrideCount > deletedAttackIndex + 1) {
    const extra = overrideCount - newAttackCount;
    if (extra > 0 && !lines.some((l) => l.includes('Move attack'))) {
      lines.push(`Drop ${extra} extra override slot(s) no longer used.`);
    }
  }

  if (lines.length === 0) {
    lines.push(
      `Re-align overrides (${attacksBeforeCount} → ${newAttackCount} attacks).`,
    );
  }

  return lines;
}

export function planWeaponAttackDeleteImpacts(
  abilityName: string,
  deletedAttackIndex: number,
  currentAttacks: AbilityAttack[],
  items: ItemTemplate[],
  baseRestores: AbilityRestore[] = [],
): AbilityDeleteImpact[] {
  const newAttacks = currentAttacks.filter((_, i) => i !== deletedAttackIndex);
  const impacts: AbilityDeleteImpact[] = [];

  for (const item of items) {
    const weapon = item.weapon;
    if (weapon?.abilityName === abilityName) {
      const afterWeapon = reconcileWeaponAfterAttackDelete(
        weapon,
        deletedAttackIndex,
        newAttacks,
      );
      if (weaponStorageSnapshot(weapon) !== weaponStorageSnapshot(afterWeapon)) {
        impacts.push({
          kind: 'item',
          name: item.name,
          label: item.label || item.name,
          lines: [
            'Weapon:',
            ...describeWeaponAttackDeleteImpact(
              weapon,
              deletedAttackIndex,
              currentAttacks.length,
            ),
          ],
        });
      }
    }

    const useAbility = item.useAbility;
    if (useAbility?.abilityName === abilityName && useAbility.dmgOverrides?.length) {
      const afterUseAbility = reconcileUseAbilityAfterAttackDelete(
        useAbility,
        deletedAttackIndex,
        newAttacks,
        baseRestores,
      );
      if (
        useAbilityStorageSnapshot(useAbility) !==
        useAbilityStorageSnapshot(afterUseAbility)
      ) {
        impacts.push({
          kind: 'item',
          name: item.name,
          label: item.label || item.name,
          lines: [
            'Use ability:',
            ...describeWeaponAttackDeleteImpact(
              useAbility,
              deletedAttackIndex,
              currentAttacks.length,
            ),
          ],
        });
      }
    }
  }

  return impacts;
}

export function applyWeaponAttackDeleteToItems(
  abilityName: string,
  deletedAttackIndex: number,
  newAttacks: AbilityAttack[],
  items: ItemTemplate[],
  baseRestores: AbilityRestore[] = [],
): ItemTemplate[] {
  return items.map((item) => {
    let next = item;
    const weapon = item.weapon;
    if (weapon?.abilityName === abilityName) {
      next = {
        ...next,
        weapon: reconcileWeaponAfterAttackDelete(
          weapon,
          deletedAttackIndex,
          newAttacks,
        ),
      };
    }

    const useAbility = item.useAbility;
    if (useAbility?.abilityName === abilityName && useAbility.dmgOverrides?.length) {
      next = {
        ...next,
        useAbility: reconcileUseAbilityAfterAttackDelete(
          useAbility,
          deletedAttackIndex,
          newAttacks,
          baseRestores,
        ),
      };
    }

    return next;
  });
}

export function clearItemWeaponForDeletedAbility(
  _weapon: ItemWeaponConfig,
): ItemWeaponConfig {
  return { abilityName: '' };
}

export function clearItemUseAbilityForDeletedAbility(
  _useAbility: ItemUseAbilityConfig,
): ItemUseAbilityConfig {
  return { abilityName: '' };
}

export function describeItemAbilityDeleteImpact(weapon: ItemWeaponConfig): string[] {
  const lines: string[] = ['Clear weapon base ability reference.'];
  if (weapon.dmgOverrides?.length || weapon.dmg) {
    lines.push('Remove all stored damage overrides.');
  }
  return lines;
}

export function describeItemUseAbilityDeleteImpact(
  useAbility: ItemUseAbilityConfig,
): string[] {
  const lines: string[] = ['Clear use-ability reference.'];
  if (useAbility.dmgOverrides?.length) {
    lines.push('Remove all stored damage overrides.');
  }
  if (useAbility.restoreOverrides?.length) {
    lines.push('Remove all stored restore overrides.');
  }
  return lines;
}

function statusEffectStorageSnapshot(
  statusEffect: StatusEffectTemplate,
): string {
  return JSON.stringify({
    actions: statusEffect.actions,
  });
}

export function applyAbilityDeleteToStatusEffects(
  abilityName: string,
  statusEffects: StatusEffectTemplate[],
): StatusEffectTemplate[] {
  return statusEffects.map((statusEffect) => {
    if (!statusEffect.actions?.some((a) => a.abilityName === abilityName)) {
      return statusEffect;
    }
    return {
      ...statusEffect,
      actions: statusEffect.actions.map((action) =>
        action.abilityName === abilityName
          ? { ...action, abilityName: '' }
          : action,
      ),
    };
  });
}

export function planAbilityDeleteImpacts(
  abilityName: string,
  items: ItemTemplate[],
  statusEffects: StatusEffectTemplate[],
): AbilityDeleteImpact[] {
  const impacts: AbilityDeleteImpact[] = [];

  for (const item of items) {
    const weapon = item.weapon;
    if (weapon?.abilityName === abilityName) {
      const afterWeapon = clearItemWeaponForDeletedAbility(weapon);
      if (weaponStorageSnapshot(weapon) !== weaponStorageSnapshot(afterWeapon)) {
        impacts.push({
          kind: 'item',
          name: item.name,
          label: item.label || item.name,
          lines: describeItemAbilityDeleteImpact(weapon),
        });
      }
    }

    const useAbility = item.useAbility;
    if (useAbility?.abilityName === abilityName) {
      const afterUseAbility = clearItemUseAbilityForDeletedAbility(useAbility);
      if (
        useAbilityStorageSnapshot(useAbility) !==
        useAbilityStorageSnapshot(afterUseAbility)
      ) {
        impacts.push({
          kind: 'item',
          name: item.name,
          label: item.label || item.name,
          lines: describeItemUseAbilityDeleteImpact(useAbility),
        });
      }
    }
  }

  for (const statusEffect of statusEffects) {
    const matchingActions = statusEffect.actions?.filter(
      (a) => a.abilityName === abilityName,
    );
    if (!matchingActions?.length) {
      continue;
    }
    const after = applyAbilityDeleteToStatusEffects(abilityName, [statusEffect])[0];
    if (
      statusEffectStorageSnapshot(statusEffect) ===
      statusEffectStorageSnapshot(after)
    ) {
      continue;
    }
    impacts.push({
      kind: 'statusEffect',
      name: statusEffect.name,
      label: statusEffect.name,
      lines: [
        `Clear ability reference on ${matchingActions.length} invoked action(s).`,
      ],
    });
  }

  return impacts;
}

export function applyAbilityDeleteToItems(
  abilityName: string,
  items: ItemTemplate[],
): ItemTemplate[] {
  return items.map((item) => {
    let next = item;
    const weapon = item.weapon;
    if (weapon?.abilityName === abilityName) {
      next = {
        ...next,
        weapon: clearItemWeaponForDeletedAbility(weapon),
      };
    }

    const useAbility = item.useAbility;
    if (useAbility?.abilityName === abilityName) {
      next = {
        ...next,
        useAbility: clearItemUseAbilityForDeletedAbility(useAbility),
      };
    }

    return next;
  });
}

/** Map tiles that reference a character template by name. */
export function planCharacterDeleteImpacts(
  characterName: string,
  maps: CarcerMapTemplate[],
): AbilityDeleteImpact[] {
  const impacts: AbilityDeleteImpact[] = [];

  for (const map of maps) {
    const levelLines: string[] = [];
    let totalPlacements = 0;

    const byLevel = new Map<number, string[]>();
    for (const placement of map.characters ?? []) {
      if (placement.name !== characterName) {
        continue;
      }
      totalPlacements += 1;
      const x = placement.i % map.width;
      const y = Math.floor(placement.i / map.width);
      const summaries = byLevel.get(placement.l) ?? [];
      summaries.push(`(${x}, ${y})`);
      byLevel.set(placement.l, summaries);
    }

    for (const [levelKey, tileSummaries] of byLevel.entries()) {
      const levelPlacements = tileSummaries.length;
      const shown = tileSummaries.slice(0, 6).join(', ');
      const extra =
        tileSummaries.length > 6
          ? `, +${tileSummaries.length - 6} more tile(s)`
          : '';
      levelLines.push(
        `Level ${levelKey}: ${levelPlacements} placement(s) at ${shown}${extra}`,
      );
    }

    if (totalPlacements > 0) {
      impacts.push({
        kind: 'map',
        name: map.name,
        label: map.label || map.name,
        lines: [
          `Remove this character from ${totalPlacements} tile placement(s).`,
          ...levelLines,
        ],
      });
    }
  }

  return impacts;
}

export function applyCharacterDeleteToMaps(
  characterName: string,
  maps: CarcerMapTemplate[],
): CarcerMapTemplate[] {
  return maps.map((map) => ({
    ...map,
    characters: (map.characters ?? []).filter((c) => c.name !== characterName),
  }));
}

export function normalizeItemWeaponConfig(
  weapon: ItemWeaponConfig,
  baseAttacks: AbilityAttack[] = [],
): ItemWeaponConfig {
  const { attackIndex: _attackIndex, dmg: _dmg, dmgMin: _min, dmgMax: _max, ...rest } =
    weapon as LegacyItemWeapon;

  if (!rest.abilityName) {
    return { abilityName: '' };
  }

  const dmgOverrides = resolveItemWeaponDmgOverrides(weapon, baseAttacks);
  return {
    abilityName: rest.abilityName,
    dmgOverrides: baseAttacks.length > 0 ? dmgOverrides : rest.dmgOverrides,
  };
}

export interface ItemTemplate {
  itemType: ItemType;
  name: string;
  label: string;
  icon: string;
  description: string;
  weight: number;
  value: number;
  stackable?: boolean;
  /** Quest items and other items that cannot be dropped or destroyed */
  indestructable?: boolean;
  itemUsability?: ItemUsability;
  /** @deprecated replaced by useAbility */
  itemUsabilityArgs?: {
    itemUsabilityType: string;
    intArgs?: number[];
    stringArgs?: string[];
  };
  useAbility?: ItemUseAbilityConfig;
  /** Special event id to run when the item is used (alternative to useAbility) */
  useSpecialEvent?: string;
  statusEffects?: ItemStatusEffectRef[];
  weapon?: ItemWeaponConfig;
}

export function sanitizeItemTemplates(
  items: ItemTemplate[],
  abilities: {
    name: string;
    attacks?: AbilityAttack[];
    restores?: AbilityRestore[];
  }[] = [],
): ItemTemplate[] {
  return items.map((rawItem) => {
    const { itemUsabilityArgs: _legacyArgs, ...item } = rawItem;
    let next: ItemTemplate = item;

    if (next.weapon && !isWeaponItemType(next.itemType)) {
      const { weapon: _weapon, ...withoutWeapon } = next;
      next = withoutWeapon;
    }

    if (!isItemUsable(next.itemUsability)) {
      if (next.useAbility) {
        const { useAbility: _useAbility, ...withoutUseAbility } = next;
        next = withoutUseAbility;
      }
      if (next.useSpecialEvent) {
        const { useSpecialEvent: _useSpecialEvent, ...withoutUseSpecialEvent } =
          next;
        next = withoutUseSpecialEvent;
      }
    } else if (next.useAbility?.abilityName) {
      const baseAbility =
        abilities.find((a) => a.name === next.useAbility?.abilityName) ?? {};
      next = {
        ...next,
        useAbility: normalizeItemUseAbilityConfig(next.useAbility, baseAbility),
      };
    }

    if (!next.weapon) {
      return next;
    }

    const weapon = next.weapon as LegacyItemWeapon;
    const baseAttacks =
      abilities.find((a) => a.name === weapon.abilityName)?.attacks ?? [];

    if ('dmgMin' in weapon || 'dmgMax' in weapon) {
      const { dmgMin: _min, dmgMax: _max, ...rest } = weapon;
      if (!rest.abilityName) {
        return { ...next, weapon: { abilityName: '' } };
      }
      return {
        ...next,
        weapon: normalizeItemWeaponConfig(rest, baseAttacks),
      };
    }

    if (weapon.dmg || weapon.attackIndex !== undefined) {
      return {
        ...next,
        weapon: normalizeItemWeaponConfig(weapon, baseAttacks),
      };
    }

    return next;
  });
}

// ============================================================================
// Character Templates
// ============================================================================

/** Matches model::CharacterTemplateType in CharacterTemplate.h */
export type CharacterTemplateType =
  | 'TOWNSPERSON'
  | 'TOWNSPERSON_STATIC'
  | 'ENEMY'
  | 'ENEMY_STATIC';

export const CHARACTER_TEMPLATE_TYPES: CharacterTemplateType[] = [
  'TOWNSPERSON',
  'TOWNSPERSON_STATIC',
  'ENEMY',
  'ENEMY_STATIC',
];

export interface GenericCombatStats {
  str?: number;
  mnd?: number;
  con?: number;
  agi?: number;
  lck?: number;
}

export interface WeaponMasteryStats {
  edged?: number;
  pole?: number;
  blunt?: number;
  range?: number;
  unarmed?: number;
}

export interface MagicMasteryStats {
  mana?: number;
  abilityPower?: number;
  attunement?: number;
  faith?: number;
  lore?: number;
}

export interface BodyMasteryStats {
  resistPhysical?: number;
  resistMagical?: number;
  healingEffectiveness?: number;
  dr?: number;
  armorTraining?: number;
}

export interface TrainableCombatStats {
  weapon?: WeaponMasteryStats;
  magic?: MagicMasteryStats;
  body?: BodyMasteryStats;
}

export interface CharacterSkills {
  trickery?: number;
  stealth?: number;
  social?: number;
  magicItemUse?: number;
  cooking?: number;
  acrobatics?: number;
  survival?: number;
  focus?: number;
  conditioning?: number;
}

export interface CharacterStats {
  generic?: GenericCombatStats;
  trainable?: TrainableCombatStats;
  skills?: CharacterSkills;
}

export function createDefaultCharacterStats(): CharacterStats {
  return {
    generic: { str: 0, mnd: 0, con: 0, agi: 0, lck: 0 },
    trainable: {
      weapon: { edged: 0, pole: 0, blunt: 0, range: 0, unarmed: 0 },
      magic: { mana: 0, abilityPower: 0, attunement: 0, faith: 0, lore: 0 },
      body: {
        resistPhysical: 0,
        resistMagical: 0,
        healingEffectiveness: 0,
        dr: 0,
        armorTraining: 0,
      },
    },
    skills: {
      trickery: 0,
      stealth: 0,
      social: 0,
      magicItemUse: 0,
      cooking: 0,
      acrobatics: 0,
      survival: 0,
      focus: 0,
      conditioning: 0,
    },
  };
}

export interface CharacterTemplateSound {
  deathSoundName?: string;
  weaponSoundName?: string;
  /** @deprecated use deathSoundName — still accepted by game loader */
  deathSound?: string;
  /** @deprecated use weaponSoundName — still accepted by game loader */
  weaponSound?: string;
}

export interface CharacterTemplate {
  type: CharacterTemplateType;
  name: string;
  label: string;
  spritesheet: string;
  /** Integer index or string offset; game loader accepts both. */
  spriteOffset: number | string;
  stats?: CharacterStats;
  talk?: {
    talkName?: string;
    portraitName?: string;
  };
  behavior?: {
    behaviorName?: string;
  };
  combat?: {
    hp?: number;
    mp?: number;
    dropTable?: string;
    /** @deprecated legacy path merged into stats.generic by game loader */
    stats?: GenericCombatStats;
  };
  sound?: CharacterTemplateSound;
  statuses?: Array<{
    status: string;
  }>;
  vision?: {
    radius?: number;
  };
}

// ============================================================================
// Tileset Templates
// ============================================================================

export enum TileStepSound {
  TILE_STEP_SOUND_FLOOR,
  TILE_STEP_SOUND_GRASS,
  TILE_STEP_SOUND_DIRT,
  TILE_STEP_SOUND_GRAVEL,
}

/** Stored as enum index, numeric string, or integer in tilesets.json */
export type TileStepSoundValue = TileStepSound | number | string;

export interface TileMetadata {
  id: number;
  description?: string;
  stepSound?: TileStepSoundValue;
  isWalkable?: boolean;
  isSeeThrough?: boolean;
  isDoor?: boolean;
  isContainer?: boolean;
  tileTerrainBorderMeta?: TileTerrainBorderMeta;
}

export enum TileTerrainBorderTag {
  NONE = 'NONE',
  GRASS = 'GRASS',
  DIRT = 'DIRT',
  WATER = 'WATER',
  CAVE_FLOOR = 'CAVE_FLOOR',
  CAVE_WALL = 'CAVE_WALL',
  CAVE_WATER = 'CAVE_WATER',
  CLIFF = 'CLIFF',
  SNOW = 'SNOW',
}

/** Terrains that can be painted as primary in the map editor. */
export const PAINTABLE_TERRAIN_BORDER_TAGS: TileTerrainBorderTag[] = [
  TileTerrainBorderTag.GRASS,
  TileTerrainBorderTag.DIRT,
  TileTerrainBorderTag.WATER,
  TileTerrainBorderTag.CAVE_FLOOR,
  TileTerrainBorderTag.CAVE_WALL,
  TileTerrainBorderTag.CAVE_WATER,
  TileTerrainBorderTag.CLIFF,
  TileTerrainBorderTag.SNOW,
];

export const TERRAIN_BORDER_TAG_LABELS: Record<TileTerrainBorderTag, string> = {
  [TileTerrainBorderTag.NONE]: 'None',
  [TileTerrainBorderTag.GRASS]: 'Grass',
  [TileTerrainBorderTag.DIRT]: 'Dirt',
  [TileTerrainBorderTag.WATER]: 'Water',
  [TileTerrainBorderTag.CAVE_FLOOR]: 'Cave Floor',
  [TileTerrainBorderTag.CAVE_WALL]: 'Cave Wall',
  [TileTerrainBorderTag.CAVE_WATER]: 'Cave Water',
  [TileTerrainBorderTag.CLIFF]: 'Cliff',
  [TileTerrainBorderTag.SNOW]: 'Snow',
};

export const PAINTABLE_TERRAIN_BORDER_OPTIONS = PAINTABLE_TERRAIN_BORDER_TAGS.map(
  (value) => ({ value, label: TERRAIN_BORDER_TAG_LABELS[value] })
);

export const TERRAIN_BORDER_META_OPTIONS: {
  value: TileTerrainBorderTag;
  label: string;
}[] = [
  {
    value: TileTerrainBorderTag.NONE,
    label: TERRAIN_BORDER_TAG_LABELS[TileTerrainBorderTag.NONE],
  },
  ...PAINTABLE_TERRAIN_BORDER_OPTIONS,
];

export interface TileTerrainBorderMeta {
  ne: TileTerrainBorderTag;
  nw: TileTerrainBorderTag;
  se: TileTerrainBorderTag;
  sw: TileTerrainBorderTag;
}

export interface TilesetTemplate {
  name: string;
  spriteBase: string;
  imageWidth: number;
  imageHeight: number;
  tileWidth: number;
  tileHeight: number;
  terrain?: {
    primaryTerrain: TileTerrainBorderTag;
    secondaryTerrain: TileTerrainBorderTag;
    mode: number;
    startTileId: number;
  };
  tiles: TileMetadata[];
}

// ============================================================================
// Game Events
// ============================================================================

export interface GameEvent {
  id: string;
  title: string;
  eventType: 'MODAL' | 'TALK' | 'TRAVEL';
  icon: string;
  vars: Variable[];
  children: SENode[];
}

export interface Variable {
  id: string;
  key: string;
  value: string;
  importFrom: string;
}

// ephemeral event used in the SE editor
export interface SENode {
  id: string;
  eventChildType: GameEventChildType;
  x: number;
  y: number;
  h: number;
}

export enum GameEventType {
  MODAL = 'MODAL',
  TALK = 'TALK',
}

export enum GameEventChildType {
  KEYWORD = 'KEYWORD',
  CHOICE = 'CHOICE',
  END = 'END',
  EXEC = 'EXEC',
  SWITCH = 'SWITCH',
  COMMENT = 'COMMENT',
}

export enum KeywordType {
  K = 'K',
  K_DUP = 'K_DUP',
  K_SWITCH = 'K_SWITCH',
  K_CHILD = 'K_CHILD',
}

export interface VariableValue {
  str: string;
  evaluated?: string;
}

export interface AudioInfo {
  audioName: string;
  volume: number;
  offset: number;
}

// Keyword Data subtypes
export interface KeywordDataK {
  keywordType: KeywordType.K;
  text: string;
}

export interface KeywordDataKDup {
  keywordType: KeywordType.K_DUP;
  keyword: string;
}

export interface KeywordCheck {
  conditionStr: string;
  next: string;
}

export interface KeywordDataKSwitch {
  keywordType: KeywordType.K_SWITCH;
  defaultNext: string;
  checks: KeywordCheck[];
}

export interface KeywordDataKChild {
  keywordType: KeywordType.K_CHILD;
  next: string;
}

// Discriminated union for KeywordData
export type KeywordData =
  | KeywordDataK
  | KeywordDataKDup
  | KeywordDataKSwitch
  | KeywordDataKChild;

export interface ChoiceSwitchText {
  conditionStr: string;
  text: string;
}

export interface Choice {
  text: string;
  switchText?: ChoiceSwitchText[];
  prefixText?: string;
  conditionStr?: string;
  evalStr?: string;
  next: string;
}

// Game Event Child subtypes
export interface GameEventChildKeyword extends SENode {
  eventChildType: GameEventChildType.KEYWORD;
  id: string;
  keywords: Record<string, KeywordData>;
}

export interface GameEventChildChoice extends SENode {
  eventChildType: GameEventChildType.CHOICE;
  id: string;
  text: string;
  choices: Choice[];
  audioInfo?: AudioInfo;
}

export interface SwitchCase {
  conditionStr: string;
  next: string;
}

export interface GameEventChildSwitch extends SENode {
  eventChildType: GameEventChildType.SWITCH;
  id: string;
  defaultNext: string;
  cases: SwitchCase[];
}

export interface GameEventChildExec extends SENode {
  eventChildType: GameEventChildType.EXEC;
  id: string;
  p: string;
  execStr: string;
  next: string;
  autoAdvance: boolean;
  audioInfo?: AudioInfo;
}

export interface GameEventChildEnd extends SENode {
  eventChildType: GameEventChildType.END;
  id: string;
  next: string;
}

export interface GameEventChildComment extends SENode {
  eventChildType: GameEventChildType.COMMENT;
  id: string;
  comment: string;
}

// Discriminated union for GameEventChild
export type GameEventChild =
  | GameEventChildKeyword
  | GameEventChildChoice
  | GameEventChildSwitch
  | GameEventChildExec
  | GameEventChildEnd
  | GameEventChildComment;

export type GameEventChildUnion = GameEventChildKeyword &
  GameEventChildChoice &
  GameEventChildSwitch &
  GameEventChildExec &
  GameEventChildEnd;

// Expanded GameEvent type for full compatibility
export interface GameEventFull {
  id: string;
  title: string;
  eventType: GameEventType;
  icon: string;
  vars: Record<string, VariableValue>;
  children: GameEventChild[];
}

// ============================================================================
// Map Templates
// ============================================================================

export interface TileLightSource {
  angle: number;
  intensity: number;
  radius: number;
}

export interface TileOverrides {
  isWalkableOverride?: boolean;
  isSeeThroughOverride?: boolean;
  isContainerOverride?: boolean;
  lightSourceOverride?: TileLightSource;
}

export interface TileEventTrigger {
  eventId: string;
  isNonCombatTrigger?: boolean;
  isLookTrigger?: boolean;
}

export interface TravelTrigger {
  destinationMapName: string;
  destinationMarkerName: string;
  destinationX: number;
  destinationY: number;
}

export type MapType = 'TOWN' | 'OUTDOOR';

export const MAP_TYPES: MapType[] = ['TOWN', 'OUTDOOR'];

export interface MapTileItemEntry {
  name: string;
  quantity: number;
}

/** Materialized tile view for editor/runtime; not stored in maps.json. */
export interface CarcerMapTileTemplate {
  characters: string[];
  items: MapTileItemEntry[];
  markers: string[];
  tileOverrides?: TileOverrides;
  lightSource?: TileLightSource;
  eventTrigger?: TileEventTrigger;
  travelTrigger?: TravelTrigger;
  tilesetName: string;
  tileId: number;
}

/** 1D position on a map layer: i = y * width + x */
export interface MapTileRef {
  l: number;
  i: number;
}

export interface MapCharacterPlacement extends MapTileRef {
  name: string;
}

export interface MapItemPlacement extends MapTileRef {
  name: string;
  quantity: number;
}

export interface MapMarkerPlacement extends MapTileRef {
  name: string;
}

export interface MapEventTriggerPlacement extends MapTileRef, TileEventTrigger {}

export interface MapTravelTriggerPlacement extends MapTileRef, TravelTrigger {}

export interface MapTileOverridePlacement extends MapTileRef {
  overrides: TileOverrides;
}

export interface MapLightSourcePlacement extends MapTileRef, TileLightSource {}

export interface CarcerMapTemplate {
  name: string;
  label: string;
  type: MapType;
  width: number;
  height: number;
  spriteWidth: number;
  spriteHeight: number;
  /** Tileset name dictionary; index 0 is always empty string. */
  tilesets: string[];
  /** Z layers present on this map, e.g. [0] or [-1, 0, 1]. */
  layers: number[];
  /**
   * Dense tile graphics per layer.
   * Key = layer id string; value = row-major [tilesetIndex, tileId, ...].
   */
  tiles: Record<string, number[]>;
  characters: MapCharacterPlacement[];
  items: MapItemPlacement[];
  markers: MapMarkerPlacement[];
  eventTriggers: MapEventTriggerPlacement[];
  travelTriggers: MapTravelTriggerPlacement[];
  tileOverrides: MapTileOverridePlacement[];
  lightSources: MapLightSourcePlacement[];
}

/** Map names arranged in a grid; used when loading an Area in the game. */
export interface MapGridTemplate {
  name: string;
  label: string;
  /** Number of map slots across. */
  gridWidth: number;
  /** Number of map slots tall. */
  gridHeight: number;
  /** Required tile width for maps placed in this grid. */
  mapWidth: number;
  /** Required tile height for maps placed in this grid. */
  mapHeight: number;
  /** Map name per cell; empty string when unassigned. Row-major [y][x]. */
  cells: string[][];
}

export function createEmptyMapGridCells(
  gridWidth: number,
  gridHeight: number
): string[][] {
  const width = Math.max(1, Math.floor(gridWidth));
  const height = Math.max(1, Math.floor(gridHeight));
  return Array.from({ length: height }, () =>
    Array.from({ length: width }, () => '')
  );
}

export function resizeMapGridCells(
  cells: string[][],
  newWidth: number,
  newHeight: number
): string[][] {
  const width = Math.max(1, Math.floor(newWidth));
  const height = Math.max(1, Math.floor(newHeight));
  const next = createEmptyMapGridCells(width, height);
  const oldHeight = cells.length;
  const oldWidth = cells[0]?.length ?? 0;
  for (let y = 0; y < Math.min(oldHeight, height); y++) {
    for (let x = 0; x < Math.min(oldWidth, width); x++) {
      next[y][x] = cells[y]?.[x] ?? '';
    }
  }
  return next;
}

export function createDefaultMapGridTemplate(): MapGridTemplate {
  const gridWidth = 3;
  const gridHeight = 3;
  return {
    name: '',
    label: '',
    gridWidth,
    gridHeight,
    mapWidth: 25,
    mapHeight: 20,
    cells: createEmptyMapGridCells(gridWidth, gridHeight),
  };
}

export function normalizeMapGridTemplate(grid: MapGridTemplate): MapGridTemplate {
  const gridWidth = Math.max(1, Math.floor(grid.gridWidth) || 1);
  const gridHeight = Math.max(1, Math.floor(grid.gridHeight) || 1);
  const mapWidth = Math.max(1, Math.floor(grid.mapWidth) || 1);
  const mapHeight = Math.max(1, Math.floor(grid.mapHeight) || 1);
  const cells = resizeMapGridCells(grid.cells ?? [], gridWidth, gridHeight);
  return {
    name: grid.name ?? '',
    label: grid.label ?? '',
    gridWidth,
    gridHeight,
    mapWidth,
    mapHeight,
    cells,
  };
}

export function sanitizeMapGridTemplates(
  grids: MapGridTemplate[]
): MapGridTemplate[] {
  return grids.map(normalizeMapGridTemplate);
}

// ============================================================================
// Feat Templates (editor-only; not yet loaded by game db)
// ============================================================================

export type FeatImplementation = 'DATA' | 'CUSTOM';

export interface FeatTemplate {
  id: string;
  label: string;
  description?: string;
  implementation?: FeatImplementation;
  excludesGroup?: string;
  requiresFeats?: string[];
  effects?: FeatEffect[];
}

export interface FeatEffect {
  type: string;
  when?: string;
  target?: string;
  op?: string;
  value?: number;
}

export function createDefaultFeatTemplate(): FeatTemplate {
  return {
    id: '',
    label: '',
    description: '',
    implementation: 'DATA',
  };
}
