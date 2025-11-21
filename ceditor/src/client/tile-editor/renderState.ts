// import { getMapDataOptional } from './mapCache';
import {
  getScreenMouseCoords,
  // onTileHoverIndChange,
  screenCoordsToTileIndex,
} from './editorEvents';
// import { MapTile } from 'shared';
// import { tileIdToSpriteName } from './tileset';
import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
} from '../components/MapTemplateForm';

// let hoveredTileInd = -1;
// let fillIndsFloor: number[] = [];
// let rectSelectTileIndStart = -1;
// let rectSelectTileIndEnd = -1;
// let hoveredTileData = {
//   x: -1,
//   y: -1,
//   ind: -1,
//   // indTileset: -1,
//   // floorId: 0,
//   // topId: 0,
// };
export interface FloorBrushData {
  xOffset: number;
  yOffset: number;
  originalTile: {
    ref: CarcerMapTileTemplate;
  };
}
// let tileFloorBrush: FloorBrushData[] = [];

// export const setTileFloorBrush = (brush: typeof tileFloorBrush) => {
//   tileFloorBrush = brush;
// };
// export const getTileFloorBrush = () => {
//   return tileFloorBrush;
// };

// export const setHoveredTileInd = (mapData: CarcerMapTemplate, ind: number) => {
//   if (hoveredTileInd !== ind) {
//     const prevInd = hoveredTileInd;
//     hoveredTileInd = ind;
//     onTileHoverIndChange(mapData, prevInd, ind);
//   }
// };
// export const getHoveredTileInd = () => {
//   return hoveredTileInd;
// };
// export const setFillIndsFloor = (inds: number[]) => {
//   fillIndsFloor = inds;
// };
// export const getFillIndsFloor = () => {
//   return fillIndsFloor;
// };

// export const setRectSelectTileIndStart = (ind: number) => {
//   rectSelectTileIndStart = ind;
// };
// export const setRectSelectTileIndEnd = (ind: number) => {
//   rectSelectTileIndEnd = ind;
// };
// export const getRectSelectTileInds = () => {
//   return [rectSelectTileIndStart, rectSelectTileIndEnd];
// };

// export const getHoveredTileData = () => {
//   return hoveredTileData;
// };
// export const setHoveredTileData = (data: typeof hoveredTileData) => {
//   hoveredTileData = data;
// };

// information about the tile under the mouse
export const calculateHoveredTile = (
  mapData: CarcerMapTemplate,
  panzoomCanvas: HTMLCanvasElement
) => {
  const [mouseX, mouseY] = getScreenMouseCoords();

  let tileX = -1;
  let tileY = -1;
  let ind = -1;
  let tilesetName = '';
  const topId = 0;
  let indTileset = -1;
  if (mapData?.tiles) {
    const [tileInd, localTileX, localTileY] = screenCoordsToTileIndex(
      mouseX,
      mouseY,
      mapData,
      panzoomCanvas
    );
    ind = tileInd;
    tileX = localTileX;
    tileY = localTileY;
    // tileX = ind % mapData.width;
    // tileY = Math.floor(ind / mapData.width);
    if (ind !== -1) {
      tilesetName = mapData.tiles[ind]?.tilesetName ?? '';
    }
    // const sprite = tileIdToSpriteName(floorId) || '';
    // const arr = sprite.split('_');
    // indTileset = parseInt(arr[arr.length - 1]);
    // if (isNaN(indTileset)) {
    //   indTileset = -1;
    // }
  }

  return {
    x: tileX,
    y: tileY,
    ind,
    // indTileset,
    // floorId,
    // topId,
  };
};
