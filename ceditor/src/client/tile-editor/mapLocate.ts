import { CarcerMapTemplate } from '../types/assets';
import { centerViewOnTile } from './editorEvents';
import {
  getEditorState,
  setSoleSelectedTile,
  updateEditorState,
} from './editorState';
import { tileXY } from '../utils/mapIndex';
import type { MapEditorController } from './MapEditorController';

export interface MapTileLocation {
  level: number;
  tileIndex: number;
}

/** A tile whose travel trigger targets a map + marker destination. */
export interface MapMarkerReference extends MapTileLocation {
  x: number;
  y: number;
  sourceMapName: string;
  sourceMapLabel: string;
}

const MAP_CANVAS_ID = 'map-canvas-canvas';

/** Unique character names placed on any layer of the map (sorted). */
export function collectCharacterNamesOnMap(map: CarcerMapTemplate): string[] {
  const names = new Set<string>();
  for (const placement of map.characters ?? []) {
    const trimmed = placement.name.trim();
    if (trimmed) {
      names.add(trimmed);
    }
  }
  return [...names].sort((a, b) => a.localeCompare(b));
}

/** Unique marker names placed on any layer of the map (sorted). */
export function collectMarkerNamesOnMap(map: CarcerMapTemplate): string[] {
  const names = new Set<string>();
  for (const placement of map.markers ?? []) {
    const trimmed = placement.name.trim();
    if (trimmed) {
      names.add(trimmed);
    }
  }
  return [...names].sort((a, b) => a.localeCompare(b));
}

/** Tiles (any map) with a travel trigger pointing at this map + marker. */
export function findTravelTriggerReferencesToMarker(
  destinationMap: CarcerMapTemplate,
  markerName: string,
  allMaps: CarcerMapTemplate[],
): MapMarkerReference[] {
  const destMapName = destinationMap.name;
  const name = markerName.trim();
  if (!name || !destMapName) {
    return [];
  }

  const refs: MapMarkerReference[] = [];
  for (const sourceMap of allMaps) {
    for (const trigger of sourceMap.travelTriggers ?? []) {
      if (
        trigger.destinationMapName.trim() === destMapName &&
        trigger.destinationMarkerName.trim() === name
      ) {
        const { x, y } = tileXY(trigger.i, sourceMap.width);
        refs.push({
          sourceMapName: sourceMap.name,
          sourceMapLabel: sourceMap.label,
          level: trigger.l,
          tileIndex: trigger.i,
          x,
          y,
        });
      }
    }
  }

  return refs.sort((a, b) => {
    const mapCmp = a.sourceMapLabel.localeCompare(b.sourceMapLabel);
    if (mapCmp !== 0) {
      return mapCmp;
    }
    if (a.level !== b.level) {
      return b.level - a.level;
    }
    if (a.y !== b.y) {
      return a.y - b.y;
    }
    return a.x - b.x;
  });
}

/** First tile on a map that has this marker placed (markers list). */
export function findMarkerOnMap(
  map: CarcerMapTemplate,
  markerName: string,
): MapTileLocation | null {
  const name = markerName.trim();
  if (!name) {
    return null;
  }
  const placement = (map.markers ?? []).find((m) => m.name === name);
  if (!placement) {
    return null;
  }
  return { level: placement.l, tileIndex: placement.i };
}

export function findCharacterOnMap(
  map: CarcerMapTemplate,
  characterName: string,
): MapTileLocation | null {
  const name = characterName.trim();
  if (!name) {
    return null;
  }
  const placement = (map.characters ?? []).find((c) => c.name === name);
  if (!placement) {
    return null;
  }
  return { level: placement.l, tileIndex: placement.i };
}

export function locateOnCurrentMap(
  controller: MapEditorController,
  map: CarcerMapTemplate,
  location: MapTileLocation,
): boolean {
  const mapName = getEditorState(controller).selectedMapName;
  if (!mapName) {
    return false;
  }

  updateEditorState(controller, { currentLevel: location.level });
  setSoleSelectedTile(controller, mapName, location.tileIndex);

  const canvas = document.getElementById(MAP_CANVAS_ID);
  if (canvas instanceof HTMLCanvasElement) {
    centerViewOnTile(controller, canvas, map, location.tileIndex);
  }

  return true;
}
