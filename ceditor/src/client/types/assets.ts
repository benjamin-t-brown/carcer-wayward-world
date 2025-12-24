// Consolidated asset types for the map and asset editor

// ============================================================================
// Item Templates
// ============================================================================

export interface ItemTemplate {
  itemType: string;
  name: string;
  label: string;
  icon: string;
  description: string;
  weight: number;
  value: number;
  // Optional fields
  itemUsability?: string;
  itemUsabilityArgs?: {
    itemUsabilityType: string;
    intArgs?: number[];
    stringArgs?: string[];
  };
  statusEffects?: Array<{
    statusEffectType: string;
    name?: string;
    label?: string;
    description?: string;
  }>;
  weapon?: {
    dmgMin?: number;
    dmgMax?: number;
  };
}

// ============================================================================
// Character Templates
// ============================================================================

export interface CharacterTemplate {
  type: string;
  name: string;
  label: string;
  spritesheet: string;
  spriteOffset: number;
  // Optional fields
  talk?: {
    talkName?: string;
    portraitName?: string;
  };
  behavior?: {
    behaviorName?: string;
  };
  combat?: {
    stats?: {
      str?: number;
      mnd?: number;
      con?: number;
      agi?: number;
      lck?: number;
    };
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
  width: number;
  height: number;
  spriteWidth: number;
  spriteHeight: number;
  tiles: CarcerMapTileTemplate[];
}
