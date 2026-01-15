import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
} from '../types/assets';
import { getTileList } from './editorEvents';

const getNeighbors = (x: number, y: number, width: number, height: number) => {
  const neighbors: number[] = [];
  if (x > 0) {
    neighbors.push(y * width + x - 1);
  }
  if (x < width - 1) {
    neighbors.push(y * width + x + 1);
  }
  if (y > 0) {
    neighbors.push((y - 1) * width + x);
  }
  if (y < height - 1) {
    neighbors.push((y + 1) * width + x);
  }
  return neighbors;
};

const getTileCoords = (ind: number, width: number) => {
  const x = ind % width;
  const y = Math.floor(ind / width);
  return { x, y };
};

const getTileId = (tile: CarcerMapTileTemplate) => {
  return `${tile.tilesetName}_${tile.tileId}`;
};

export const calculateFillIndsFloor = (
  ind: number,
  mapData: CarcerMapTemplate,
  level: number
) => {
  const mapTiles = getTileList(mapData, level);
  const tileId = getTileId(mapTiles[ind]);
  const stack: number[] = [ind];
  const visited = new Set<number>();

  const inds: number[] = [];

  while (stack.length) {
    const currentInd = stack.pop();
    if (currentInd === undefined) {
      break;
    }

    const { x, y } = getTileCoords(currentInd, mapData.width);

    if (visited.has(currentInd)) {
      continue;
    }
    visited.add(currentInd);

    if (getTileId(mapTiles[currentInd]) === tileId) {
      const neighbors = getNeighbors(x, y, mapData.width, mapData.height);
      stack.push(...neighbors);
      inds.push(currentInd);
    }
  }

  return inds;
};
