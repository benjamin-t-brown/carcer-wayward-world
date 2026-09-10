import type {
  CarcerMapTemplate,
  CharacterTemplate,
  GameEvent,
  ItemTemplate,
  MapGridTemplate,
  TileMetadata,
  TilesetTemplate,
} from '../types/assets';

export interface MapDocumentIndexAssets {
  readonly maps: readonly CarcerMapTemplate[];
  readonly mapGrids: readonly MapGridTemplate[];
  readonly tilesets: readonly TilesetTemplate[];
  readonly characters: readonly CharacterTemplate[];
  readonly items: readonly ItemTemplate[];
  readonly gameEvents: readonly GameEvent[];
}

export interface IndexedMapGridPlacement {
  readonly grid: MapGridTemplate;
  readonly cellX: number;
  readonly cellY: number;
}

/** The hot-path subset consumed while rendering map tiles. */
export interface MapRenderLookups {
  readonly tilesetsByName: ReadonlyMap<string, TilesetTemplate>;
  readonly tileDefinitionsByTilesetName: ReadonlyMap<
    string,
    ReadonlyMap<number, TileMetadata>
  >;
  readonly charactersByName: ReadonlyMap<string, CharacterTemplate>;
  readonly itemsByName: ReadonlyMap<string, ItemTemplate>;
}

/**
 * Immutable lookup tables for one coherent database view.
 *
 * Duplicate identifiers retain the first document, matching the array `.find`
 * behavior this replaces. Placements deliberately retain every occurrence so
 * malformed duplicate grid assignments remain inspectable and deterministic.
 */
export interface MapDocumentIndex extends MapRenderLookups {
  readonly revision: string;
  readonly mapsByName: ReadonlyMap<string, CarcerMapTemplate>;
  readonly gridsByName: ReadonlyMap<string, MapGridTemplate>;
  readonly placementsByMapName: ReadonlyMap<
    string,
    readonly IndexedMapGridPlacement[]
  >;
  readonly eventsById: ReadonlyMap<string, GameEvent>;
}

export function buildMapRenderLookups(
  tilesets: readonly TilesetTemplate[],
  characters: readonly CharacterTemplate[],
  items: readonly ItemTemplate[],
): MapRenderLookups {
  const tilesetsByName = indexFirst(tilesets, (tileset) => tileset.name);
  const tileDefinitionsByTilesetName = new Map<
    string,
    ReadonlyMap<number, TileMetadata>
  >();
  for (const [name, tileset] of tilesetsByName) {
    tileDefinitionsByTilesetName.set(
      name,
      indexFirst(tileset.tiles, (tile) => tile.id),
    );
  }

  return {
    tilesetsByName,
    tileDefinitionsByTilesetName,
    charactersByName: indexFirst(characters, (character) => character.name),
    itemsByName: indexFirst(items, (item) => item.name),
  };
}

export function buildMapDocumentIndex(
  revision: string,
  assets: MapDocumentIndexAssets,
): MapDocumentIndex {
  const mapsByName = indexFirst(assets.maps, (map) => map.name);
  const gridsByName = indexFirst(assets.mapGrids, (grid) => grid.name);
  const renderLookups = buildMapRenderLookups(
    assets.tilesets,
    assets.characters,
    assets.items,
  );
  const eventsById = indexFirst(assets.gameEvents, (event) => event.id);

  const mutablePlacements = new Map<string, IndexedMapGridPlacement[]>();
  for (const grid of assets.mapGrids) {
    const maxY = Math.min(grid.gridHeight, grid.cells.length);
    for (let cellY = 0; cellY < maxY; cellY += 1) {
      const row = grid.cells[cellY];
      if (!row) continue;
      const maxX = Math.min(grid.gridWidth, row.length);
      for (let cellX = 0; cellX < maxX; cellX += 1) {
        const mapName = row[cellX]?.trim();
        if (!mapName) continue;
        const placements = mutablePlacements.get(mapName) ?? [];
        placements.push({ grid, cellX, cellY });
        mutablePlacements.set(mapName, placements);
      }
    }
  }

  return {
    revision,
    mapsByName,
    gridsByName,
    placementsByMapName: mutablePlacements,
    ...renderLookups,
    eventsById,
  };
}

/**
 * Retains a document index for a database revision. Collection identity is
 * also checked because local, unsaved React edits occur before the server gives
 * the session a new revision.
 */
export class MapDocumentIndexCache {
  private cachedRevision: string | undefined;
  private cachedAssets: MapDocumentIndexAssets | undefined;
  private cachedIndex: MapDocumentIndex | undefined;

  get(revision: string, assets: MapDocumentIndexAssets): MapDocumentIndex {
    if (
      this.cachedIndex &&
      this.cachedRevision === revision &&
      this.cachedAssets &&
      sameCollectionReferences(this.cachedAssets, assets)
    ) {
      return this.cachedIndex;
    }

    const index = buildMapDocumentIndex(revision, assets);
    this.cachedRevision = revision;
    this.cachedAssets = assets;
    this.cachedIndex = index;
    return index;
  }

  clear(): void {
    this.cachedRevision = undefined;
    this.cachedAssets = undefined;
    this.cachedIndex = undefined;
  }
}

function indexFirst<T, K>(
  values: readonly T[],
  getKey: (value: T) => K,
): ReadonlyMap<K, T> {
  const result = new Map<K, T>();
  for (const value of values) {
    const key = getKey(value);
    if (!result.has(key)) {
      result.set(key, value);
    }
  }
  return result;
}

function sameCollectionReferences(
  left: MapDocumentIndexAssets,
  right: MapDocumentIndexAssets,
): boolean {
  return (
    left.maps === right.maps &&
    left.mapGrids === right.mapGrids &&
    left.tilesets === right.tilesets &&
    left.characters === right.characters &&
    left.items === right.items &&
    left.gameEvents === right.gameEvents
  );
}
