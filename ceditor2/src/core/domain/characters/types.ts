import type { JsonObject } from '../../database/types.js';

export const CHARACTER_TYPES = [
  'TOWNSPERSON',
  'TOWNSPERSON_STATIC',
  'ENEMY',
  'ENEMY_STATIC',
] as const;

export const CHARACTER_BEHAVIORS = [
  'MOVE_RANDOMLY',
  'IMMOBILE',
  'IMMOBILE_UNTIL_ENEMY_SPOTTED',
  'SEEK_MARKER',
  'MOVE_LEFT_RIGHT',
  'MOVE_UP_DOWN',
] as const;

export const COMBAT_BEHAVIORS = ['SEEK_AND_MELEE'] as const;

export type CharacterType = (typeof CHARACTER_TYPES)[number];
export type CharacterBehaviorName = (typeof CHARACTER_BEHAVIORS)[number];
export type CombatBehaviorName = (typeof COMBAT_BEHAVIORS)[number];

export type GenericCharacterStats = JsonObject & {
  str?: number;
  mnd?: number;
  con?: number;
  agi?: number;
  lck?: number;
};

export type WeaponMasteryStats = JsonObject & {
  edged?: number;
  pole?: number;
  blunt?: number;
  range?: number;
  unarmed?: number;
};

export type MagicMasteryStats = JsonObject & {
  mana?: number;
  abilityPower?: number;
  attunement?: number;
  faith?: number;
  lore?: number;
};

export type BodyMasteryStats = JsonObject & {
  resistPhysical?: number;
  resistMagical?: number;
  healingEffectiveness?: number;
  dr?: number;
  armorTraining?: number;
};

export type TrainableCharacterStats = JsonObject & {
  weapon?: WeaponMasteryStats;
  magic?: MagicMasteryStats;
  body?: BodyMasteryStats;
};

export type CharacterSkills = JsonObject & {
  trickery?: number;
  stealth?: number;
  social?: number;
  magicItemUse?: number;
  cooking?: number;
  acrobatics?: number;
  survival?: number;
  focus?: number;
  conditioning?: number;
};

export type CharacterStats = JsonObject & {
  generic?: GenericCharacterStats;
  trainable?: TrainableCharacterStats;
  skills?: CharacterSkills;
};

export type CharacterTalk = JsonObject & {
  talkName?: string;
  portraitName?: string;
};

export type CharacterBehavior = JsonObject & {
  behaviorName?: CharacterBehaviorName;
};

export type CharacterCombat = JsonObject & {
  hp?: number;
  mp?: number;
  dropTable?: string;
  /** Legacy loader path, retained and editable data remains lossless. */
  stats?: GenericCharacterStats;
};

export type CharacterCombatBehavior = JsonObject & {
  town?: CombatBehaviorName;
  combat?: CombatBehaviorName;
};

export type CharacterSound = JsonObject & {
  deathSoundName?: string;
  weaponSoundName?: string;
  /** Legacy aliases still accepted by the game loader. */
  deathSound?: string;
  weaponSound?: string;
};

export type CharacterStatus = JsonObject & { status: string };
export type CharacterVision = JsonObject & { radius?: number };

/** Known fields are typed while JsonObject retains forward-compatible data. */
export type CharacterRecord = JsonObject & {
  type: CharacterType;
  name: string;
  label: string;
  spritesheet: string;
  spriteOffset: number | string;
  stats?: CharacterStats;
  talk?: CharacterTalk;
  behavior?: CharacterBehavior;
  combat?: CharacterCombat;
  combatBehavior?: CharacterCombatBehavior;
  sound?: CharacterSound;
  statuses?: CharacterStatus[];
  vision?: CharacterVision;
};
