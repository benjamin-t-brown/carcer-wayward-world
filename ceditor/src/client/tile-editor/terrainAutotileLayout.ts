import {
  TileMetadata,
  TileTerrainBorderMeta,
  TileTerrainBorderTag,
  TilesetTemplate,
} from '../types/assets';

/** Number of tiles in the standard autotile strip (9 + 4). */
export const TERRAIN_AUTOTILE_COUNT = 13;

// /**
//  * 13-tile strip order used in tilesets.json (e.g. terrain_borders ids 16–28,
//  * grass primary + dirt secondary). corners: nw, ne, sw, se — P = primary, S = secondary.
//  */
export const TERRAIN_AUTOTILE_LAYOUT: ReadonlyArray<{
  label: string;
  corners: string;
}> = [
  { label: 'center', corners: 'PPPP' },
  { label: 'edge N', corners: 'SSPP' },
  { label: 'edge S', corners: 'PPSS' },
  { label: 'edge E', corners: 'PSPS' },
  { label: 'edge W', corners: 'SPSP' },
  { label: 'outer NE', corners: 'PSPP' },
  { label: 'outer NW', corners: 'SPPP' },
  { label: 'outer SW', corners: 'PPPS' },
  { label: 'outer SE', corners: 'PPSP' },
  { label: 'inner NW', corners: 'SSPS' },
  { label: 'inner NE', corners: 'SSSP' },
  { label: 'inner SE', corners: 'PSSS' },
  { label: 'inner SW', corners: 'SPSS' },
];

function cornersFromPattern(
  pattern: string,
  primary: TileTerrainBorderTag,
  secondary: TileTerrainBorderTag,
): TileTerrainBorderMeta {
  const mapChar = (c: string): TileTerrainBorderTag => {
    if (c === 'P') return primary;
    if (c === 'S') return secondary;
    throw new Error(`Invalid corner char: ${c}`);
  };
  return {
    nw: mapChar(pattern[0]),
    ne: mapChar(pattern[1]),
    sw: mapChar(pattern[2]),
    se: mapChar(pattern[3]),
  };
}

export function getTerrainLayoutMeta(
  layoutIndex: number,
  primary: TileTerrainBorderTag,
  secondary: TileTerrainBorderTag,
): TileTerrainBorderMeta | null {
  if (layoutIndex < 0 || layoutIndex >= TERRAIN_AUTOTILE_COUNT) {
    return null;
  }
  return cornersFromPattern(
    TERRAIN_AUTOTILE_LAYOUT[layoutIndex].corners,
    primary,
    secondary,
  );
}

// export function getTerrainLayoutIndex(
//   tileId: number,
//   startTileId: number,
// ): number | null {
//   const offset = tileId - startTileId;
//   if (offset < 0 || offset >= TERRAIN_AUTOTILE_COUNT) {
//     return null;
//   }
//   return offset;
// }

// export function applyTerrainLayoutFromStartTile(tileset: TilesetTemplate): {
//   ok: boolean;
//   error?: string;
// } {
//   const terrain = tileset.terrain;
//   if (!terrain) {
//     return { ok: false, error: 'Tileset has no terrain configuration' };
//   }
//   const startId = terrain.startTileId ?? 0;
//   const { primaryTerrain: primary, secondaryTerrain: secondary } = terrain;

//   if (primary === secondary) {
//     return { ok: false, error: 'Primary and secondary terrain must differ' };
//   }

//   const tileById = new Map<number, TileMetadata>();
//   for (const t of tileset.tiles) {
//     tileById.set(t.id, t);
//   }

//   for (let i = 0; i < TERRAIN_AUTOTILE_COUNT; i++) {
//     const tileId = startId + i;
//     const tile = tileById.get(tileId);
//     if (!tile) {
//       return {
//         ok: false,
//         error: `Missing tile id ${tileId} (need ids ${startId}–${startId + TERRAIN_AUTOTILE_COUNT - 1})`,
//       };
//     }
//     const meta = getTerrainLayoutMeta(i, primary, secondary);
//     if (meta) {
//       tile.tileTerrainBorderMeta = meta;
//     }
//   }

//   return { ok: true };
// }

// export function validateTerrainTileset(tileset: TilesetTemplate): {
//   ok: boolean;
//   error?: string;
// } {
//   const terrain = tileset.terrain;
//   if (!terrain) {
//     return { ok: true };
//   }

//   if (!terrain.primaryTerrain || !terrain.secondaryTerrain) {
//     return {
//       ok: false,
//       error: `${tileset.name}: terrain requires primary and secondary`,
//     };
//   }
//   if (terrain.primaryTerrain === terrain.secondaryTerrain) {
//     return {
//       ok: false,
//       error: `${tileset.name}: primary and secondary terrain must differ`,
//     };
//   }

//   const startId = terrain.startTileId ?? 0;
//   const tileById = new Map(tileset.tiles.map((t) => [t.id, t]));
//   const { primaryTerrain: primary, secondaryTerrain: secondary } = terrain;

//   for (let i = 0; i < TERRAIN_AUTOTILE_COUNT; i++) {
//     const tile = tileById.get(startId + i);
//     if (!tile) {
//       return {
//         ok: false,
//         error: `${tileset.name}: missing tile id ${startId + i} for autotile strip`,
//       };
//     }
//     const meta = tile.tileTerrainBorderMeta;
//     if (!meta) {
//       return {
//         ok: false,
//         error: `${tileset.name}: tile ${startId + i} missing terrain meta (run Apply layout)`,
//       };
//     }
//     const allowed = new Set([primary, secondary]);
//     for (const corner of [meta.nw, meta.ne, meta.sw, meta.se]) {
//       if (!allowed.has(corner)) {
//         return {
//           ok: false,
//           error: `${tileset.name}: tile ${tile.id} corner ${corner} must be ${primary} or ${secondary}`,
//         };
//       }
//     }
//   }

//   return { ok: true };
// }

// export function getDefaultTerrainConfig(): TilesetTemplate['terrain'] {
//   return {
//     primaryTerrain: TileTerrainBorderTag.GRASS,
//     secondaryTerrain: TileTerrainBorderTag.WATER,
//     mode: 1,
//     startTileId: 0,
//   };
// }
