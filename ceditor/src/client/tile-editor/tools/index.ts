import { calculateFillIndsFloor } from '../fill';
import { CarcerMapTemplate } from '../../types/assets';
import { createDefaultCarcerMapTile } from '../../components/MapTemplateForm';
import {
  EditorState,
  getEditorStateMap,
  getPaintMapName,
} from '../editorState';
import { getTileList } from '../editorEvents';
import type { PaintAction } from '../paintTools';
import { MapTool } from './types';

export type { MapTool } from './types';

/** Restore mapTiles[tileInds[i]] from prevRefData[i]. The common undo shape. */
function restorePrevRefData(action: PaintAction, map: CarcerMapTemplate): void {
  const mapTiles = getTileList(map);
  for (let i = 0; i < action.data.tileInds.length; i++) {
    const ind = action.data.tileInds[i];
    if (i < action.data.prevRefData.length) {
      mapTiles[ind] = structuredClone(action.data.prevRefData[i]);
    }
  }
}

const draw: MapTool = {
  id: 'DRAW',
  icon: '🖌️',
  title: 'Draw tool',
  shortcut: 'b',
  shortcutBlockedByCtrl: false,
  row: 'primary',
  apply() {
    // Draw writes happen in update().
  },
  update(action, map) {
    const mapTiles = getTileList(map);
    for (const ind of action.data.tileInds) {
      const startX = ind % map.width;
      const startY = Math.floor(ind / map.width);
      const floorDrawBrush = action.data.floorDrawBrush;
      if (floorDrawBrush?.length) {
        for (const bt of floorDrawBrush) {
          const newX = startX + bt.xOffset;
          const newY = startY + bt.yOffset;
          const newInd = newY * map.width + newX;

          if (newX < 0 || newX >= map.width || newY < 0 || newY >= map.height) {
            continue;
          }

          Object.assign(mapTiles[newInd], bt.originalTile.ref);
        }
      } else {
        Object.assign(mapTiles[ind], action.data.paintTileRef);
      }
    }
  },
  undo(action, map) {
    const mapTiles = getTileList(map);
    for (let i = 0; i < action.data.tileInds.length; i++) {
      const ind = action.data.tileInds[i];
      if (i < action.data.prevRefData.length) {
        mapTiles[ind] = structuredClone(action.data.prevRefData[i]);
      }
    }
    for (let i = 0; i < action.data.extraTileInds.length; i++) {
      const ind = action.data.extraTileInds[i];
      if (i < action.data.extraPrevRefData.length) {
        mapTiles[ind] = structuredClone(action.data.extraPrevRefData[i]);
      }
    }
  },
};

const erase: MapTool = {
  id: 'ERASE',
  icon: '✖️',
  title: 'Erase tool',
  shortcut: 'e',
  shortcutBlockedByCtrl: true,
  row: 'secondary',
  apply() {
    // Erase writes happen in update().
  },
  update(action, map) {
    const mapTiles = getTileList(map);
    for (const ind of action.data.tileInds) {
      mapTiles[ind] = createDefaultCarcerMapTile();
    }
  },
  undo: restorePrevRefData,
};

const eraseMeta: MapTool = {
  id: 'ERASE_META',
  icon: '🗑',
  title: 'Erase metadata tool',
  row: 'secondary',
  apply() {
    // Erase-metadata writes happen in update().
  },
  update(action, map) {
    const mapTiles = getTileList(map);
    for (const ind of action.data.tileInds) {
      const tile = mapTiles[ind];
      tile.characters = [];
      tile.items = [];
      tile.markers = [];
      delete tile.tileOverrides;
      delete tile.lightSource;
      delete tile.eventTrigger;
      delete tile.travelTrigger;
    }
  },
  undo(action, map) {
    const mapTiles = getTileList(map);
    for (let i = 0; i < action.data.tileInds.length; i++) {
      const ind = action.data.tileInds[i];
      if (i < action.data.prevRefData.length) {
        const prevTile = action.data.prevRefData[i];
        const currentTile = mapTiles[ind];
        currentTile.characters = structuredClone(prevTile.characters);
        currentTile.items = structuredClone(prevTile.items);
        if (prevTile.tileOverrides) {
          currentTile.tileOverrides = structuredClone(prevTile.tileOverrides);
        } else {
          delete currentTile.tileOverrides;
        }
        if (prevTile.lightSource) {
          currentTile.lightSource = structuredClone(prevTile.lightSource);
        } else {
          delete currentTile.lightSource;
        }
        if (prevTile.eventTrigger) {
          currentTile.eventTrigger = structuredClone(prevTile.eventTrigger);
        } else {
          delete currentTile.eventTrigger;
        }
        if (prevTile.travelTrigger) {
          currentTile.travelTrigger = structuredClone(prevTile.travelTrigger);
        } else {
          delete currentTile.travelTrigger;
        }
      }
    }
  },
};

const fill: MapTool = {
  id: 'FILL',
  icon: '🪣',
  title: 'Fill tool',
  shortcut: 'f',
  shortcutBlockedByCtrl: false,
  row: 'primary',
  apply(action, map, editorState) {
    const mapTiles = getTileList(map);
    const ind =
      getEditorStateMap(getPaintMapName(editorState))?.hoveredTileIndex ?? -1;
    const fillIndsFloor = calculateFillIndsFloor(
      ind,
      map,
      editorState.currentLevel,
    );
    action.data.tileInds = fillIndsFloor;
    for (let i = 0; i < fillIndsFloor.length; i++) {
      const tileInd = fillIndsFloor[i];
      if (i >= action.data.prevRefData.length) {
        action.data.prevRefData.push(structuredClone(mapTiles[tileInd]));
      }
      mapTiles[tileInd] = Object.assign(
        mapTiles[tileInd],
        action.data.paintTileRef,
      );
    }
  },
  undo: restorePrevRefData,
};

const deleteFill: MapTool = {
  id: 'DELETE_FILL',
  icon: '🪣',
  iconClassName: 'tile-editor-tool-icon-bucket-delete',
  title: 'Delete fill tool',
  row: 'secondary',
  apply(action, map, editorState) {
    const mapTiles = getTileList(map);
    const ind =
      getEditorStateMap(getPaintMapName(editorState))?.hoveredTileIndex ?? -1;
    const fillIndsFloor = calculateFillIndsFloor(
      ind,
      map,
      editorState.currentLevel,
    );
    action.data.tileInds = fillIndsFloor;
    for (let i = 0; i < fillIndsFloor.length; i++) {
      const tileInd = fillIndsFloor[i];
      if (i >= action.data.prevRefData.length) {
        action.data.prevRefData.push(structuredClone(mapTiles[tileInd]));
      }
      mapTiles[tileInd] = createDefaultCarcerMapTile();
    }
  },
  undo: restorePrevRefData,
};

const select: MapTool = {
  id: 'SELECT',
  icon: '👆',
  title: 'Select tool',
  shortcut: 's',
  shortcutBlockedByCtrl: true,
  row: 'primary',
  apply(action, map) {
    const mapTiles = getTileList(map);
    if (
      action.data.tileInds.length >= 2 &&
      action.data.prevRefData.length >= 2
    ) {
      const sourceTileIndex = action.data.startInd;
      const destTileIndex = action.data.endInd;

      if (
        sourceTileIndex >= 0 &&
        destTileIndex >= 0 &&
        sourceTileIndex !== destTileIndex
      ) {
        const sourceTile = mapTiles[sourceTileIndex];
        const destTile = mapTiles[destTileIndex];

        destTile.characters = [
          ...(destTile.characters || []),
          ...(sourceTile.characters || []),
        ];
        destTile.items = [...(destTile.items || []), ...(sourceTile.items || [])];
        destTile.markers = [
          ...(destTile.markers || []),
          ...(sourceTile.markers || []),
        ];

        if (sourceTile.tileOverrides) {
          destTile.tileOverrides = {
            ...sourceTile.tileOverrides,
            ...(destTile.tileOverrides || {}),
          };
        }

        if (sourceTile.lightSource && !destTile.lightSource) {
          destTile.lightSource = sourceTile.lightSource;
        }

        if (sourceTile.eventTrigger && !destTile.eventTrigger) {
          destTile.eventTrigger = sourceTile.eventTrigger;
        }

        if (sourceTile.travelTrigger && !destTile.travelTrigger) {
          destTile.travelTrigger = structuredClone(sourceTile.travelTrigger);
        }

        sourceTile.characters = [];
        sourceTile.items = [];
        sourceTile.markers = [];
        delete sourceTile.tileOverrides;
        delete sourceTile.lightSource;
        delete sourceTile.eventTrigger;
        delete sourceTile.travelTrigger;
      }
    }
  },
  undo(action, map) {
    const mapTiles = getTileList(map);
    if (
      action.data.tileInds.length >= 2 &&
      action.data.prevRefData.length >= 2
    ) {
      const sourceTileIndex = action.data.startInd;
      const destTileIndex = action.data.endInd;

      if (sourceTileIndex >= 0 && destTileIndex >= 0) {
        if (0 < action.data.prevRefData.length) {
          mapTiles[sourceTileIndex] = structuredClone(
            action.data.prevRefData[0],
          );
        }
        if (1 < action.data.prevRefData.length) {
          mapTiles[destTileIndex] = structuredClone(action.data.prevRefData[1]);
        }
      }
    }
  },
};

const clone: MapTool = {
  id: 'CLONE',
  icon: '📋',
  title: 'Clone tool',
  shortcut: 'c',
  shortcutBlockedByCtrl: true,
  row: 'primary',
  apply(action, map) {
    const mapTiles = getTileList(map);
    if (
      action.data.tileInds.length >= 2 &&
      action.data.prevRefData.length >= 2
    ) {
      const sourceTileIndex = action.data.startInd;
      const destTileIndex = action.data.endInd;

      if (
        sourceTileIndex >= 0 &&
        destTileIndex >= 0 &&
        sourceTileIndex !== destTileIndex
      ) {
        const sourceTile = mapTiles[sourceTileIndex];
        const destTile = mapTiles[destTileIndex];

        destTile.characters = [
          ...(destTile.characters || []),
          ...(sourceTile.characters || []),
        ];
        destTile.items = [...(destTile.items || []), ...(sourceTile.items || [])];
        destTile.markers = [
          ...(destTile.markers || []),
          ...(sourceTile.markers || []),
        ];

        if (sourceTile.tileOverrides) {
          destTile.tileOverrides = {
            ...sourceTile.tileOverrides,
            ...(destTile.tileOverrides || {}),
          };
        }

        if (sourceTile.lightSource && !destTile.lightSource) {
          destTile.lightSource = structuredClone(sourceTile.lightSource);
        }

        if (sourceTile.eventTrigger && !destTile.eventTrigger) {
          destTile.eventTrigger = structuredClone(sourceTile.eventTrigger);
        }

        if (sourceTile.travelTrigger && !destTile.travelTrigger) {
          destTile.travelTrigger = structuredClone(sourceTile.travelTrigger);
        }
      }
    }
  },
  undo(action, map) {
    const mapTiles = getTileList(map);
    if (
      action.data.tileInds.length >= 2 &&
      action.data.prevRefData.length >= 2
    ) {
      const destTileIndex = action.data.endInd;
      if (destTileIndex >= 0) {
        if (1 < action.data.prevRefData.length) {
          mapTiles[destTileIndex] = structuredClone(action.data.prevRefData[1]);
        }
      }
    }
  },
};

const terrain: MapTool = {
  id: 'TERRAIN',
  icon: '🏔️',
  title: 'Terrain tool (T)',
  shortcut: 't',
  shortcutBlockedByCtrl: true,
  row: 'primary',
  apply() {
    // Terrain writes happen in onActionUpdate -> applyTerrainPaintUpdate.
  },
  update() {
    // Terrain writes happen in onActionUpdate -> applyTerrainPaintUpdate.
  },
  undo: restorePrevRefData,
};

/** Every tool, keyed by its id (== PaintActionType value). */
export const TOOLS: Record<string, MapTool> = {
  [select.id]: select,
  [draw.id]: draw,
  [fill.id]: fill,
  [deleteFill.id]: deleteFill,
  [clone.id]: clone,
  [terrain.id]: terrain,
  [erase.id]: erase,
  [eraseMeta.id]: eraseMeta,
};

/** Overlay/keyboard iteration order (primary row, then secondary). */
export const TOOL_LIST: readonly MapTool[] = [
  select,
  draw,
  fill,
  clone,
  terrain,
  erase,
  eraseMeta,
  deleteFill,
];
