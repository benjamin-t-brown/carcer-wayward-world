/*
global
G_model_tileBlocksSight
G_model_roomGetTileAt
G_model_getRoomSize
*/

const G_controller_createEmptyVisMap = (dimensions: number[]): number[][] => {
  const array: number[][] = [];
  for (var i = 0; i < dimensions[0]; ++i) {
    array.push(
      dimensions.length === 1
        ? 0
        : (G_controller_createEmptyVisMap(dimensions.slice(1)) as any)
    );
  }
  return array;
};

const G_controller_setExploredMap = (
  origExploredMap: number[][],
  visMap: number[][]
) => {
  for (let i = 0; i < visMap.length; i++) {
    for (let j = 0; j < visMap[0].length; j++) {
      origExploredMap[i][j] = origExploredMap[i][j] || visMap[i][j];
    }
  }
};

const G_controller_setVisibility = (chx: number, chy: number, room: Room) => {
  const roomSize = G_model_getRoomSize();
  const visMap = G_controller_createEmptyVisMap([roomSize, roomSize]);

  const blocksSight = (x: number, y: number) => {
    return G_model_tileBlocksSight(G_model_roomGetTileAt(room, x, y));
  };

  const castRay = (x1: number, y1: number, x2: number, y2: number) => {
    let visibility = 1;

    let dx = Math.abs(x2 - x1),
      sx = x1 < x2 ? 1 : -1;
    let dy = Math.abs(y2 - y1),
      sy = y1 < y2 ? 1 : -1;
    let err = (dx > dy ? dx : -dy) / 2;

    let ctr = 0;
    /*eslint-disable-line*/ while (1) {
      ctr++;
      if (ctr > 10) {
        break;
      }
      let checkX = x1;
      let checkY = y1;

      if (
        visMap[checkY] === undefined ||
        visMap[checkY][checkX] === undefined
      ) {
        continue;
      }
      if (visMap[checkY][checkX] === 0 || visibility === 1) {
        visMap[checkY][checkX] = visibility;
      }

      if (blocksSight(checkX, checkY)) {
        visibility = 0;
      }
      if (x1 === x2 && y1 === y2) {
        break;
      }
      let e2 = err;
      if (e2 > -dx) {
        err -= dy;
        x1 += sx;
      }
      if (e2 < dy) {
        err += dx;
        y1 += sy;
      }
    }
  };

  let boxSize = 7;
  for (let i = -boxSize; i <= boxSize; i++) {
    castRay(chx, chy, chx + i, chy + boxSize); // top side of box
    castRay(chx, chy, chx + i, chy - boxSize); // bottom side of box
    castRay(chx, chy, chx + boxSize, chy + i); // right side of box
    castRay(chx, chy, chx - boxSize, chy + i); // left side of box
  }

  visMap[chy][chx] = 1;

  G_controller_setExploredMap(room.explMap, visMap);
  room.visMap = visMap;
};3