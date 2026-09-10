import type { JsonArray, JsonObject } from '../../core/database/index.js';
import type { MapDocument } from '../../core/domain/maps/index.js';
import type { MediaCatalog, SpriteDefinition } from '../../core/media/index.js';
import type { RenderMapDocument } from './canvas/index.js';

export interface TilesetTile {
  id: number;
  description: string;
}

export interface TilesetInfo {
  name: string;
  spriteBase: string;
  tiles: TilesetTile[];
}

/** Bridges lossless map storage to pre-indexed renderer sprite lookups. */
export class MapRenderDocument implements RenderMapDocument {
  private spriteByTileset: readonly (
    ReadonlyMap<number, SpriteDefinition> | undefined
  )[] = [];
  private cachedLayer?: number;
  private cachedLayerData?: readonly number[];

  constructor(readonly source: MapDocument) {}

  get width(): number {
    return this.source.width;
  }

  get height(): number {
    return this.source.height;
  }

  get tileWidth(): number {
    return this.source.spriteWidth;
  }

  get tileHeight(): number {
    return this.source.spriteHeight;
  }

  rebuildSprites(
    catalog: MediaCatalog,
    tilesets: ReadonlyMap<string, TilesetInfo>,
  ): void {
    this.spriteByTileset = this.source.tilesetNames.map((name) => {
      const tileset = tilesets.get(name);
      if (!tileset) return undefined;
      const sprites = new Map<number, SpriteDefinition>();
      for (const tile of tileset.tiles) {
        const sprite = catalog.spriteByName.get(
          `${tileset.spriteBase}_${tile.id}`,
        );
        if (sprite) sprites.set(tile.id, sprite);
      }
      return sprites;
    });
  }

  spriteAt(
    layer: number,
    tileIndex: number,
  ): SpriteDefinition | null | undefined {
    if (layer !== this.cachedLayer) {
      this.cachedLayer = layer;
      this.cachedLayerData = this.source.layerData(layer);
    }
    const graphics = this.cachedLayerData;
    if (!graphics) return null;
    if (tileIndex < 0 || tileIndex >= this.source.cellCount) return undefined;
    const pair = tileIndex * 2;
    const tilesetIndex = graphics[pair]!;
    const spriteIndex = graphics[pair + 1]!;
    if (tilesetIndex === 0 && spriteIndex === 0) return null;
    return this.spriteByTileset[tilesetIndex]?.get(spriteIndex);
  }
}

export function parseTilesets(values: JsonArray): Map<string, TilesetInfo> {
  const result = new Map<string, TilesetInfo>();
  for (const value of values) {
    if (
      !isObject(value) ||
      typeof value.name !== 'string' ||
      typeof value.spriteBase !== 'string'
    )
      continue;
    const tiles: TilesetTile[] = [];
    if (Array.isArray(value.tiles))
      for (const tile of value.tiles)
        if (isObject(tile) && Number.isSafeInteger(tile.id))
          tiles.push({
            id: tile.id as number,
            description:
              typeof tile.description === 'string' ? tile.description : '',
          });
    result.set(value.name, {
      name: value.name,
      spriteBase: value.spriteBase,
      tiles,
    });
  }
  return result;
}

function isObject(value: unknown): value is JsonObject {
  return typeof value === 'object' && value !== null && !Array.isArray(value);
}
