// Consolidated asset types for the map and asset editor

import type {
  AbilityAttack,
  AbilityAttackDmg,
  StatusEffectTemplate,
} from './combat';
import { mergeAbilityAttackDmg } from './combat';

// ============================================================================
// Item Templates
// ============================================================================

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

/** A single asset that will change when an ability (or its attack) is deleted. */
export interface AbilityDeleteImpact {
  kind: 'item' | 'statusEffect';
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
): AbilityDeleteImpact[] {
  const newAttacks = currentAttacks.filter((_, i) => i !== deletedAttackIndex);
  const impacts: AbilityDeleteImpact[] = [];

  for (const item of items) {
    const weapon = item.weapon;
    if (!weapon || weapon.abilityName !== abilityName) {
      continue;
    }

    const afterWeapon = reconcileWeaponAfterAttackDelete(
      weapon,
      deletedAttackIndex,
      newAttacks,
    );
    if (weaponStorageSnapshot(weapon) === weaponStorageSnapshot(afterWeapon)) {
      continue;
    }

    impacts.push({
      kind: 'item',
      name: item.name,
      label: item.label || item.name,
      lines: describeWeaponAttackDeleteImpact(
        weapon,
        deletedAttackIndex,
        currentAttacks.length,
      ),
    });
  }

  return impacts;
}

export function applyWeaponAttackDeleteToItems(
  abilityName: string,
  deletedAttackIndex: number,
  newAttacks: AbilityAttack[],
  items: ItemTemplate[],
): ItemTemplate[] {
  return items.map((item) => {
    const weapon = item.weapon;
    if (!weapon || weapon.abilityName !== abilityName) {
      return item;
    }
    return {
      ...item,
      weapon: reconcileWeaponAfterAttackDelete(
        weapon,
        deletedAttackIndex,
        newAttacks,
      ),
    };
  });
}

export function clearItemWeaponForDeletedAbility(
  _weapon: ItemWeaponConfig,
): ItemWeaponConfig {
  return { abilityName: '' };
}

export function describeItemAbilityDeleteImpact(weapon: ItemWeaponConfig): string[] {
  const lines: string[] = ['Clear weapon base ability reference.'];
  if (weapon.dmgOverrides?.length || weapon.dmg) {
    lines.push('Remove all stored damage overrides.');
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
    if (!weapon || weapon.abilityName !== abilityName) {
      continue;
    }
    const afterWeapon = clearItemWeaponForDeletedAbility(weapon);
    if (weaponStorageSnapshot(weapon) === weaponStorageSnapshot(afterWeapon)) {
      continue;
    }
    impacts.push({
      kind: 'item',
      name: item.name,
      label: item.label || item.name,
      lines: describeItemAbilityDeleteImpact(weapon),
    });
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
    const weapon = item.weapon;
    if (!weapon || weapon.abilityName !== abilityName) {
      return item;
    }
    return {
      ...item,
      weapon: clearItemWeaponForDeletedAbility(weapon),
    };
  });
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
  itemType: string;
  name: string;
  label: string;
  icon: string;
  description: string;
  weight: number;
  value: number;
  stackable?: boolean;
  // Optional fields
  itemUsability?: string;
  itemUsabilityArgs?: {
    itemUsabilityType: string;
    intArgs?: number[];
    stringArgs?: string[];
  };
  /** Names of entries in status-effects.json applied when this item is used/equipped */
  statusEffects?: string[];
  weapon?: ItemWeaponConfig;
}

export function sanitizeItemTemplates(
  items: ItemTemplate[],
  abilities: { name: string; attacks?: AbilityAttack[] }[] = [],
): ItemTemplate[] {
  return items.map((item) => {
    if (item.weapon && !isWeaponItemType(item.itemType)) {
      const { weapon: _weapon, ...withoutWeapon } = item;
      item = withoutWeapon;
    }

    if (!item.weapon) {
      return item;
    }
    const weapon = item.weapon as LegacyItemWeapon;
    const baseAttacks =
      abilities.find((a) => a.name === weapon.abilityName)?.attacks ?? [];

    if ('dmgMin' in weapon || 'dmgMax' in weapon) {
      const { dmgMin: _min, dmgMax: _max, ...rest } = weapon;
      if (!rest.abilityName) {
        return { ...item, weapon: { abilityName: '' } };
      }
      return {
        ...item,
        weapon: normalizeItemWeaponConfig(rest, baseAttacks),
      };
    }

    if (weapon.dmg || weapon.attackIndex !== undefined) {
      return {
        ...item,
        weapon: normalizeItemWeaponConfig(weapon, baseAttacks),
      };
    }

    return item;
  });
}

// ============================================================================
// Character Templates
// ============================================================================

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

export interface CharacterTemplate {
  type: string;
  name: string;
  label: string;
  spritesheet: string;
  spriteOffset: number;
  stats?: CharacterStats;
  // Optional fields
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
  };
  sound?: {
    deathSoundName?: string;
    weaponSoundName?: string;
  };
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

export interface TileMetadata {
  id: number;
  description?: string;
  stepSound?: TileStepSound;
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
}

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

export interface Choice {
  text: string;
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
  | GameEventChildEnd;

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

export interface CarcerMapTileTemplate {
  characters: string[];
  items: string[];
  markers: string[];
  tileOverrides?: TileOverrides;
  lightSource?: TileLightSource;
  eventTrigger?: TileEventTrigger;
  travelTrigger?: TravelTrigger;
  tilesetName: string;
  tileId: number;
}

export interface CarcerMapTemplate {
  name: string;
  label: string;
  type: MapType;
  width: number;
  height: number;
  spriteWidth: number;
  spriteHeight: number;
  levels: Record<string, CarcerMapTileTemplate[]>;
}
