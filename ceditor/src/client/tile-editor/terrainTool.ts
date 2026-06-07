import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
  TileMetadata,
  TileTerrainBorderMeta,
  PAINTABLE_TERRAIN_BORDER_TAGS,
  TileTerrainBorderTag,
  TilesetTemplate,
} from '../types/assets';
import { EditorState, EditorStateMap, getEditorStateMap } from './editorState';
import { getTileList } from './editorEvents';

export const TERRAIN_TILESET_NAME = 'terrain_borders';

export function getTerrainTileset(
  tilesets: TilesetTemplate[],
): TilesetTemplate {
  const ts = tilesets.find((t) => t.name === TERRAIN_TILESET_NAME);
  if (!ts) {
    throw new Error(`Tileset "${TERRAIN_TILESET_NAME}" not found`);
  }
  return ts;
}

export function getPaintableTerrainTags(
  lookup: Map<string, number>,
): TileTerrainBorderTag[] {
  return PAINTABLE_TERRAIN_BORDER_TAGS.filter(
    (tag) =>
      lookup.get(terrainMetaKey({ nw: tag, ne: tag, sw: tag, se: tag })) !==
      undefined,
  );
}

export function terrainMetaKey(args: {
  nw: TileTerrainBorderTag;
  ne: TileTerrainBorderTag;
  sw: TileTerrainBorderTag;
  se: TileTerrainBorderTag;
}): string {
  return `${args.nw}|${args.ne}|${args.sw}|${args.se}`;
}

const TERRAIN_AUTOTILE_KEYS: ReadonlyArray<string> = [
  'PPPP',
  'SSPP',
  'PPSS',
  'PSPS',
  'SPSP',
  'PSPP',
  'SPPP',
  'PPPS',
  'PPSP',
  'SSPS',
  'SSSP',
  'PSSS',
  'SPSS',
];

let cachedLookup: Map<string, number> | null = null;

export function buildTerrainLookup(tileset: TilesetTemplate) {
  if (cachedLookup) {
    return cachedLookup;
  }
  const lookup = new Map<string, number>();
  console.log('building terrain lookup', lookup);
  cachedLookup = lookup;
  for (const tile of tileset.tiles) {
    if (!tile.tileTerrainBorderMeta) {
      continue;
    }
    lookup.set(
      terrainMetaKey({
        ...tile.tileTerrainBorderMeta,
      }),
      tile.id,
    );
  }

  // handle NONE case be defaulting to GRASS/DIRT
  const tags = [
    TileTerrainBorderTag.GRASS,
    TileTerrainBorderTag.DIRT,
    TileTerrainBorderTag.WATER,
    // TileTerrainBorderTag.CAVE_FLOOR,
    // TileTerrainBorderTag.SNOW,
  ];
  for (const tag of tags) {
    if (tag === TileTerrainBorderTag.GRASS) {
      for (const key of TERRAIN_AUTOTILE_KEYS) {
        if (key === 'PPPP') {
          continue;
        }
        // map grass+none to grass+dirt
        const grassDirtKey: TileTerrainBorderTag[] = key
          .split('')
          .map((c) =>
            c === 'P' ? TileTerrainBorderTag.GRASS : TileTerrainBorderTag.DIRT,
          );
        const grassNoneKey: TileTerrainBorderTag[] = key
          .split('')
          .map((c) =>
            c === 'P' ? TileTerrainBorderTag.GRASS : TileTerrainBorderTag.NONE,
          );
        const grassDirtId = lookup.get(
          terrainMetaKey({
            nw: grassDirtKey[0],
            ne: grassDirtKey[1],
            sw: grassDirtKey[2],
            se: grassDirtKey[3],
          }),
        );
        if (grassDirtId) {
          lookup.set(
            terrainMetaKey({
              nw: grassNoneKey[0],
              ne: grassNoneKey[1],
              sw: grassNoneKey[2],
              se: grassNoneKey[3],
            }),
            grassDirtId,
          );
        } else {
          console.error(
            `WARNING: Cannot map Terrain tag ${tag} none to default value:  not found for key: ${key}`,
          );
        }
      }
    } else {
      for (const key of TERRAIN_AUTOTILE_KEYS) {
        if (key === 'PPPP') {
          continue;
        }
        // map <tag>+none to <tag>+grass
        const tagGrassKey: TileTerrainBorderTag[] = key
          .split('')
          .map((c) => (c === 'P' ? tag : TileTerrainBorderTag.GRASS));
        const tagNoneKey: TileTerrainBorderTag[] = key
          .split('')
          .map((c) => (c === 'P' ? tag : TileTerrainBorderTag.NONE));
        const tagGrassId = lookup.get(
          terrainMetaKey({
            nw: tagGrassKey[0],
            ne: tagGrassKey[1],
            sw: tagGrassKey[2],
            se: tagGrassKey[3],
          }),
        );
        if (tagGrassId) {
          lookup.set(
            terrainMetaKey({
              nw: tagNoneKey[0],
              ne: tagNoneKey[1],
              sw: tagNoneKey[2],
              se: tagNoneKey[3],
            }),
            tagGrassId,
          );
        } else {
          console.error(
            `WARNING: Cannot map Terrain tag ${tag} none to default value:  not found for key: ${key}`,
          );
        }
      }
    }
  }

  return lookup;
}

export function getIndsConnectedToVertex(
  mapData: CarcerMapTemplate,
  ind: number,
  location: 'nw' | 'ne' | 'sw' | 'se',
) {
  if (location === 'nw') {
    return [ind - mapData.width - 1, ind - mapData.width, ind - 1];
  }
  if (location === 'ne') {
    return [ind - mapData.width + 1, ind - mapData.width, ind + 1];
  }
  if (location === 'sw') {
    return [ind - mapData.width - 1, ind - mapData.width, ind - 1];
  }
  if (location === 'se') {
    return [ind - mapData.width + 1, ind - mapData.width, ind + 1];
  }
  return [];
}

export function getAdjacentTileInds(
  mapData: CarcerMapTemplate,
  x: number,
  y: number,
): number[] {
  const indices: number[] = [];
  for (let dy = -1; dy <= 1; dy++) {
    for (let dx = -1; dx <= 1; dx++) {
      const tx = x + dx;
      const ty = y + dy;
      if (tx < 0 || ty < 0 || tx >= mapData.width || ty >= mapData.height) {
        continue;
      }
      if (dx === 0 && dy === 0) {
        continue;
      }
      indices.push(ty * mapData.width + tx);
    }
  }
  return indices;
}

function getAdjacentTileInd(
  mapData: CarcerMapTemplate,
  ind: number,
  location: 'nw' | 'ne' | 'sw' | 'se' | 'n' | 's' | 'e' | 'w',
) {
  const width = mapData.width;
  const height = mapData.height;
  const x = ind % width;
  const y = Math.floor(ind / width);

  let adjX = x;
  let adjY = y;

  switch (location) {
    case 'nw':
      adjX = x - 1; 
      adjY = y - 1;
      break;
    case 'ne':
      adjX = x + 1;
      adjY = y - 1;
      break;
    case 'sw':
      adjX = x - 1;
      adjY = y + 1;
      break;
    case 'se':
      adjX = x + 1;
      adjY = y + 1;
      break;
    case 'n':
      adjY = y - 1;
      break;
    case 's':
      adjY = y + 1;
      break;
    case 'e':
      adjX = x + 1;
      break;
    case 'w':
      adjX = x - 1;
      break;
    default:
      throw new Error(`Invalid location: ${location}`);
  }

  if (
    adjX < 0 ||
    adjX >= width ||
    adjY < 0 ||
    adjY >= height
  ) {
    return -1;
  }

  return adjY * width + adjX;
}

export type TerrainPaintTileChange = {
  ind: number;
  tileId: number;
};

export function getTileChangesForPaintingTerrainAt(
  mapData: CarcerMapTemplate,
  editorState: EditorState,
  mapState: EditorStateMap,
  terrainTileset: TilesetTemplate,
  lookup: Map<string, number>,
  x: number,
  y: number,
  terrainTag: TileTerrainBorderTag,
): TerrainPaintTileChange[] {
  if (x < 0 || y < 0 || x >= mapData.width || y >= mapData.height) {
    return [];
  }

  const mapTiles = getTileList(mapData, editorState.currentLevel);
  const tilesToChange: {
    ind: number;
    tileId: number;
  }[] = [];

  // base case, the tile you are currently painting should be the base terrain
  // tile
  const startInd = y * mapData.width + x;
  const baseTerrainTileId = lookup.get(
    terrainMetaKey({
      nw: terrainTag,
      ne: terrainTag,
      sw: terrainTag,
      se: terrainTag,
    }),
  );
  if (baseTerrainTileId === undefined) {
    throw new Error(`Base terrain tile not found for tag: ${terrainTag}`);
  }
  tilesToChange.push({
    ind: startInd,
    tileId: baseTerrainTileId,
  });

  const _deriveNextTileChange = (
    tile: CarcerMapTileTemplate,
    tileInd: number,
    tag: TileTerrainBorderTag,
    location: 'nw' | 'ne' | 'sw' | 'se' | 'n' | 's' | 'e' | 'w',
  ) => {
    const tileTemplate =
      tile.tilesetName === TERRAIN_TILESET_NAME
        ? terrainTileset.tiles.find((t) => t.id === tile.tileId)
        : undefined;

    const nwTag =
      tileTemplate?.tileTerrainBorderMeta?.nw ?? TileTerrainBorderTag.NONE;
    const neTag =
      tileTemplate?.tileTerrainBorderMeta?.ne ?? TileTerrainBorderTag.NONE;
    const swTag =
      tileTemplate?.tileTerrainBorderMeta?.sw ?? TileTerrainBorderTag.NONE;
    const seTag =
      tileTemplate?.tileTerrainBorderMeta?.se ?? TileTerrainBorderTag.NONE;
    const metaKeyArgs = {
      nw: tag,
      ne: tag,
      sw: tag,
      se: tag,
    };
    switch (location) {
      case 'n':
        metaKeyArgs.nw = nwTag;
        metaKeyArgs.ne = neTag;
        break;
      case 's':
        metaKeyArgs.sw = swTag;
        metaKeyArgs.se = seTag;
        break;
      case 'e':
        metaKeyArgs.ne = neTag;
        metaKeyArgs.se = seTag;
        break;
      case 'w':
        metaKeyArgs.nw = nwTag;
        metaKeyArgs.sw = swTag;
        break;
      case 'ne':
        // everything but sw
        metaKeyArgs.ne = neTag;
        metaKeyArgs.nw = nwTag;
        metaKeyArgs.se = seTag;
        break;
      case 'se':
        // everything but nw
        metaKeyArgs.se = seTag;
        metaKeyArgs.ne = neTag;
        metaKeyArgs.sw = swTag;
        break;
      case 'sw':
        // everything but ne
        metaKeyArgs.sw = swTag;
        metaKeyArgs.nw = nwTag;
        metaKeyArgs.se = seTag;
        break;
      case 'nw':
        // everything but se
        metaKeyArgs.nw = nwTag;
        metaKeyArgs.ne = neTag;
        metaKeyArgs.sw = swTag;
        break;
    }

    const nextTileId = lookup.get(terrainMetaKey(metaKeyArgs));
    if (nextTileId !== undefined) {
      return {
        ind: tileInd,
        tileId: nextTileId,
      };
    }
    return undefined;
  };

  // n case
  const nTileInd = getAdjacentTileInd(mapData, startInd, 'n');
  const nTile = nTileInd !== -1 ? mapTiles[nTileInd] : undefined;
  if (nTile) {
    const nTileChange = _deriveNextTileChange(nTile, nTileInd, terrainTag, 'n');
    if (nTileChange) {
      tilesToChange.push(nTileChange);
    }
  }

  // s case
  const sTileInd = getAdjacentTileInd(mapData, startInd, 's');
  const sTile = sTileInd !== -1 ? mapTiles[sTileInd] : undefined;
  if (sTile) {
    const sTileChange = _deriveNextTileChange(sTile, sTileInd, terrainTag, 's');
    if (sTileChange) {
      tilesToChange.push(sTileChange);
    }
  }

  // e case
  const eTileInd = getAdjacentTileInd(mapData, startInd, 'e');
  const eTile = eTileInd !== -1 ? mapTiles[eTileInd] : undefined;
  if (eTile) {
    const eTileChange = _deriveNextTileChange(eTile, eTileInd, terrainTag, 'e');
    if (eTileChange) {
      tilesToChange.push(eTileChange);
    }
  }

  // w case
  const wTileInd = getAdjacentTileInd(mapData, startInd, 'w');
  const wTile = wTileInd !== -1 ? mapTiles[wTileInd] : undefined;
  if (wTile) {
    const wTileChange = _deriveNextTileChange(wTile, wTileInd, terrainTag, 'w');
    if (wTileChange) {
      tilesToChange.push(wTileChange);
    }
  }

  // ne case
  const neTileInd = getAdjacentTileInd(mapData, startInd, 'ne');
  const neTile = neTileInd !== -1 ? mapTiles[neTileInd] : undefined;
  if (neTile) {
    const neTileChange = _deriveNextTileChange(
      neTile,
      neTileInd,
      terrainTag,
      'ne',
    );
    if (neTileChange) {
      tilesToChange.push(neTileChange);
    }
  }

  // se case
  const seTileInd = getAdjacentTileInd(mapData, startInd, 'se');
  const seTile = seTileInd !== -1 ? mapTiles[seTileInd] : undefined;
  if (seTile) {
    const seTileChange = _deriveNextTileChange(
      seTile,
      seTileInd,
      terrainTag,
      'se',
    );
    if (seTileChange) {
      tilesToChange.push(seTileChange);
    }
  }

  // sw case
  const swTileInd = getAdjacentTileInd(mapData, startInd, 'sw');
  const swTile = swTileInd !== -1 ? mapTiles[swTileInd] : undefined;
  if (swTile) {
    const swTileChange = _deriveNextTileChange(
      swTile,
      swTileInd,
      terrainTag,
      'sw',
    );
    if (swTileChange) {
      tilesToChange.push(swTileChange);
    }
  }

  // nw case
  const nwTileInd = getAdjacentTileInd(mapData, startInd, 'nw');
  const nwTile = nwTileInd !== -1 ? mapTiles[nwTileInd] : undefined;
  if (nwTile) {
    const nwTileChange = _deriveNextTileChange(
      nwTile,
      nwTileInd,
      terrainTag,
      'nw',
    );
    if (nwTileChange) {
      tilesToChange.push(nwTileChange);
    }
  }

  return tilesToChange;

  // // const nTileInd = getAdjacentTileInd(mapData, startInd, 'n');
  // // const nTile = nTileInd !== -1 ? mapTiles[nTileInd] : undefined;
  // if (nTile) {
  //   const nTileTemplate =
  //     nTile.tilesetName === TERRAIN_TILESET_NAME
  //       ? terrainTileset.tiles.find((t) => t.id === nTile.tileId)
  //       : undefined;
  //   // const nwTag = ntile.tileTerrainBorderMeta?.sw ?? TileTerrainBorderTag.NONE;
  //   const nextTileId = lookup.get(
  //     terrainMetaKey({
  //       nw:
  //         nTileTemplate?.tileTerrainBorderMeta?.nw ?? TileTerrainBorderTag.NONE,
  //       ne:
  //         nTileTemplate?.tileTerrainBorderMeta?.ne ?? TileTerrainBorderTag.NONE,
  //       sw: terrainTag,
  //       se: terrainTag,
  //     }),
  //   );
  //   if (nextTileId !== undefined) {
  //     tilesToChange.push({
  //       ind: nTileInd,
  //       tileId: nextTileId,
  //     });
  //   }
  // }

  // now we need to get the tiles that are connected to the vertex
  // const connectedInds = getIndsConnectedToVertex(mapData, startInd, 'nw');
  // for (const ind of connectedInds) {
  // tilesToChange.push({
  //   ind,
  //   tileId: lookup.get(

  // const grid = ensureTerrainVertexGrid(
  //   mapData,
  //   editorState,
  //   mapState,
  //   terrainTileset,
  //   false,
  // );

  // setVertex(grid, mapData, x, y, terrainTag);
  // setVertex(grid, mapData, x + 1, y, terrainTag);
  // setVertex(grid, mapData, x, y + 1, terrainTag);
  // setVertex(grid, mapData, x + 1, y + 1, terrainTag);

  // const affected = collectAffectedTileIndices(mapData, x, y);
  // const mapTiles = getTileList(mapData, editorState.currentLevel);
  // applyTerrainToAffectedTiles(
  //   mapData,
  //   mapTiles,
  //   grid,
  //   affected,
  //   lookup,
  //   terrainTileset,
  //   terrainTag,
  // );

  // return affected;
}

// export function findInteriorTileId(
//   lookup: TerrainLookup,
//   tag: TileTerrainBorderTag
// ): number | undefined {
//   return lookup.get(`${tag}|${tag}|${tag}|${tag}`);
// }

// export function getPaintableTerrainTags(
//   lookup: TerrainLookup
// ): TileTerrainBorderTag[] {
//   return PAINTABLE_TERRAIN_TAGS.filter(
//     (tag) => findInteriorTileId(lookup, tag) !== undefined
//   );
// }

// /**
//  * Empty map vertices are NONE. When resolving autotile corners, treat NONE as the
//  * exterior terrain that borders the paint brush:
//  * - Primary (e.g. GRASS) against void uses WATER (grass/water strip ids 0–12).
//  * - Secondary (e.g. DIRT) against void uses primary (grass/dirt strip ids 16–28).
//  * - WATER against void uses GRASS.
//  */
// export function inferExteriorVertexTag(
//   paintTag: TileTerrainBorderTag,
//   terrain?: TilesetTemplate['terrain']
// ): TileTerrainBorderTag {
//   if (paintTag === TileTerrainBorderTag.NONE) {
//     return TileTerrainBorderTag.NONE;
//   }
//   if (terrain) {
//     const { primaryTerrain, secondaryTerrain } = terrain;
//     if (paintTag === primaryTerrain) {
//       return TileTerrainBorderTag.WATER;
//     }
//     if (paintTag === secondaryTerrain) {
//       return primaryTerrain;
//     }
//   }
//   if (
//     paintTag === TileTerrainBorderTag.GRASS ||
//     paintTag === TileTerrainBorderTag.DIRT
//   ) {
//     return TileTerrainBorderTag.WATER;
//   }
//   if (paintTag === TileTerrainBorderTag.WATER) {
//     return TileTerrainBorderTag.GRASS;
//   }
//   return TileTerrainBorderTag.NONE;
// }

// export function coerceCornersForTerrainResolve(
//   corners: TileTerrainBorderMeta,
//   paintTag: TileTerrainBorderTag,
//   terrain?: TilesetTemplate['terrain']
// ): TileTerrainBorderMeta {
//   const exteriorTag = inferExteriorVertexTag(paintTag, terrain);
//   const mapVertex = (tag: TileTerrainBorderTag): TileTerrainBorderTag => {
//     if (tag !== TileTerrainBorderTag.NONE) {
//       return tag;
//     }
//     return exteriorTag;
//   };
//   return {
//     nw: mapVertex(corners.nw),
//     ne: mapVertex(corners.ne),
//     sw: mapVertex(corners.sw),
//     se: mapVertex(corners.se),
//   };
// }

// function getAllowedCornerTagsForPaint(
//   paintTag: TileTerrainBorderTag,
//   terrain?: TilesetTemplate['terrain']
// ): Set<TileTerrainBorderTag> {
//   const allowed = new Set<TileTerrainBorderTag>([
//     paintTag,
//     TileTerrainBorderTag.NONE,
//     inferExteriorVertexTag(paintTag, terrain),
//   ]);
//   if (terrain) {
//     allowed.add(terrain.primaryTerrain);
//     allowed.add(terrain.secondaryTerrain);
//   }
//   if (paintTag === TileTerrainBorderTag.GRASS) {
//     allowed.add(TileTerrainBorderTag.WATER);
//   }
//   if (paintTag === TileTerrainBorderTag.WATER) {
//     allowed.add(TileTerrainBorderTag.GRASS);
//   }
//   return allowed;
// }

// function countPaintTagCorners(
//   meta: TileTerrainBorderMeta,
//   paintTag: TileTerrainBorderTag
// ): number {
//   let count = 0;
//   if (meta.nw === paintTag) count++;
//   if (meta.ne === paintTag) count++;
//   if (meta.sw === paintTag) count++;
//   if (meta.se === paintTag) count++;
//   return count;
// }

// function countMatchingCorners(
//   meta: TileTerrainBorderMeta,
//   corners: TileTerrainBorderMeta
// ): number {
//   let count = 0;
//   if (meta.nw === corners.nw) count++;
//   if (meta.ne === corners.ne) count++;
//   if (meta.sw === corners.sw) count++;
//   if (meta.se === corners.se) count++;
//   return count;
// }

// function isAllowedFallbackTile(
//   meta: TileTerrainBorderMeta,
//   paintTag: TileTerrainBorderTag,
//   terrain?: TilesetTemplate['terrain']
// ): boolean {
//   const allowed = getAllowedCornerTagsForPaint(paintTag, terrain);
//   return [meta.nw, meta.ne, meta.sw, meta.se].every((t) => allowed.has(t));
// }

// export function resolveTerrainTileId(
//   lookup: TerrainLookup,
//   corners: TileTerrainBorderMeta,
//   tileset: TilesetTemplate,
//   paintTag?: TileTerrainBorderTag
// ): number | null {
//   const resolvedCorners = paintTag
//     ? coerceCornersForTerrainResolve(corners, paintTag, tileset.terrain)
//     : corners;

//   const exact = lookup.get(terrainMetaKey(resolvedCorners));
//   if (exact !== undefined) {
//     return exact;
//   }

//   let bestId: number | null = null;
//   let bestScore = -1;
//   let bestPaintTagScore = -1;
//   for (const tile of tileset.tiles) {
//     if (!tile.tileTerrainBorderMeta) {
//       continue;
//     }
//     if (
//       paintTag &&
//       !isAllowedFallbackTile(tile.tileTerrainBorderMeta, paintTag, tileset.terrain)
//     ) {
//       continue;
//     }
//     const score = countMatchingCorners(
//       tile.tileTerrainBorderMeta,
//       resolvedCorners
//     );
//     const paintTagScore = paintTag
//       ? countPaintTagCorners(tile.tileTerrainBorderMeta, paintTag)
//       : 0;
//     if (
//       score > bestScore ||
//       (score === bestScore && paintTagScore > bestPaintTagScore)
//     ) {
//       bestScore = score;
//       bestPaintTagScore = paintTagScore;
//       bestId = tile.id;
//     }
//   }

//   if (bestId !== null && bestScore < 4) {
//     console.warn(
//       'Terrain autotile: no exact match for',
//       terrainMetaKey(resolvedCorners),
//       'using fallback tile id',
//       bestId
//     );
//   }
//   return bestId;
// }

// export function vertexGridSize(mapData: CarcerMapTemplate): number {
//   return (mapData.width + 1) * (mapData.height + 1);
// }

// function vertexIndex(
//   mapData: CarcerMapTemplate,
//   vx: number,
//   vy: number
// ): number {
//   return vy * (mapData.width + 1) + vx;
// }

// export function getVertex(
//   grid: TileTerrainBorderTag[],
//   mapData: CarcerMapTemplate,
//   vx: number,
//   vy: number
// ): TileTerrainBorderTag {
//   if (
//     vx < 0 ||
//     vy < 0 ||
//     vx > mapData.width ||
//     vy > mapData.height
//   ) {
//     return TileTerrainBorderTag.NONE;
//   }
//   return grid[vertexIndex(mapData, vx, vy)] ?? TileTerrainBorderTag.NONE;
// }

// export function setVertex(
//   grid: TileTerrainBorderTag[],
//   mapData: CarcerMapTemplate,
//   vx: number,
//   vy: number,
//   tag: TileTerrainBorderTag
// ): void {
//   if (
//     vx < 0 ||
//     vy < 0 ||
//     vx > mapData.width ||
//     vy > mapData.height
//   ) {
//     return;
//   }
//   grid[vertexIndex(mapData, vx, vy)] = tag;
// }

// export function getTileCornersFromGrid(
//   grid: TileTerrainBorderTag[],
//   mapData: CarcerMapTemplate,
//   x: number,
//   y: number
// ): TileTerrainBorderMeta {
//   return {
//     nw: getVertex(grid, mapData, x, y),
//     ne: getVertex(grid, mapData, x + 1, y),
//     sw: getVertex(grid, mapData, x, y + 1),
//     se: getVertex(grid, mapData, x + 1, y + 1),
//   };
// }

// function preferVertexTag(
//   current: TileTerrainBorderTag,
//   incoming: TileTerrainBorderTag
// ): TileTerrainBorderTag {
//   if (incoming !== TileTerrainBorderTag.NONE) {
//     return incoming;
//   }
//   return current;
// }

// export function buildVertexGridFromMap(
//   mapData: CarcerMapTemplate,
//   level: number,
//   terrainTileset: TilesetTemplate
// ): TileTerrainBorderTag[] {
//   const size = vertexGridSize(mapData);
//   const grid = new Array<TileTerrainBorderTag>(size).fill(
//     TileTerrainBorderTag.NONE
//   );
//   const mapTiles = getTileList(mapData, level);
//   const tileMetaById = new Map<number, TileMetadata>();
//   for (const t of terrainTileset.tiles) {
//     tileMetaById.set(t.id, t);
//   }

//   for (let y = 0; y < mapData.height; y++) {
//     for (let x = 0; x < mapData.width; x++) {
//       const ind = y * mapData.width + x;
//       const mapTile = mapTiles[ind];
//       if (mapTile.tilesetName !== TERRAIN_TILESET_NAME) {
//         continue;
//       }
//       const meta = tileMetaById.get(mapTile.tileId)?.tileTerrainBorderMeta;
//       if (!meta) {
//         continue;
//       }
//       const corners: Array<[number, number, keyof TileTerrainBorderMeta]> = [
//         [x, y, 'nw'],
//         [x + 1, y, 'ne'],
//         [x, y + 1, 'sw'],
//         [x + 1, y + 1, 'se'],
//       ];
//       for (const [vx, vy, key] of corners) {
//         const idx = vertexIndex(mapData, vx, vy);
//         grid[idx] = preferVertexTag(grid[idx], meta[key]);
//       }
//     }
//   }
//   return grid;
// }

// export function getTerrainVertexGridForLevel(
//   mapState: EditorStateMap,
//   level: number
// ): TileTerrainBorderTag[] | undefined {
//   return mapState.terrainVertexGridsByLevel?.[level];
// }

// export function ensureTerrainVertexGrid(
//   mapData: CarcerMapTemplate,
//   editorState: EditorState,
//   mapState: EditorStateMap,
//   terrainTileset: TilesetTemplate,
//   forceRebuild: boolean
// ): TileTerrainBorderTag[] {
//   const level = editorState.currentLevel;
//   const existing = getTerrainVertexGridForLevel(mapState, level);
//   const needsRebuild =
//     forceRebuild ||
//     editorState.terrainGridDirty ||
//     !existing ||
//     existing.length !== vertexGridSize(mapData);

//   if (needsRebuild) {
//     if (!mapState.terrainVertexGridsByLevel) {
//       mapState.terrainVertexGridsByLevel = {};
//     }
//     mapState.terrainVertexGridsByLevel[level] = buildVertexGridFromMap(
//       mapData,
//       level,
//       terrainTileset
//     );
//     editorState.terrainGridDirty = false;
//   }
//   return mapState.terrainVertexGridsByLevel![level];
// }

// export function buildTerrainPreviewGrid(
//   grid: TileTerrainBorderTag[],
//   mapData: CarcerMapTemplate,
//   x: number,
//   y: number,
//   terrainTag: TileTerrainBorderTag
// ): TileTerrainBorderTag[] {
//   const previewGrid = [...grid];
//   setVertex(previewGrid, mapData, x, y, terrainTag);
//   setVertex(previewGrid, mapData, x + 1, y, terrainTag);
//   setVertex(previewGrid, mapData, x, y + 1, terrainTag);
//   setVertex(previewGrid, mapData, x + 1, y + 1, terrainTag);
//   return previewGrid;
// }

// export function previewTerrainCorners(
//   grid: TileTerrainBorderTag[],
//   mapData: CarcerMapTemplate,
//   x: number,
//   y: number,
//   terrainTag: TileTerrainBorderTag
// ): TileTerrainBorderMeta {
//   const previewGrid = buildTerrainPreviewGrid(
//     grid,
//     mapData,
//     x,
//     y,
//     terrainTag
//   );
//   return getTileCornersFromGrid(previewGrid, mapData, x, y);
// }

// export interface TerrainPaintPreviewCell {
//   x: number;
//   y: number;
//   tileId: number;
// }

// /** Tiles that would change if terrain were painted at (paintX, paintY). */
// export function getTerrainPaintPreviewCells(
//   mapData: CarcerMapTemplate,
//   grid: TileTerrainBorderTag[],
//   paintX: number,
//   paintY: number,
//   paintTag: TileTerrainBorderTag,
//   lookup: TerrainLookup,
//   terrainTileset: TilesetTemplate
// ): TerrainPaintPreviewCell[] {
//   const previewGrid = buildTerrainPreviewGrid(
//     grid,
//     mapData,
//     paintX,
//     paintY,
//     paintTag
//   );
//   const cells: TerrainPaintPreviewCell[] = [];
//   for (const ind of collectAffectedTileIndices(mapData, paintX, paintY)) {
//     const x = ind % mapData.width;
//     const y = Math.floor(ind / mapData.width);
//     const corners = getTileCornersFromGrid(previewGrid, mapData, x, y);
//     const tileId = resolveTerrainTileId(
//       lookup,
//       corners,
//       terrainTileset,
//       paintTag
//     );
//     if (tileId !== null) {
//       cells.push({ x, y, tileId });
//     }
//   }
//   return cells;
// }

// export function applyTerrainToAffectedTiles(
//   mapData: CarcerMapTemplate,
//   mapTiles: CarcerMapTileTemplate[],
//   grid: TileTerrainBorderTag[],
//   affectedIndices: number[],
//   lookup: TerrainLookup,
//   terrainTileset: TilesetTemplate,
//   paintTag: TileTerrainBorderTag
// ): void {
//   for (const ind of affectedIndices) {
//     const x = ind % mapData.width;
//     const y = Math.floor(ind / mapData.width);
//     const corners = getTileCornersFromGrid(grid, mapData, x, y);
//     const tileId = resolveTerrainTileId(
//       lookup,
//       corners,
//       terrainTileset,
//       paintTag
//     );
//     if (tileId === null) {
//       continue;
//     }
//     Object.assign(mapTiles[ind], {
//       tilesetName: TERRAIN_TILESET_NAME,
//       tileId,
//     });
//   }
// }

// export function invalidateTerrainVertexGridForMap(mapName: string): void {
//   const mapState = getEditorStateMap(mapName);
//   if (mapState) {
//     delete mapState.terrainVertexGridsByLevel;
//   }
// }
