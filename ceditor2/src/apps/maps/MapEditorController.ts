import type { JsonArray } from '../../core/database/index.js';
import { MapGridTopology } from '../../core/domain/mapGrids/index.js';
import {
  addMapLayer,
  cloneMapInCollection,
  createMapInCollection,
  deleteMapAcrossDatabase,
  deleteMapLayer,
  MapDocument,
  renameMapAcrossDatabase,
  resizeMapRecord,
  replaceTilePlacements,
  tilePlacementsAt,
  updateMapMetadata,
  type MapTilePlacementBundle,
  type MapRecord,
} from '../../core/domain/maps/index.js';
import {
  parseTilesetCollection,
  type TerrainBorderTag,
} from '../../core/domain/tilesets/index.js';
import { loadMediaCatalog, type MediaCatalog } from '../../core/media/index.js';
import { element } from '../../core/ui/dom.js';
import type { DatabasePageContext } from '../../core/ui/index.js';
import {
  MapRenderer,
  MapOverlayRenderer,
  Viewport,
  clientToLogicalCanvasPoint,
  configureCanvasBackingStore,
  createCanvasMetrics,
  normalizeWheelDelta,
  wheelZoomFactor,
  writeCanvasMetrics,
  type RenderMapBlock,
  type TileBounds,
} from './canvas/index.js';
import { BoundedHistory } from './history/BoundedHistory.js';
import type {
  GraphicCell,
  GraphicCellAccess,
  TileGraphic,
} from './history/cellPatches.js';
import {
  createEraseGesture,
  createPencilGesture,
  type PaintGesture,
} from './tools/paintGesture.js';
import { translateTileGraphic } from './tools/tileGraphic.js';
import {
  floodFill,
  paintRectangle,
  type WorldCellResolver,
  type WorldTilePoint,
} from './tools/graphicRegionTools.js';
import {
  buildTerrainMetadataLookup,
  terrainTagLabel,
  TerrainPaintStroke,
  type TerrainMetadataLookup,
} from './tools/terrainTool.js';
import {
  MapRenderDocument,
  parseTilesets,
  type TilesetInfo,
} from './MapRenderDocument.js';
import {
  createMapScene,
  hitTestMapScene,
  isMapSceneBlockEditable,
  writeMapScene,
  type MapSceneBlock,
  type MapSceneHit,
} from './MapScene.js';
import { MapWorkspace } from './MapWorkspace.js';

type MapTool = 'select' | 'pencil' | 'erase' | 'fill' | 'rectangle' | 'terrain';

class MapGraphicAccess implements GraphicCellAccess {
  constructor(private readonly documents: ReadonlyMap<string, MapDocument>) {}
  getGraphic(cell: GraphicCell): TileGraphic {
    const value = this.documents
      .get(cell.documentId)
      ?.readCell(cell.layer, cell.index);
    if (!value)
      throw new RangeError(
        `Invalid map cell ${cell.documentId}/${cell.layer}/${cell.index}`,
      );
    return [value.tilesetIndex, value.tileIndex];
  }
  setGraphic(cell: GraphicCell, graphic: TileGraphic): void {
    if (
      !this.documents.get(cell.documentId)?.writeCell(cell.layer, cell.index, {
        tilesetIndex: graphic[0],
        tileIndex: graphic[1],
      })
    ) {
      throw new RangeError(
        `Invalid map cell ${cell.documentId}/${cell.layer}/${cell.index}`,
      );
    }
  }
}

export class MapEditorController {
  private readonly documents: MapDocument[];
  private readonly documentsByName: Map<string, MapDocument>;
  private readonly access: MapGraphicAccess;
  private readonly history = new BoundedHistory<GraphicCellAccess>();
  private readonly tilesets: Map<string, TilesetInfo>;
  private readonly gridTopologies: MapGridTopology[];
  private readonly viewports = new Map<string, Viewport>();
  private readonly renderer = new MapRenderer();
  private readonly overlayRenderer = new MapOverlayRenderer();
  private readonly renderDocuments = new Map<string, MapRenderDocument>();
  private readonly renderBlocks: RenderMapBlock[] = [];
  private readonly canvasMetrics = createCanvasMetrics();
  private readonly pointerPoint = { x: 0, y: 0 };
  private readonly canvas = element('canvas', {
    className: 'map-canvas',
    attributes: { tabindex: '0', 'aria-label': 'Map canvas' },
  });
  private readonly mapSelect = element('select');
  private readonly layerSelect = element('select');
  private readonly tilesetSelect = element('select');
  private readonly tileSelect = element('select');
  private readonly terrainSelect = element('select');
  private readonly gridToggle = element('input');
  private readonly toolButtons = new Map<MapTool, HTMLButtonElement>();
  private readonly undoButton = this.makeButton('Undo', () => this.undo());
  private readonly redoButton = this.makeButton('Redo', () => this.redo());
  private readonly selectionStatus = element('p', {
    className: 'map-selection-status',
  });
  private readonly renderStatus = element('p', {
    className: 'map-render-status muted',
  });
  private readonly metadataEditor = element('textarea', {
    className: 'form-control map-metadata-editor',
    attributes: { 'aria-label': 'Selected tile metadata JSON' },
  });
  private readonly metadataStatus = element('p', { className: 'field__help' });
  private readonly visibleBounds: TileBounds = {
    minX: 0,
    maxX: -1,
    minY: 0,
    maxY: -1,
  };
  private currentIndex: number;
  private currentLayer = 0;
  private selectedHit?: MapSceneHit;
  private hoveredHit?: MapSceneHit;
  private tool: MapTool = 'select';
  private readonly scene = createMapScene();
  private mapWorkspace?: MapWorkspace;
  private media?: MediaCatalog;
  private frameRequest?: number;
  private lastStatusUpdate = 0;
  private centerPending = true;
  private gesture?: PaintGesture;
  private rectangleStart?: WorldTilePoint;
  private terrainStroke?: TerrainPaintStroke;
  private terrainIssueCount = 0;
  private metadataSelectionKey = '';
  private panPointerId?: number;
  private lastPointerX = 0;
  private lastPointerY = 0;
  private destroyed = false;
  private showOverlayLabels = false;
  private readonly terrainLookup?: TerrainMetadataLookup;

  constructor(
    private readonly root: HTMLElement,
    private readonly context: DatabasePageContext,
    initialUrl = new URL(window.location.href),
  ) {
    this.documents = context.session
      .collection('maps')
      .map((record, index) => MapDocument.from(record, `maps[${index}]`));
    this.documentsByName = new Map(
      this.documents.map((document) => [document.name, document]),
    );
    this.access = new MapGraphicAccess(this.documentsByName);
    this.tilesets = parseTilesets(context.session.collection('tilesets'));
    try {
      this.terrainLookup = buildTerrainMetadataLookup(
        parseTilesetCollection(context.session.collection('tilesets')),
      );
    } catch {
      this.terrainLookup = undefined;
    }
    this.gridTopologies = context.session
      .collection('mapGrids')
      .map((value, index) => MapGridTopology.from(value, `mapGrids[${index}]`));
    this.rebuildRenderDocuments();
    const requested = initialUrl.searchParams.get('map');
    this.currentIndex = requested
      ? this.documents.findIndex((document) => document.name === requested)
      : this.documents.length
        ? 0
        : -1;
  }

  mount(): void {
    this.root.replaceChildren(this.buildLayout());
    this.bindEvents();
    this.context.setBeforeSaveHandler(() => this.finishGesture());
    this.populateMapSelect();
    this.selectMap(this.currentIndex);
    this.setTool('select');
    this.updateHistoryControls();
    this.frameRequest = requestAnimationFrame(this.renderFrame);
    void loadMediaCatalog().then(
      (catalog) => {
        if (this.destroyed) return;
        this.media = catalog;
        this.rebuildRenderDocuments();
        this.rebuildScene();
        this.renderStatus.textContent = 'Sprite sheets ready.';
      },
      (error: unknown) => {
        if (this.destroyed) return;
        this.renderStatus.textContent = `Could not load sprite definitions: ${error instanceof Error ? error.message : 'unknown error'}`;
        this.renderStatus.classList.remove('muted');
      },
    );
  }

  destroy(): void {
    if (this.destroyed) return;
    this.finishGesture();
    this.destroyed = true;
    this.context.setBeforeSaveHandler(undefined);
    if (this.frameRequest !== undefined)
      cancelAnimationFrame(this.frameRequest);
    window.removeEventListener('keydown', this.handleKeyDown);
    window.removeEventListener('keyup', this.handleKeyUp);
    this.root.replaceChildren();
  }

  private buildLayout(): HTMLElement {
    const toolbar = element('section', {
      className: 'map-toolbar',
      attributes: { 'aria-label': 'Map controls' },
    });
    this.gridToggle.type = 'checkbox';
    this.gridToggle.checked = true;
    toolbar.append(
      this.makeField('Map', this.mapSelect),
      this.makeField('Layer', this.layerSelect),
      this.makeField('Tileset', this.tilesetSelect),
      this.makeField('Tile', this.tileSelect),
      this.makeField('Terrain', this.terrainSelect),
    );
    const toolGroup = element('div', { className: 'map-toolbar__group' });
    for (const [tool, label] of [
      ['select', 'Select'],
      ['pencil', 'Pencil'],
      ['erase', 'Erase'],
      ['fill', 'Fill'],
      ['rectangle', 'Rectangle'],
      ['terrain', 'Terrain'],
    ] as const) {
      const control = this.makeButton(label, () => this.setTool(tool));
      control.dataset.tool = tool;
      this.toolButtons.set(tool, control);
      toolGroup.append(control);
    }
    const historyGroup = element('div', { className: 'map-toolbar__group' });
    historyGroup.append(
      this.undoButton,
      this.redoButton,
      this.makeButton('Center', () => {
        this.centerPending = true;
      }),
    );
    const lifecycleGroup = element('div', {
      className: 'map-toolbar__group',
    });
    lifecycleGroup.append(
      this.makeButton('+ Map', () => this.createMap()),
      this.makeButton('Clone Map', () => this.cloneMap()),
      this.makeButton('Properties', () => this.editMapProperties()),
      this.makeButton('Delete Map', () => this.deleteMap()),
      this.makeButton('+ Layer', () => this.addLayer()),
      this.makeButton('− Layer', () => this.deleteLayer()),
    );
    const gridLabel = element('label', { className: 'map-grid-toggle' });
    gridLabel.append(this.gridToggle, document.createTextNode(' Tile grid'));
    toolbar.append(toolGroup, historyGroup, lifecycleGroup, gridLabel);
    this.populateTerrainSelect();
    const canvasWrap = element('div', { className: 'map-canvas-wrap' });
    canvasWrap.append(this.canvas);
    const sidebar = element('aside', { className: 'map-inspector surface' });
    this.metadataEditor.rows = 14;
    this.metadataEditor.spellcheck = false;
    const metadataDetails = element('details');
    metadataDetails.append(
      element('summary', { text: 'Tile metadata' }),
      element('p', {
        className: 'field__help',
        text: 'Edit sparse character, item, marker, event, travel, override, and light-source arrays. Unknown fields are preserved.',
      }),
      this.metadataEditor,
      this.makeButton('Apply Metadata', () => this.applyTileMetadata()),
      this.metadataStatus,
    );
    sidebar.append(
      element('h2', { text: 'Current tile' }),
      this.selectionStatus,
      element('p', {
        className: 'muted',
        text: 'Left-drag paints seamlessly across visible grid partitions. Middle-drag pans. Wheel zooms around the pointer.',
      }),
      metadataDetails,
      this.renderStatus,
    );
    const workspace = element('div', { className: 'map-workspace' });
    workspace.append(canvasWrap, sidebar);
    const editor = element('div', { className: 'map-editor' });
    editor.append(toolbar, workspace);
    return editor;
  }

  private bindEvents(): void {
    this.mapSelect.addEventListener('change', () =>
      this.selectMap(this.mapSelect.selectedIndex),
    );
    this.layerSelect.addEventListener('change', () => {
      this.finishGesture();
      this.currentLayer = Number(this.layerSelect.value);
      this.selectedHit = undefined;
      this.hoveredHit = undefined;
      this.rebuildRenderBlocks();
      this.updateSelectionStatus();
    });
    this.tilesetSelect.addEventListener('change', () =>
      this.populateTileSelect(),
    );
    this.canvas.addEventListener('pointerdown', this.handlePointerDown);
    this.canvas.addEventListener('pointermove', this.handlePointerMove);
    this.canvas.addEventListener('pointerup', this.handlePointerUp);
    this.canvas.addEventListener('pointercancel', this.handlePointerCancel);
    this.canvas.addEventListener('wheel', this.handleWheel, { passive: false });
    this.canvas.addEventListener('contextmenu', (event) =>
      event.preventDefault(),
    );
    window.addEventListener('keydown', this.handleKeyDown);
    window.addEventListener('keyup', this.handleKeyUp);
  }

  private populateMapSelect(): void {
    this.mapSelect.replaceChildren();
    for (const map of this.documents) {
      const option = element('option', { text: map.name });
      option.value = map.name;
      this.mapSelect.append(option);
    }
  }

  private selectMap(index: number): void {
    this.finishGesture();
    this.currentIndex =
      index >= 0 && index < this.documents.length ? index : -1;
    const current = this.currentDocument();
    if (!current) {
      this.mapSelect.selectedIndex = -1;
      this.scene.blocks.length = 0;
      this.scene.gridName = undefined;
      this.mapWorkspace = undefined;
      this.renderBlocks.length = 0;
      this.updateSelectionStatus();
      return;
    }
    this.mapSelect.value = current.name;
    this.currentLayer = current.hasLayer(0) ? 0 : (current.layers[0] ?? 0);
    this.selectedHit = undefined;
    this.hoveredHit = undefined;
    this.mapWorkspace = new MapWorkspace(
      current,
      this.documentsByName,
      this.gridTopologies,
    );
    this.populateLayerSelect(current);
    this.populateTilesetSelect(current);
    this.rebuildScene();
    if (!this.viewports.has(current.name)) {
      this.viewports.set(current.name, new Viewport());
      this.centerPending = true;
    }
    const url = new URL(window.location.href);
    url.searchParams.set('map', current.name);
    window.history.replaceState(null, '', url);
    this.updateSelectionStatus();
  }

  private populateLayerSelect(current: MapDocument): void {
    this.layerSelect.replaceChildren();
    for (const layer of [...current.layers].sort((a, b) => b - a)) {
      const option = element('option', { text: String(layer) });
      option.value = String(layer);
      this.layerSelect.append(option);
    }
    this.layerSelect.value = String(this.currentLayer);
  }

  private populateTilesetSelect(current: MapDocument): void {
    this.tilesetSelect.replaceChildren();
    current.tilesetNames.forEach((name, index) => {
      if (name) {
        const option = element('option', { text: name });
        option.value = String(index);
        this.tilesetSelect.append(option);
      }
    });
    this.tilesetSelect.selectedIndex = this.tilesetSelect.options.length
      ? 0
      : -1;
    this.populateTileSelect();
  }

  private populateTileSelect(): void {
    this.tileSelect.replaceChildren();
    const current = this.currentDocument();
    if (!current) return;
    const tileset = this.tilesets.get(
      current.tilesetNames[Number(this.tilesetSelect.value)] ?? '',
    );
    for (const tile of tileset?.tiles ?? []) {
      const option = element('option', {
        text: `${tile.id}: ${tile.description || 'tile'}`,
      });
      option.value = String(tile.id);
      this.tileSelect.append(option);
    }
    this.tileSelect.selectedIndex = this.tileSelect.options.length ? 0 : -1;
  }

  private populateTerrainSelect(): void {
    this.terrainSelect.replaceChildren();
    const tags = this.terrainLookup?.paintableTags ?? [];
    for (const tag of tags) {
      const option = element('option', { text: terrainTagLabel(tag) });
      option.value = tag;
      this.terrainSelect.append(option);
    }
    if (!tags.length) {
      const option = element('option', { text: 'Unavailable' });
      option.value = '';
      this.terrainSelect.append(option);
      this.terrainSelect.disabled = true;
    }
  }

  private rebuildRenderDocuments(): void {
    this.renderDocuments.clear();
    for (const document of this.documents) {
      const adapter = new MapRenderDocument(document);
      if (this.media) adapter.rebuildSprites(this.media, this.tilesets);
      this.renderDocuments.set(document.name, adapter);
    }
  }

  private rebuildScene(): void {
    const current = this.currentDocument();
    if (!current) {
      this.scene.blocks.length = 0;
      this.scene.gridName = undefined;
      this.renderBlocks.length = 0;
      return;
    }
    writeMapScene(
      this.scene,
      current,
      this.documentsByName,
      this.gridTopologies,
      {
        worldBounds: {
          left: 0,
          top: 0,
          right: current.width * current.spriteWidth,
          bottom: current.height * current.spriteHeight,
        },
      },
    );
    this.rebuildRenderBlocks();
  }

  private writeVisibleScene(
    viewport: Viewport,
    width: number,
    height: number,
  ): void {
    const current = this.currentDocument();
    if (!current) return;
    writeMapScene(
      this.scene,
      current,
      this.documentsByName,
      this.gridTopologies,
      {
        worldBounds: {
          left: viewport.screenToWorldX(0),
          top: viewport.screenToWorldY(0),
          right: viewport.screenToWorldX(width),
          bottom: viewport.screenToWorldY(height),
        },
      },
    );
    this.rebuildRenderBlocks();
  }

  private rebuildRenderBlocks(): void {
    this.renderBlocks.length = 0;
    for (const block of this.scene.blocks) {
      const document = this.renderDocuments.get(block.document.name);
      if (!document) continue;
      this.renderBlocks.push({
        document,
        layer: this.currentLayer,
        originX: block.originX,
        originY: block.originY,
        opacity: block.focused
          ? 1
          : isMapSceneBlockEditable(block, this.currentLayer)
            ? 0.82
            : 0.4,
        background: '#09090b',
      });
    }
  }

  private currentDocument(): MapDocument | undefined {
    return this.documents[this.currentIndex];
  }
  private currentViewport(): Viewport | undefined {
    const name = this.currentDocument()?.name;
    return name ? this.viewports.get(name) : undefined;
  }

  private setTool(tool: MapTool): void {
    this.finishGesture();
    this.rectangleStart = undefined;
    this.tool = tool;
    for (const [value, control] of this.toolButtons) {
      control.classList.toggle('button--primary', value === tool);
      control.setAttribute('aria-pressed', String(value === tool));
    }
  }

  private selectedGraphicFor(document: MapDocument): TileGraphic | undefined {
    const current = this.currentDocument();
    const tile = Number(this.tileSelect.value);
    if (!current || !Number.isSafeInteger(tile)) return undefined;
    return translateTileGraphic(
      current,
      document,
      Number(this.tilesetSelect.value),
      tile,
    );
  }

  private hitAtCanvasPoint(x: number, y: number): MapSceneHit | undefined {
    const viewport = this.currentViewport();
    if (!viewport) return undefined;
    this.writeVisibleScene(
      viewport,
      this.canvasMetrics.logicalWidth,
      this.canvasMetrics.logicalHeight,
    );
    return hitTestMapScene(
      this.scene,
      viewport.screenToWorldX(x),
      viewport.screenToWorldY(y),
      this.currentLayer,
    );
  }

  private worldTileAtCanvasPoint(
    x: number,
    y: number,
  ): WorldTilePoint | undefined {
    const current = this.currentDocument();
    const viewport = this.currentViewport();
    if (!current || !viewport) return undefined;
    return {
      x: Math.floor(viewport.screenToWorldX(x) / current.spriteWidth),
      y: Math.floor(viewport.screenToWorldY(y) / current.spriteHeight),
    };
  }

  private readonly resolveWorldCell: WorldCellResolver = (point, layer) =>
    this.mapWorkspace?.resolve(point, layer)?.cell;

  private selectedGraphicForCell(cell: GraphicCell): TileGraphic | undefined {
    const document = this.documentsByName.get(cell.documentId);
    return document ? this.selectedGraphicFor(document) : undefined;
  }

  private graphicsMatchAcrossMaps(
    cell: GraphicCell,
    graphic: TileGraphic,
    startCell: GraphicCell,
    startGraphic: TileGraphic,
  ): boolean {
    return (
      this.graphicIdentity(cell, graphic) ===
      this.graphicIdentity(startCell, startGraphic)
    );
  }

  private graphicIdentity(cell: GraphicCell, graphic: TileGraphic): string {
    const document = this.documentsByName.get(cell.documentId);
    const tileset = document?.tilesetNames[graphic[0]];
    return `${tileset ?? `#${graphic[0]}`}\u0000${graphic[1]}`;
  }

  private performFill(point: WorldTilePoint): void {
    const command = floodFill(
      this.access,
      this.resolveWorldCell,
      this.currentLayer,
      point,
      (cell) => this.selectedGraphicForCell(cell),
      1_000_000,
      (cell, graphic, startCell, startGraphic) =>
        this.graphicsMatchAcrossMaps(cell, graphic, startCell, startGraphic),
    );
    if (this.history.recordApplied(command)) this.commitDocuments();
    this.updateHistoryControls();
  }

  private finishRectangle(end: WorldTilePoint): void {
    const start = this.rectangleStart;
    this.rectangleStart = undefined;
    if (!start) return;
    const command = paintRectangle(
      this.access,
      this.resolveWorldCell,
      this.currentLayer,
      start,
      end,
      (cell) => this.selectedGraphicForCell(cell),
    );
    if (this.history.recordApplied(command)) this.commitDocuments();
    this.updateHistoryControls();
  }

  private pickGraphic(hit: MapSceneHit | undefined): void {
    const current = this.currentDocument();
    if (!current || !hit) return;
    const graphic = hit.block.document.readCell(hit.cell.layer, hit.cell.index);
    if (!graphic) return;
    if (graphic.tilesetIndex === 0 && graphic.tileIndex === 0) {
      this.setTool('erase');
      return;
    }
    const tilesetName = hit.block.document.tilesetNames[graphic.tilesetIndex];
    const currentTilesetIndex = tilesetName
      ? current.tilesetNames.indexOf(tilesetName)
      : -1;
    if (currentTilesetIndex < 0) {
      this.renderStatus.textContent = `Cannot pick ${tilesetName ?? 'unknown tileset'}: it is not in ${current.name}.`;
      this.renderStatus.classList.remove('muted');
      return;
    }
    this.tilesetSelect.value = String(currentTilesetIndex);
    this.populateTileSelect();
    this.tileSelect.value = String(graphic.tileIndex);
    this.setTool('pencil');
  }

  private canvasPoint(event: PointerEvent | WheelEvent) {
    const rect = this.canvas.getBoundingClientRect();
    if (
      this.canvasMetrics.logicalWidth <= 0 ||
      this.canvasMetrics.logicalHeight <= 0
    ) {
      writeCanvasMetrics(
        this.canvasMetrics,
        rect.width,
        rect.height,
        window.devicePixelRatio,
      );
    }
    return clientToLogicalCanvasPoint(
      this.pointerPoint,
      event.clientX,
      event.clientY,
      rect,
      this.canvasMetrics,
    );
  }

  private startGesture(hit: MapSceneHit | undefined): void {
    if (!hit || !this.isHitEditable(hit)) return;
    if (this.tool === 'pencil') {
      const graphic = this.selectedGraphicFor(hit.block.document);
      if (!graphic) return;
      this.gesture = createPencilGesture(this.access, graphic);
    } else if (this.tool === 'erase')
      this.gesture = createEraseGesture(this.access);
    else return;
    this.gesture.visit(hit.cell);
  }

  private visitGesture(hit: MapSceneHit | undefined): void {
    if (!this.gesture || !hit || !this.isHitEditable(hit)) return;
    if (this.tool === 'pencil') {
      const graphic = this.selectedGraphicFor(hit.block.document);
      if (graphic) this.gesture.visit(hit.cell, graphic);
    } else {
      this.gesture.visit(hit.cell);
    }
  }
  private finishGesture(): void {
    if (this.terrainStroke) {
      const stroke = this.terrainStroke;
      this.terrainStroke = undefined;
      if (this.history.recordApplied(stroke.finish())) this.commitDocuments();
      if (this.terrainIssueCount) {
        this.renderStatus.textContent = `Terrain stroke completed with ${this.terrainIssueCount} unresolved border variant${this.terrainIssueCount === 1 ? '' : 's'}.`;
        this.renderStatus.classList.remove('muted');
      }
      this.terrainIssueCount = 0;
      this.updateHistoryControls();
    }
    const gesture = this.gesture;
    if (!gesture) return;
    this.gesture = undefined;
    if (this.history.recordApplied(gesture.finish())) this.commitDocuments();
    this.updateHistoryControls();
  }
  private cancelGesture(): void {
    if (this.terrainStroke) {
      this.terrainStroke.cancel();
      this.terrainStroke = undefined;
      this.terrainIssueCount = 0;
    }
    if (this.gesture) {
      this.gesture.cancel();
      this.gesture = undefined;
    }
  }

  private startTerrain(point: WorldTilePoint): void {
    const lookup = this.terrainLookup;
    const workspace = this.mapWorkspace;
    const tag = this.terrainSelect.value as TerrainBorderTag;
    if (!lookup || !workspace || !tag) return;
    this.runLifecycle(() => {
      this.terrainIssueCount = 0;
      this.terrainStroke = new TerrainPaintStroke(
        this.access,
        workspace,
        lookup,
        this.currentLayer,
        tag,
      );
      this.visitTerrain(point);
    });
  }

  private visitTerrain(point: WorldTilePoint): void {
    if (!this.terrainStroke) return;
    this.terrainIssueCount += this.terrainStroke.paint(point).issues.length;
  }
  private undo(): void {
    this.finishGesture();
    if (this.history.undo(this.access)) this.commitDocuments();
    this.updateHistoryControls();
  }
  private redo(): void {
    this.finishGesture();
    if (this.history.redo(this.access)) this.commitDocuments();
    this.updateHistoryControls();
  }
  private commitDocuments(): void {
    this.context.session.replaceCollection(
      'maps',
      this.documents.map((document) => document.snapshot()) as JsonArray,
    );
    this.context.notifyChanged();
  }

  private mapSnapshots(): MapRecord[] {
    return this.documents.map((document) => document.snapshot());
  }

  private replaceMapState(
    maps: MapRecord[],
    selectedName: string,
    mapGrids?: ReturnType<MapGridTopology['snapshot']>[],
  ): void {
    this.finishGesture();
    this.history.clear();
    this.documents.length = 0;
    this.documentsByName.clear();
    maps.forEach((record, index) => {
      const document = MapDocument.from(record, `maps[${index}]`);
      this.documents.push(document);
      this.documentsByName.set(document.name, document);
    });
    if (mapGrids) {
      this.gridTopologies.length = 0;
      mapGrids.forEach((record, index) =>
        this.gridTopologies.push(
          MapGridTopology.from(record, `mapGrids[${index}]`),
        ),
      );
      this.context.session.replaceCollection('mapGrids', mapGrids as JsonArray);
    }
    this.context.session.replaceCollection('maps', maps as JsonArray);
    this.rebuildRenderDocuments();
    this.populateMapSelect();
    this.selectMap(
      this.documents.findIndex((document) => document.name === selectedName),
    );
    this.updateHistoryControls();
    this.context.notifyChanged();
  }

  private createMap(): void {
    const name = window.prompt(
      'New standalone map API name',
      this.uniqueMapName('NEW_MAP'),
    );
    if (name === null) return;
    const label = window.prompt('Map label', name.trim());
    if (label === null) return;
    const dimensions = this.promptDimensions(40, 40);
    if (!dimensions) return;
    this.runLifecycle(() => {
      const maps = createMapInCollection(
        this.mapSnapshots(),
        {
          name,
          label,
          width: dimensions.width,
          height: dimensions.height,
        },
        this.gridTopologies.map((grid) => grid.snapshot()),
      );
      this.replaceMapState(maps, name.trim());
    });
  }

  private cloneMap(): void {
    const current = this.currentDocument();
    if (!current) return;
    const name = window.prompt(
      'Clone map API name',
      this.uniqueMapName(`${current.name}_copy`),
    );
    if (name === null) return;
    const source = current.snapshot();
    const label = window.prompt(
      'Clone label',
      `${source.label ?? current.name} (Copy)`,
    );
    if (label === null) return;
    this.runLifecycle(() => {
      const maps = cloneMapInCollection(
        this.mapSnapshots(),
        current.name,
        { name, label },
        this.gridTopologies.map((grid) => grid.snapshot()),
      );
      this.replaceMapState(maps, name.trim());
    });
  }

  private editMapProperties(): void {
    const current = this.currentDocument();
    if (!current) return;
    const source = current.snapshot();
    const name = window.prompt('Map API name', current.name);
    if (name === null) return;
    const label = window.prompt('Map label', source.label ?? current.name);
    if (label === null) return;
    const type = window.prompt(
      'Map type: TOWN or OUTDOOR',
      source.type ?? 'TOWN',
    );
    if (type === null) return;
    const dimensions = this.promptDimensions(current.width, current.height);
    if (!dimensions) return;
    this.runLifecycle(() => {
      let maps = this.mapSnapshots();
      let grids = this.gridTopologies.map((grid) => grid.snapshot());
      const nextName = name.trim();
      if (nextName !== current.name) {
        const renamed = renameMapAcrossDatabase(
          { maps, mapGrids: grids },
          current.name,
          nextName,
        );
        maps = renamed.maps;
        grids = renamed.mapGrids;
      }
      const index = maps.findIndex((map) => map.name === nextName);
      if (index < 0) throw new Error(`Renamed map ${nextName} was not found`);
      let updated = updateMapMetadata(maps[index], {
        label,
        type: type.trim().toUpperCase() as 'TOWN' | 'OUTDOOR',
      });
      if (
        dimensions.width !== updated.width ||
        dimensions.height !== updated.height
      ) {
        updated = resizeMapRecord(updated, dimensions).map;
      }
      maps[index] = updated;
      this.replaceMapState(maps, nextName, grids);
    });
  }

  private deleteMap(): void {
    const current = this.currentDocument();
    if (!current) return;
    this.runLifecycle(() => {
      const result = deleteMapAcrossDatabase(
        {
          maps: this.mapSnapshots(),
          mapGrids: this.gridTopologies.map((grid) => grid.snapshot()),
        },
        current.name,
      );
      const references =
        result.preview.gridCells.length + result.preview.travelTriggers.length;
      if (
        !window.confirm(
          `Delete ${current.name}? ${references} map/grid reference${references === 1 ? '' : 's'} will be updated.`,
        )
      )
        return;
      const selected =
        result.maps[Math.min(this.currentIndex, result.maps.length - 1)];
      this.replaceMapState(result.maps, selected?.name ?? '', result.mapGrids);
    });
  }

  private addLayer(): void {
    const current = this.currentDocument();
    if (!current) return;
    const value = window.prompt(
      'New numeric layer',
      String(Math.max(...current.layers) + 1),
    );
    if (value === null) return;
    this.runLifecycle(() => {
      const maps = this.mapSnapshots();
      maps[this.currentIndex] = addMapLayer(
        maps[this.currentIndex],
        Number(value),
      );
      this.replaceMapState(maps, current.name);
    });
  }

  private deleteLayer(): void {
    const current = this.currentDocument();
    if (!current) return;
    this.runLifecycle(() => {
      const result = deleteMapLayer(current.snapshot(), this.currentLayer);
      const dropped = Object.values(result.droppedPlacements).reduce(
        (sum, count) => sum + count,
        0,
      );
      if (
        !window.confirm(
          `Delete layer ${this.currentLayer}? ${dropped} sparse placement${dropped === 1 ? '' : 's'} will also be removed.`,
        )
      )
        return;
      const maps = this.mapSnapshots();
      maps[this.currentIndex] = result.map;
      this.replaceMapState(maps, current.name);
    });
  }

  private promptDimensions(
    width: number,
    height: number,
  ): { width: number; height: number } | undefined {
    const value = window.prompt(
      'Map dimensions (width × height)',
      `${width}x${height}`,
    );
    if (value === null) return undefined;
    const match = /^\s*(\d+)\s*[x×,]\s*(\d+)\s*$/i.exec(value);
    if (!match) {
      this.renderStatus.textContent = 'Dimensions must look like 40x40.';
      this.renderStatus.classList.remove('muted');
      return undefined;
    }
    return { width: Number(match[1]), height: Number(match[2]) };
  }

  private uniqueMapName(base: string): string {
    const used = new Set(this.documents.map((document) => document.name));
    if (!used.has(base)) return base;
    for (let suffix = 2; ; suffix += 1) {
      const candidate = `${base}${suffix}`;
      if (!used.has(candidate)) return candidate;
    }
  }

  private runLifecycle(action: () => void): void {
    try {
      action();
    } catch (error) {
      this.renderStatus.textContent = `Map change failed: ${error instanceof Error ? error.message : 'unknown error'}`;
      this.renderStatus.classList.remove('muted');
    }
  }
  private updateHistoryControls(): void {
    this.undoButton.disabled = !this.history.canUndo;
    this.redoButton.disabled = !this.history.canRedo;
  }

  private updateSelectionStatus(): void {
    const current = this.currentDocument();
    if (!current) {
      this.selectionStatus.textContent = 'No maps are available.';
      this.updateMetadataEditor();
      return;
    }
    const hit = this.selectedHit ?? this.hoveredHit;
    const document = hit?.block.document ?? current;
    const position = hit ? document.coordinatesOf(hit.cell.index) : undefined;
    const graphic = hit
      ? document.readCell(this.currentLayer, hit.cell.index)
      : undefined;
    if (!position) {
      this.selectionStatus.textContent = `${current.name} · layer ${this.currentLayer}${this.scene.gridName ? ` · grid ${this.scene.gridName}` : ''}`;
      this.updateMetadataEditor();
      return;
    }
    if (!graphic) {
      this.selectionStatus.textContent = `${document.name}:${hit?.cell.index} (${position.x}, ${position.y}) · layer ${this.currentLayer} unavailable · context only`;
      this.updateMetadataEditor();
      return;
    }
    const tileset = document.tilesetNames[graphic.tilesetIndex] || '(empty)';
    const mode = this.selectedHit ? 'Selected' : 'Hover';
    const access = this.isHitEditable(hit) ? 'editable' : 'context only';
    this.selectionStatus.textContent = `${mode} ${document.name}:${hit?.cell.index} (${position.x}, ${position.y}) · ${tileset}:${graphic.tileIndex} · ${access}`;
    this.updateMetadataEditor();
  }

  private updateMetadataEditor(): void {
    const hit = this.selectedHit;
    const key = hit
      ? `${hit.cell.documentId}/${hit.cell.layer}/${hit.cell.index}`
      : '';
    if (key === this.metadataSelectionKey) return;
    this.metadataSelectionKey = key;
    this.metadataStatus.textContent = '';
    if (!hit || !this.isHitEditable(hit)) {
      this.metadataEditor.value = '';
      this.metadataEditor.disabled = true;
      return;
    }
    this.metadataEditor.disabled = false;
    this.metadataEditor.value = JSON.stringify(
      tilePlacementsAt(
        hit.block.document.snapshot(),
        hit.cell.layer,
        hit.cell.index,
      ),
      null,
      2,
    );
  }

  private applyTileMetadata(): void {
    const hit = this.selectedHit;
    if (!hit || !this.isHitEditable(hit)) return;
    this.runLifecycle(() => {
      const parsed = JSON.parse(this.metadataEditor.value) as unknown;
      if (
        typeof parsed !== 'object' ||
        parsed === null ||
        Array.isArray(parsed)
      ) {
        throw new TypeError('Tile metadata must be a JSON object');
      }
      const maps = this.mapSnapshots();
      const index = maps.findIndex((map) => map.name === hit.cell.documentId);
      if (index < 0)
        throw new Error(`Map ${hit.cell.documentId} was not found`);
      maps[index] = replaceTilePlacements(
        maps[index],
        hit.cell.layer,
        hit.cell.index,
        parsed as MapTilePlacementBundle,
      );
      const focusedName = this.currentDocument()?.name ?? hit.cell.documentId;
      this.replaceMapState(maps, focusedName);
      this.metadataStatus.textContent = 'Tile metadata applied.';
    });
  }

  private isHitEditable(hit: MapSceneHit | undefined): boolean {
    return Boolean(
      hit && isMapSceneBlockEditable(hit.block, this.currentLayer),
    );
  }

  private readonly handlePointerDown = (event: PointerEvent): void => {
    const point = this.canvasPoint(event);
    if (event.button === 1) {
      event.preventDefault();
      this.panPointerId = event.pointerId;
      this.lastPointerX = point.x;
      this.lastPointerY = point.y;
      this.canvas.setPointerCapture(event.pointerId);
      return;
    }
    if (event.button === 2) {
      event.preventDefault();
      this.pickGraphic(this.hitAtCanvasPoint(point.x, point.y));
      return;
    }
    if (event.button !== 0) return;
    const hit = this.hitAtCanvasPoint(point.x, point.y);
    const worldTile = this.worldTileAtCanvasPoint(point.x, point.y);
    this.hoveredHit = hit;
    this.selectedHit = hit;
    if (this.tool === 'fill' && worldTile) {
      this.runLifecycle(() => this.performFill(worldTile));
    } else if (this.tool === 'rectangle' && worldTile) {
      this.rectangleStart = worldTile;
      this.canvas.setPointerCapture(event.pointerId);
    } else if (this.tool === 'terrain' && worldTile) {
      this.startTerrain(worldTile);
      if (this.terrainStroke) this.canvas.setPointerCapture(event.pointerId);
    } else if (this.tool !== 'select') {
      this.startGesture(hit);
      if (this.gesture) this.canvas.setPointerCapture(event.pointerId);
    }
    this.updateSelectionStatus();
  };

  private readonly handlePointerMove = (event: PointerEvent): void => {
    const point = this.canvasPoint(event);
    if (this.panPointerId === event.pointerId) {
      this.currentViewport()?.panBy(
        point.x - this.lastPointerX,
        point.y - this.lastPointerY,
      );
      this.lastPointerX = point.x;
      this.lastPointerY = point.y;
      return;
    }
    const hit = this.hitAtCanvasPoint(point.x, point.y);
    if (!sameSceneHit(hit, this.hoveredHit)) {
      this.hoveredHit = hit;
      this.updateSelectionStatus();
    }
    if (this.gesture && (event.buttons & 1) !== 0) this.visitGesture(hit);
    if (this.terrainStroke && (event.buttons & 1) !== 0) {
      const worldTile = this.worldTileAtCanvasPoint(point.x, point.y);
      if (worldTile) this.visitTerrain(worldTile);
    }
  };

  private readonly handlePointerUp = (event: PointerEvent): void => {
    if (this.panPointerId === event.pointerId) this.panPointerId = undefined;
    if (event.button === 0 && this.rectangleStart) {
      const point = this.canvasPoint(event);
      const worldTile = this.worldTileAtCanvasPoint(point.x, point.y);
      if (worldTile) this.finishRectangle(worldTile);
      else this.rectangleStart = undefined;
    } else if (event.button === 0) this.finishGesture();
    if (this.canvas.hasPointerCapture(event.pointerId))
      this.canvas.releasePointerCapture(event.pointerId);
  };
  private readonly handlePointerCancel = (event: PointerEvent): void => {
    if (this.panPointerId === event.pointerId) this.panPointerId = undefined;
    this.rectangleStart = undefined;
    this.cancelGesture();
  };
  private readonly handleWheel = (event: WheelEvent): void => {
    event.preventDefault();
    const point = this.canvasPoint(event);
    const delta = normalizeWheelDelta(
      event.deltaY,
      event.deltaMode,
      this.canvasMetrics.logicalHeight,
    );
    this.currentViewport()?.zoomAt(point.x, point.y, wheelZoomFactor(delta));
  };
  private readonly handleKeyDown = (event: KeyboardEvent): void => {
    if (event.key === 'Tab' && !isTextInput(event.target)) {
      event.preventDefault();
      this.showOverlayLabels = true;
      return;
    }
    if (isTextInput(event.target)) return;
    const modifier = event.ctrlKey || event.metaKey;
    if (modifier && event.key.toLowerCase() === 'z') {
      event.preventDefault();
      if (event.shiftKey) this.redo();
      else this.undo();
    } else if (modifier && event.key.toLowerCase() === 'y') {
      event.preventDefault();
      this.redo();
    }
  };
  private readonly handleKeyUp = (event: KeyboardEvent): void => {
    if (event.key === 'Tab') this.showOverlayLabels = false;
  };
  private readonly renderFrame = (time: number): void => {
    if (this.destroyed) return;
    try {
      this.draw(time);
    } catch (error) {
      this.renderStatus.textContent = `Render error: ${error instanceof Error ? error.message : 'unknown error'}`;
      this.renderStatus.classList.remove('muted');
    } finally {
      this.frameRequest = requestAnimationFrame(this.renderFrame);
    }
  };

  private draw(time: number): void {
    const rect = this.canvas.getBoundingClientRect();
    const previousWidth = this.canvasMetrics.logicalWidth;
    const previousHeight = this.canvasMetrics.logicalHeight;
    writeCanvasMetrics(
      this.canvasMetrics,
      rect.width,
      rect.height,
      window.devicePixelRatio,
    );
    if (
      previousWidth !== this.canvasMetrics.logicalWidth ||
      previousHeight !== this.canvasMetrics.logicalHeight
    ) {
      this.centerPending = true;
    }
    const context = this.canvas.getContext('2d');
    if (!context) return;
    configureCanvasBackingStore(this.canvas, context, this.canvasMetrics);
    const width = this.canvasMetrics.logicalWidth;
    const height = this.canvasMetrics.logicalHeight;
    this.renderer.beginFrame(context, width, height);
    const current = this.currentDocument(),
      viewport = this.currentViewport();
    if (!current || !viewport) return;
    if (this.centerPending) {
      viewport.centerOn(
        (current.width * current.spriteWidth) / 2,
        (current.height * current.spriteHeight) / 2,
        width,
        height,
      );
      this.centerPending = false;
    }
    this.writeVisibleScene(viewport, width, height);
    this.renderer.drawMaps(context, viewport, this.renderBlocks);
    if (this.gridToggle.checked) {
      for (const block of this.scene.blocks)
        this.drawGrid(context, block, viewport, width, height);
    }
    const overlays = this.overlayRenderer.draw(
      context,
      viewport,
      this.scene.blocks,
      {
        layer: this.currentLayer,
        canvasWidth: width,
        canvasHeight: height,
        showLabels: this.showOverlayLabels,
      },
    );
    this.drawCellOutline(context, viewport, this.hoveredHit, '#78b9e8', 1);
    this.drawCellOutline(context, viewport, this.selectedHit, '#72ddc3', 2);
    if (time - this.lastStatusUpdate > 250) {
      this.lastStatusUpdate = time;
      const stats = this.renderer.stats;
      this.renderStatus.textContent = this.media
        ? `${stats.mapBlocks}/${this.scene.blocks.length} blocks · ${stats.drawnTiles} drawn · ${overlays} overlays · ${stats.visitedTiles} visible · ${stats.unresolvedSprites} unresolved · zoom ${viewport.scale.toFixed(2)}×`
        : 'Loading sprite definitions…';
    }
  }

  private drawGrid(
    context: CanvasRenderingContext2D,
    block: MapSceneBlock,
    viewport: Viewport,
    width: number,
    height: number,
  ): void {
    const current = block.document;
    if (
      !viewport.writeVisibleTileBounds(this.visibleBounds, {
        originX: block.originX,
        originY: block.originY,
        mapWidth: current.width,
        mapHeight: current.height,
        tileWidth: current.spriteWidth,
        tileHeight: current.spriteHeight,
        canvasWidth: width,
        canvasHeight: height,
      })
    )
      return;
    context.beginPath();
    context.strokeStyle = 'rgb(255 255 255 / 18%)';
    context.lineWidth = 1;
    for (
      let x = this.visibleBounds.minX;
      x <= this.visibleBounds.maxX + 1;
      x += 1
    ) {
      const sx =
        Math.round(
          viewport.worldToScreenX(block.originX + x * current.spriteWidth),
        ) + 0.5;
      context.moveTo(
        sx,
        viewport.worldToScreenY(
          block.originY + this.visibleBounds.minY * current.spriteHeight,
        ),
      );
      context.lineTo(
        sx,
        viewport.worldToScreenY(
          block.originY + (this.visibleBounds.maxY + 1) * current.spriteHeight,
        ),
      );
    }
    for (
      let y = this.visibleBounds.minY;
      y <= this.visibleBounds.maxY + 1;
      y += 1
    ) {
      const sy =
        Math.round(
          viewport.worldToScreenY(block.originY + y * current.spriteHeight),
        ) + 0.5;
      context.moveTo(
        viewport.worldToScreenX(
          block.originX + this.visibleBounds.minX * current.spriteWidth,
        ),
        sy,
      );
      context.lineTo(
        viewport.worldToScreenX(
          block.originX + (this.visibleBounds.maxX + 1) * current.spriteWidth,
        ),
        sy,
      );
    }
    context.stroke();
  }

  private drawCellOutline(
    context: CanvasRenderingContext2D,
    viewport: Viewport,
    hit: MapSceneHit | undefined,
    color: string,
    lineWidth: number,
  ): void {
    if (!hit) return;
    const current = hit.block.document;
    const position = current.coordinatesOf(hit.cell.index);
    if (!position) return;
    const x = viewport.worldToScreenX(
        hit.block.originX + position.x * current.spriteWidth,
      ),
      y = viewport.worldToScreenY(
        hit.block.originY + position.y * current.spriteHeight,
      ),
      right = viewport.worldToScreenX(
        hit.block.originX + (position.x + 1) * current.spriteWidth,
      ),
      bottom = viewport.worldToScreenY(
        hit.block.originY + (position.y + 1) * current.spriteHeight,
      );
    context.strokeStyle = color;
    context.lineWidth = lineWidth;
    context.strokeRect(
      Math.round(x) + 0.5,
      Math.round(y) + 0.5,
      Math.round(right) - Math.round(x),
      Math.round(bottom) - Math.round(y),
    );
  }

  private makeField(label: string, control: HTMLElement): HTMLLabelElement {
    const wrapper = element('label', { className: 'map-toolbar__field' });
    wrapper.append(element('span', { text: label }), control);
    return wrapper;
  }
  private makeButton(label: string, onClick: () => void): HTMLButtonElement {
    const control = element('button', {
      className: 'button button--small',
      text: label,
      attributes: { type: 'button' },
    });
    control.addEventListener('click', onClick);
    return control;
  }
}

function sameSceneHit(
  left: MapSceneHit | undefined,
  right: MapSceneHit | undefined,
): boolean {
  return (
    left?.block === right?.block &&
    left?.cell.documentId === right?.cell.documentId &&
    left?.cell.layer === right?.cell.layer &&
    left?.cell.index === right?.cell.index
  );
}
function isTextInput(target: EventTarget | null): boolean {
  return (
    target instanceof HTMLInputElement ||
    target instanceof HTMLTextAreaElement ||
    target instanceof HTMLSelectElement ||
    (target instanceof HTMLElement && target.isContentEditable)
  );
}
