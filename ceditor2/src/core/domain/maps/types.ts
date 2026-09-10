import type { JsonObject } from '../../database/types.js';

export const MAP_TYPES = ['TOWN', 'OUTDOOR'] as const;

export const TILE_OVERLAY_VISIBILITIES = [
  'HIDDEN',
  'SHOW_EVENT_ON_TILE',
  'SHOW_TRAVEL_UP',
  'SHOW_TRAVEL_DOWN',
] as const;

export const MAP_PLACEMENT_KEYS = [
  'characters',
  'items',
  'markers',
  'eventTriggers',
  'travelTriggers',
  'tileOverrides',
  'lightSources',
] as const;

export type MapType = (typeof MAP_TYPES)[number];
export type TileOverlayVisibility = (typeof TILE_OVERLAY_VISIBILITIES)[number];
export type MapPlacementKey = (typeof MAP_PLACEMENT_KEYS)[number];

/** Raw sparse placement position. The C++ loader defaults omitted values to 0. */
export type MapTileReference = JsonObject & {
  l?: number;
  i?: number;
};

export type MapCharacterPlacement = MapTileReference & {
  name?: string;
};

export type MapItemPlacement = MapTileReference & {
  name?: string;
  quantity?: number;
};

export type MapMarkerPlacement = MapTileReference & {
  name?: string;
};

export type MapEventTriggerPlacement = MapTileReference & {
  eventId?: string;
  requiresNonCombat?: boolean;
  requiresLook?: boolean;
  /** Unknown strings are preserved; the game treats them as HIDDEN. */
  overlayVisibility?: string;
};

export type MapTravelTriggerPlacement = MapTileReference & {
  destinationMapName?: string;
  destinationMarkerName?: string;
  destinationX?: number;
  destinationY?: number;
  destinationLayer?: number;
  requiresAction?: boolean;
  /** Unknown strings are preserved; the game treats them as HIDDEN. */
  overlayVisibility?: string;
};

export type MapLightSource = JsonObject & {
  angle?: number;
  intensity?: number;
  radius?: number;
};

export type MapTileOverrides = JsonObject & {
  isWalkableOverride?: boolean;
  isSeeThroughOverride?: boolean;
  isContainerOverride?: boolean;
  lightSourceOverride?: MapLightSource;
};

export type MapTileOverridePlacement = MapTileReference & {
  overrides?: MapTileOverrides;
};

export type MapLightSourcePlacement = MapTileReference & MapLightSource;

export type MapTileLayers = JsonObject & Record<string, number[]>;

/**
 * Current compact maps.json record. Only fields needed for safe dense editing
 * are required here; loader-defaulted descriptive and placement fields remain
 * optional so parsing never invents them.
 */
export type MapRecord = JsonObject & {
  name: string;
  label?: string;
  type?: MapType;
  width: number;
  height: number;
  spriteWidth?: number;
  spriteHeight?: number;
  tilesets: string[];
  layers: number[];
  tiles: MapTileLayers;
  characters?: MapCharacterPlacement[];
  items?: MapItemPlacement[];
  markers?: MapMarkerPlacement[];
  eventTriggers?: MapEventTriggerPlacement[];
  travelTriggers?: MapTravelTriggerPlacement[];
  tileOverrides?: MapTileOverridePlacement[];
  lightSources?: MapLightSourcePlacement[];
};

export interface MapPlacements {
  readonly characters: readonly MapCharacterPlacement[] | undefined;
  readonly items: readonly MapItemPlacement[] | undefined;
  readonly markers: readonly MapMarkerPlacement[] | undefined;
  readonly eventTriggers: readonly MapEventTriggerPlacement[] | undefined;
  readonly travelTriggers: readonly MapTravelTriggerPlacement[] | undefined;
  readonly tileOverrides: readonly MapTileOverridePlacement[] | undefined;
  readonly lightSources: readonly MapLightSourcePlacement[] | undefined;
}

/** A decoded pair from a map's dense row-major graphic array. */
export interface MapCellGraphic {
  readonly tilesetIndex: number;
  readonly tileIndex: number;
}

/** Minimal mutation payload suitable for strokes and undo buffers. */
export interface MapCellPatch extends MapCellGraphic {
  readonly layer: number;
  readonly index: number;
}
