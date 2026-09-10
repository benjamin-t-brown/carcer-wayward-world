import { parseMapRecord } from './mapParser.js';
import type {
  MapCellGraphic,
  MapCellPatch,
  MapPlacements,
  MapRecord,
} from './types.js';

const DEFAULT_SPRITE_WIDTH = 28;
const DEFAULT_SPRITE_HEIGHT = 32;

/** Mutable dense-cell document owned by one map editor controller. */
export class MapDocument {
  readonly #record: MapRecord;
  readonly #layers: readonly number[];
  readonly #tilesetNames: readonly string[];
  readonly #placements: MapPlacements;

  private constructor(record: MapRecord) {
    this.#record = record;
    this.#layers = Object.freeze([...record.layers]);
    this.#tilesetNames = Object.freeze([...record.tilesets]);
    this.#placements = Object.freeze({
      characters: record.characters,
      items: record.items,
      markers: record.markers,
      eventTriggers: record.eventTriggers,
      travelTriggers: record.travelTriggers,
      tileOverrides: record.tileOverrides,
      lightSources: record.lightSources,
    });
  }

  static from(value: unknown, path = 'map'): MapDocument {
    return new MapDocument(parseMapRecord(value, path));
  }

  get name(): string {
    return this.#record.name;
  }

  get width(): number {
    return this.#record.width;
  }

  get height(): number {
    return this.#record.height;
  }

  get spriteWidth(): number {
    return this.#record.spriteWidth ?? DEFAULT_SPRITE_WIDTH;
  }

  get spriteHeight(): number {
    return this.#record.spriteHeight ?? DEFAULT_SPRITE_HEIGHT;
  }

  get cellCount(): number {
    return this.width * this.height;
  }

  get layers(): readonly number[] {
    return this.#layers;
  }

  get tilesetNames(): readonly string[] {
    return this.#tilesetNames;
  }

  /** Raw sparse arrays; no materialized per-cell metadata is created. */
  get placements(): MapPlacements {
    return this.#placements;
  }

  hasLayer(layer: number): boolean {
    return (
      Number.isSafeInteger(layer) &&
      Object.hasOwn(this.#record.tiles, String(layer))
    );
  }

  /** Read-only hot-path access for allocation-free canvas traversal. */
  layerData(layer: number): readonly number[] | undefined {
    if (!this.hasLayer(layer)) {
      return undefined;
    }
    return this.#record.tiles[String(layer)];
  }

  indexAt(x: number, y: number): number | undefined {
    if (
      !Number.isSafeInteger(x) ||
      !Number.isSafeInteger(y) ||
      x < 0 ||
      x >= this.width ||
      y < 0 ||
      y >= this.height
    ) {
      return undefined;
    }
    return y * this.width + x;
  }

  coordinatesOf(index: number): { x: number; y: number } | undefined {
    if (!this.#isBoundedIndex(index)) {
      return undefined;
    }
    return {
      x: index % this.width,
      y: Math.floor(index / this.width),
    };
  }

  readCell(layer: number, index: number): MapCellGraphic | undefined {
    const graphics = this.layerData(layer);
    if (!graphics || !this.#isBoundedIndex(index)) {
      return undefined;
    }
    const offset = index * 2;
    return {
      tilesetIndex: graphics[offset]!,
      tileIndex: graphics[offset + 1]!,
    };
  }

  readCellAt(layer: number, x: number, y: number): MapCellGraphic | undefined {
    const index = this.indexAt(x, y);
    return index === undefined ? undefined : this.readCell(layer, index);
  }

  writeCell(layer: number, index: number, graphic: MapCellGraphic): boolean {
    const graphics = this.#writableLayer(layer);
    if (
      !graphics ||
      !this.#isBoundedIndex(index) ||
      !Number.isSafeInteger(graphic.tilesetIndex) ||
      !Number.isSafeInteger(graphic.tileIndex)
    ) {
      return false;
    }
    const offset = index * 2;
    graphics[offset] = graphic.tilesetIndex;
    graphics[offset + 1] = graphic.tileIndex;
    return true;
  }

  writeCellAt(
    layer: number,
    x: number,
    y: number,
    graphic: MapCellGraphic,
  ): boolean {
    const index = this.indexAt(x, y);
    return index === undefined ? false : this.writeCell(layer, index, graphic);
  }

  applyCellPatch(patch: MapCellPatch): boolean {
    return this.writeCell(patch.layer, patch.index, patch);
  }

  /** Applies valid compact patches in order and reports how many were accepted. */
  applyCellPatches(patches: Iterable<MapCellPatch>): number {
    let applied = 0;
    for (const patch of patches) {
      if (this.applyCellPatch(patch)) {
        applied += 1;
      }
    }
    return applied;
  }

  snapshot(): MapRecord {
    return structuredClone(this.#record);
  }

  #isBoundedIndex(index: number): boolean {
    return Number.isSafeInteger(index) && index >= 0 && index < this.cellCount;
  }

  #writableLayer(layer: number): number[] | undefined {
    return this.hasLayer(layer) ? this.#record.tiles[String(layer)] : undefined;
  }
}
