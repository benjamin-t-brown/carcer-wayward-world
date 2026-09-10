import {
  getScreenMouseCoords,
  getTileList,
  screenCoordsToTileIndex,
} from './editorEvents';
import { CarcerMapTemplate, CarcerMapTileTemplate } from '../types/assets';
import type { MapEditorController } from './MapEditorController';

export interface FloorBrushData {
  xOffset: number;
  yOffset: number;
  originalTile: {
    ref: CarcerMapTileTemplate;
  };
}

/** Information about the tile under the mouse. */
export const calculateHoveredTile = (
  controller: MapEditorController,
  mapData: CarcerMapTemplate,
  panzoomCanvas: HTMLCanvasElement,
) => {
  const [mouseX, mouseY] = getScreenMouseCoords(controller);

  let tileX = -1;
  let tileY = -1;
  let ind = -1;

  const mapTiles = getTileList(controller, mapData);
  if (mapTiles.length > 0) {
    const [tileInd, localTileX, localTileY] = screenCoordsToTileIndex(
      controller,
      mouseX,
      mouseY,
      mapData,
      panzoomCanvas,
    );
    ind = tileInd;
    tileX = localTileX;
    tileY = localTileY;
  }

  return { x: tileX, y: tileY, ind };
};
