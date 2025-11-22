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
  vars?: Record<string, any>;
  children?: any[];
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

export interface TileMarker {
  name: string;
  x: number;
  y: number;
}

export interface CarcerMapTileTemplate {
  characters: string[];
  items: string[];
  markers: TileMarker[];
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

