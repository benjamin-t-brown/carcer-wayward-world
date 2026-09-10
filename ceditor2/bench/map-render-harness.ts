import { cpus, platform, release } from 'node:os';
import { performance } from 'node:perf_hooks';
import type { SpriteDefinition } from '../src/core/media/index.js';
import {
  ImageCache,
  MapRenderer,
  Viewport,
  type MapRenderStats,
  type RenderMapDocument,
} from '../src/apps/maps/canvas/index.js';

const SPRITE_PATH = 'bench/generated-tiles.png';
const SPRITES_PER_ROW = 8;
const SPRITE_SIZE = 32;
const SPRITE_COUNT = SPRITES_PER_ROW * SPRITES_PER_ROW;

export interface MapBenchmarkScenario {
  readonly id: string;
  readonly description: string;
  readonly mapWidth: number;
  readonly mapHeight: number;
  readonly tileWidth: number;
  readonly tileHeight: number;
  readonly canvasWidth: number;
  readonly canvasHeight: number;
  readonly viewportX: number;
  readonly viewportY: number;
  readonly viewportScale: number;
  readonly marginTiles: number;
}

export interface BenchmarkOptions {
  readonly warmupFrames?: number;
  readonly measuredFrames?: number;
}

export interface FrameTimingSummary {
  readonly averageMs: number;
  readonly minMs: number;
  readonly p50Ms: number;
  readonly p95Ms: number;
  readonly p99Ms: number;
  readonly maxMs: number;
}

export interface ScenarioBenchmarkResult {
  readonly id: string;
  readonly description: string;
  readonly map: {
    readonly width: number;
    readonly height: number;
    readonly compactBytes: number;
  };
  readonly canvas: { readonly width: number; readonly height: number };
  readonly viewport: {
    readonly x: number;
    readonly y: number;
    readonly scale: number;
    readonly marginTiles: number;
  };
  readonly warmupFrames: number;
  readonly measuredFrames: number;
  readonly renderStats: Readonly<MapRenderStats>;
  readonly canvasOperations: {
    readonly clearRects: number;
    readonly fillRects: number;
    readonly drawImages: number;
    readonly smoothingDisabled: boolean;
  };
  /** Hash of one untimed Canvas2D command stream for visual comparisons. */
  readonly visualHash: string;
  readonly timing: FrameTimingSummary;
}

export interface MapBenchmarkReport {
  readonly schemaVersion: 1;
  readonly runtime: {
    readonly node: string;
    readonly platform: string;
    readonly release: string;
    readonly architecture: string;
    readonly cpuModel: string;
    readonly logicalCpuCount: number;
  };
  readonly scenarios: readonly ScenarioBenchmarkResult[];
}

export const MAP_BENCHMARK_SCENARIOS: readonly MapBenchmarkScenario[] = [
  {
    id: 'small-full-map',
    description: '24×18 map fully visible in an 800×600 canvas',
    mapWidth: 24,
    mapHeight: 18,
    tileWidth: 28,
    tileHeight: 32,
    canvasWidth: 800,
    canvasHeight: 600,
    viewportX: 64,
    viewportY: 12,
    viewportScale: 1,
    marginTiles: 1,
  },
  {
    id: 'medium-panned',
    description: '128×128 map panned and zoomed to a representative work area',
    mapWidth: 128,
    mapHeight: 128,
    tileWidth: 28,
    tileHeight: 32,
    canvasWidth: 1280,
    canvasHeight: 720,
    viewportX: -900,
    viewportY: -1100,
    viewportScale: 1.25,
    marginTiles: 1,
  },
  {
    id: 'large-zoomed-out',
    description: '512×512 map zoomed out on a 1920×1080 canvas',
    mapWidth: 512,
    mapHeight: 512,
    tileWidth: 28,
    tileHeight: 32,
    canvasWidth: 1920,
    canvasHeight: 1080,
    viewportX: -3000,
    viewportY: -3500,
    viewportScale: 0.5,
    marginTiles: 1,
  },
] as const;

class CompactBenchmarkMap implements RenderMapDocument {
  readonly graphics: Uint16Array;
  private readonly sprites: readonly SpriteDefinition[];

  constructor(
    readonly width: number,
    readonly height: number,
    readonly tileWidth: number,
    readonly tileHeight: number,
  ) {
    this.graphics = generateCompactGraphics(width, height);
    this.sprites = Array.from({ length: SPRITE_COUNT }, (_, index) => ({
      name: `benchmark_tile_${index}`,
      pictureAlias: 'benchmark_tiles',
      picturePath: SPRITE_PATH,
      index,
      width: SPRITE_SIZE,
      height: SPRITE_SIZE,
    }));
  }

  spriteAt(
    _layer: number,
    tileIndex: number,
  ): SpriteDefinition | null | undefined {
    const pairIndex = tileIndex * 2;
    const tilesetIndex = this.graphics[pairIndex];
    const spriteIndex = this.graphics[pairIndex + 1];
    if (tilesetIndex === 0) {
      return null;
    }
    if (tilesetIndex !== 1 || spriteIndex === undefined) {
      return undefined;
    }
    return this.sprites[spriteIndex];
  }
}

/** Generate dense maps.json-style graphic pairs with a stable content mix. */
export function generateCompactGraphics(
  width: number,
  height: number,
): Uint16Array {
  if (!Number.isSafeInteger(width) || width < 1) {
    throw new RangeError('Map width must be a positive integer');
  }
  if (!Number.isSafeInteger(height) || height < 1) {
    throw new RangeError('Map height must be a positive integer');
  }
  const graphics = new Uint16Array(width * height * 2);
  for (let index = 0; index < width * height; index += 1) {
    const pairIndex = index * 2;
    if (index % 17 === 0) {
      continue;
    }
    graphics[pairIndex] = index % 53 === 0 ? 2 : 1;
    graphics[pairIndex + 1] = index % SPRITE_COUNT;
  }
  return graphics;
}

class FakeCanvasContext {
  readonly canvas: { width: number; height: number };
  imageSmoothingEnabled = true;
  globalAlpha = 1;
  fillStyle: string | CanvasGradient | CanvasPattern = '#000';
  clearRectCount = 0;
  fillRectCount = 0;
  drawImageCount = 0;

  private hash = 0x811c9dc5;
  private captureVisual = false;
  private readonly savedAlpha: number[] = [];

  constructor(width: number, height: number) {
    this.canvas = { width, height };
  }

  reset(captureVisual: boolean): void {
    this.clearRectCount = 0;
    this.fillRectCount = 0;
    this.drawImageCount = 0;
    this.hash = 0x811c9dc5;
    this.captureVisual = captureVisual;
    this.savedAlpha.length = 0;
    this.globalAlpha = 1;
  }

  clearRect(x: number, y: number, width: number, height: number): void {
    this.clearRectCount += 1;
    this.trace('clear', x, y, width, height);
  }

  fillRect(x: number, y: number, width: number, height: number): void {
    this.fillRectCount += 1;
    this.trace('fill', String(this.fillStyle), x, y, width, height);
  }

  save(): void {
    this.savedAlpha.push(this.globalAlpha);
    this.trace('save');
  }

  restore(): void {
    this.globalAlpha = this.savedAlpha.pop() ?? 1;
    this.trace('restore');
  }

  drawImage(source: CanvasImageSource, ...numbers: number[]): void {
    this.drawImageCount += 1;
    const benchmarkPath = (
      source as unknown as { readonly benchmarkPath?: string }
    ).benchmarkPath;
    this.trace(
      'image',
      benchmarkPath ?? 'source',
      this.globalAlpha,
      ...numbers,
    );
  }

  visualHash(): string {
    return this.hash.toString(16).padStart(8, '0');
  }

  private trace(...parts: readonly (number | string)[]): void {
    if (!this.captureVisual) {
      return;
    }
    const value = parts.join('|');
    for (let index = 0; index < value.length; index += 1) {
      this.hash ^= value.charCodeAt(index);
      this.hash = Math.imul(this.hash, 0x01000193) >>> 0;
    }
    this.hash ^= 10;
    this.hash = Math.imul(this.hash, 0x01000193) >>> 0;
  }
}

function renderFrame(
  renderer: MapRenderer,
  context: FakeCanvasContext,
  document: RenderMapDocument,
  viewport: Viewport,
  scenario: MapBenchmarkScenario,
): void {
  const canvasContext = context as unknown as CanvasRenderingContext2D;
  renderer.beginFrame(
    canvasContext,
    scenario.canvasWidth,
    scenario.canvasHeight,
  );
  renderer.drawMap(canvasContext, {
    document,
    layer: 0,
    viewport,
    marginTiles: scenario.marginTiles,
  });
}

function copyStats(stats: MapRenderStats): MapRenderStats {
  return {
    mapBlocks: stats.mapBlocks,
    visitedTiles: stats.visitedTiles,
    drawnTiles: stats.drawnTiles,
    blankTiles: stats.blankTiles,
    unresolvedSprites: stats.unresolvedSprites,
    pendingImages: stats.pendingImages,
  };
}

function sameStats(left: MapRenderStats, right: MapRenderStats): boolean {
  return (
    left.mapBlocks === right.mapBlocks &&
    left.visitedTiles === right.visitedTiles &&
    left.drawnTiles === right.drawnTiles &&
    left.blankTiles === right.blankTiles &&
    left.unresolvedSprites === right.unresolvedSprites &&
    left.pendingImages === right.pendingImages
  );
}

function positiveFrameCount(
  value: number | undefined,
  fallback: number,
): number {
  const result = value ?? fallback;
  if (!Number.isSafeInteger(result) || result < 0) {
    throw new RangeError('Frame counts must be non-negative integers');
  }
  return result;
}

function percentile(sortedValues: readonly number[], quantile: number): number {
  if (sortedValues.length === 0) {
    return 0;
  }
  const index = Math.ceil(quantile * sortedValues.length) - 1;
  return sortedValues[Math.max(0, index)]!;
}

function rounded(value: number): number {
  return Number(value.toFixed(4));
}

export function summarizeFrameTimings(
  frameTimes: readonly number[],
): FrameTimingSummary {
  if (frameTimes.length === 0) {
    return {
      averageMs: 0,
      minMs: 0,
      p50Ms: 0,
      p95Ms: 0,
      p99Ms: 0,
      maxMs: 0,
    };
  }
  const sorted = [...frameTimes].sort((left, right) => left - right);
  const total = frameTimes.reduce((sum, time) => sum + time, 0);
  return {
    averageMs: rounded(total / frameTimes.length),
    minMs: rounded(sorted[0]!),
    p50Ms: rounded(percentile(sorted, 0.5)),
    p95Ms: rounded(percentile(sorted, 0.95)),
    p99Ms: rounded(percentile(sorted, 0.99)),
    maxMs: rounded(sorted.at(-1)!),
  };
}

async function loadedImageCache(): Promise<ImageCache> {
  const cache = new ImageCache(async (path) => ({
    source: { benchmarkPath: path } as unknown as CanvasImageSource,
    width: SPRITES_PER_ROW * SPRITE_SIZE,
    height: SPRITES_PER_ROW * SPRITE_SIZE,
  }));
  await cache.load(SPRITE_PATH);
  return cache;
}

export async function benchmarkMapScenario(
  scenario: MapBenchmarkScenario,
  options: BenchmarkOptions = {},
): Promise<ScenarioBenchmarkResult> {
  const warmupFrames = positiveFrameCount(options.warmupFrames, 100);
  const measuredFrames = positiveFrameCount(options.measuredFrames, 500);
  if (measuredFrames === 0) {
    throw new RangeError('At least one measured frame is required');
  }

  const document = new CompactBenchmarkMap(
    scenario.mapWidth,
    scenario.mapHeight,
    scenario.tileWidth,
    scenario.tileHeight,
  );
  const viewport = new Viewport({
    x: scenario.viewportX,
    y: scenario.viewportY,
    scale: scenario.viewportScale,
  });
  const renderer = new MapRenderer(await loadedImageCache());
  const context = new FakeCanvasContext(
    scenario.canvasWidth,
    scenario.canvasHeight,
  );

  // Visual signature and deterministic operation counts are intentionally
  // collected outside the timed loop.
  context.reset(true);
  renderFrame(renderer, context, document, viewport, scenario);
  const expectedStats = copyStats(renderer.stats);
  const visualHash = context.visualHash();
  const canvasOperations = {
    clearRects: context.clearRectCount,
    fillRects: context.fillRectCount,
    drawImages: context.drawImageCount,
    smoothingDisabled: context.imageSmoothingEnabled === false,
  };

  context.reset(false);
  for (let frame = 0; frame < warmupFrames; frame += 1) {
    renderFrame(renderer, context, document, viewport, scenario);
  }

  const frameTimes: number[] = [];
  for (let frame = 0; frame < measuredFrames; frame += 1) {
    context.reset(false);
    const start = performance.now();
    renderFrame(renderer, context, document, viewport, scenario);
    frameTimes.push(performance.now() - start);
    if (
      !sameStats(renderer.stats, expectedStats) ||
      context.drawImageCount !== canvasOperations.drawImages
    ) {
      throw new Error(`Non-deterministic render counts in ${scenario.id}`);
    }
  }

  return {
    id: scenario.id,
    description: scenario.description,
    map: {
      width: scenario.mapWidth,
      height: scenario.mapHeight,
      compactBytes: document.graphics.byteLength,
    },
    canvas: { width: scenario.canvasWidth, height: scenario.canvasHeight },
    viewport: {
      x: scenario.viewportX,
      y: scenario.viewportY,
      scale: scenario.viewportScale,
      marginTiles: scenario.marginTiles,
    },
    warmupFrames,
    measuredFrames,
    renderStats: expectedStats,
    canvasOperations,
    visualHash,
    timing: summarizeFrameTimings(frameTimes),
  };
}

export async function runMapBenchmark(
  options: BenchmarkOptions = {},
): Promise<MapBenchmarkReport> {
  const results: ScenarioBenchmarkResult[] = [];
  for (const scenario of MAP_BENCHMARK_SCENARIOS) {
    results.push(await benchmarkMapScenario(scenario, options));
  }
  const cpuList = cpus();
  return {
    schemaVersion: 1,
    runtime: {
      node: globalThis.process.version,
      platform: platform(),
      release: release(),
      architecture: globalThis.process.arch,
      cpuModel: cpuList[0]?.model ?? 'unknown',
      logicalCpuCount: cpuList.length,
    },
    scenarios: results,
  };
}

export function formatMapBenchmark(report: MapBenchmarkReport): string {
  const lines = [
    'CEditor2 map-render benchmark',
    `${report.runtime.node} | ${report.runtime.platform} ${report.runtime.release} | ${report.runtime.architecture}`,
    `${report.runtime.cpuModel} (${report.runtime.logicalCpuCount} logical CPUs)`,
    '',
  ];
  for (const scenario of report.scenarios) {
    const stats = scenario.renderStats;
    const timing = scenario.timing;
    lines.push(
      `${scenario.id}: ${scenario.description}`,
      `  map ${scenario.map.width}×${scenario.map.height} (${scenario.map.compactBytes} compact bytes), canvas ${scenario.canvas.width}×${scenario.canvas.height}`,
      `  ${scenario.measuredFrames} frames after ${scenario.warmupFrames} warmup | visited ${stats.visitedTiles}, drawn ${stats.drawnTiles}, blank ${stats.blankTiles}, unresolved ${stats.unresolvedSprites}`,
      `  avg ${timing.averageMs.toFixed(4)} ms | p50 ${timing.p50Ms.toFixed(4)} | p95 ${timing.p95Ms.toFixed(4)} | p99 ${timing.p99Ms.toFixed(4)} | max ${timing.maxMs.toFixed(4)}`,
      `  visual ${scenario.visualHash} | drawImage calls ${scenario.canvasOperations.drawImages}`,
      '',
    );
  }
  return lines.join('\n');
}
