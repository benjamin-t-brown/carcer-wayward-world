import { useEffect, useRef, useSyncExternalStore } from 'react';
import { CarcerMapTemplate } from '../types/assets';
import { MapCanvas } from './react-components/MapCanvas';
import {
  GridNavigateStitchOffset,
  GridSlotCreateRequest,
  initPanzoom,
  setGridNavigationHandlers,
  unInitPanzoom,
} from './editorEvents';
import { loop } from './loop';
import { TilePicker } from './react-components/TilePicker';
import { ToolsPanel } from './react-components/ToolsPanel';
import { MapToolsOverlay } from './react-components/MapToolsOverlay';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { useAssets } from '../contexts/AssetsContext';
import { undo } from './paintTools';
import { LayersPanel } from './react-components/LayersPanel';
import { TerrainToolPanel } from './TerrainToolPanel';
import { MapEditorController } from './MapEditorController';

interface TileEditorProps {
  controller: MapEditorController;
  map?: CarcerMapTemplate;
  onMapUpdate: (map: CarcerMapTemplate) => void;
  onOpenMapAndSelectTile?: (args: OpenMapAndSelectTileArgs) => void;
  onNavigateToGridMap?: (
    mapName: string,
    stitchOffset: GridNavigateStitchOffset,
  ) => void;
  onCreateGridMap?: (request: GridSlotCreateRequest) => void;
}

export interface OpenMapAndSelectTileArgs {
  mapName: string;
  level?: number;
  markerName?: string;
  pos?: { x: number; y: number };
}

export function TileEditor({
  controller,
  map,
  onMapUpdate,
  onOpenMapAndSelectTile,
  onNavigateToGridMap,
  onCreateGridMap,
}: TileEditorProps) {
  const mapCanvasRef = useRef<HTMLCanvasElement>(null);
  const { sprites, spriteMap } = useSDL2WAssets();
  const { tilesets, characters, items, gameEvents, maps, mapGrids } =
    useAssets();
  const gridNavigationRef = useRef({
    maps,
    mapGrids,
    onNavigateToGridMap,
    onCreateGridMap,
  });
  const renderDataRef = useRef({
    map,
    sprites,
    spriteMap,
    tilesets,
    characters,
    items,
    gameEvents,
    maps,
    mapGrids,
    onMapUpdate,
  });
  const hasMap = Boolean(map);

  useSyncExternalStore(
    controller.subscribe,
    controller.getSnapshot,
    controller.getSnapshot,
  );
  const editorState = controller.getState();

  gridNavigationRef.current = {
    maps,
    mapGrids,
    onNavigateToGridMap,
    onCreateGridMap,
  };
  renderDataRef.current = {
    map,
    sprites,
    spriteMap,
    tilesets,
    characters,
    items,
    gameEvents,
    maps,
    mapGrids,
    onMapUpdate,
  };

  useEffect(() => {
    if (controller.getState().tilesets !== tilesets) {
      controller.update({ tilesets });
    }
  }, [controller, tilesets]);

  useEffect(() => {
    const canvas = mapCanvasRef.current;
    if (!canvas || !renderDataRef.current.map) {
      return;
    }

    const getMap = () => renderDataRef.current.map as CarcerMapTemplate;
    const handleUndo = () => {
      const state = controller.getState();
      const order = state.gridUndoOrder;
      const targetName =
        order.length > 0
          ? order[order.length - 1]
          : (renderDataRef.current.map?.name ?? '');
      const targetMap =
        renderDataRef.current.maps.find((entry) => entry.name === targetName) ??
        renderDataRef.current.map;
      if (targetMap && undo(controller, targetMap, state, targetMap.name)) {
        if (order.length > 0) order.pop();
        renderDataRef.current.onMapUpdate({ ...targetMap });
      }
    };

    initPanzoom(controller, {
      controller,
      getCanvas: () => canvas,
      getMapData: getMap,
      getEditorState: controller.getState,
      getTilesets: () => renderDataRef.current.tilesets,
      onUndo: handleUndo,
      onMapUpdate: (updatedMap) =>
        renderDataRef.current.onMapUpdate(updatedMap),
    });
    setGridNavigationHandlers(controller, {
      getMaps: () => gridNavigationRef.current.maps,
      getMapGrids: () => gridNavigationRef.current.mapGrids,
      getDocumentIndex: () => {
        const data = renderDataRef.current;
        return controller.getDocumentIndex({
          characters: data.characters,
          items: data.items,
          tilesets: data.tilesets,
          gameEvents: data.gameEvents,
          maps: data.maps,
          mapGrids: data.mapGrids,
        });
      },
      onNavigateToGridMap: (mapName, stitchOffset) => {
        gridNavigationRef.current.onNavigateToGridMap?.(mapName, stitchOffset);
      },
      onCreateGridMap: (request) => {
        gridNavigationRef.current.onCreateGridMap?.(request);
      },
    });

    controller.start((_timestamp, elapsedMs) => {
      const data = renderDataRef.current;
      if (!data.map) return;
      loop(
        controller,
        {
          getCanvas: () => canvas,
          getMapData: () => data.map as CarcerMapTemplate,
          getEditorState: controller.getState,
          getSprites: () => data.sprites,
          getSpriteMap: () => data.spriteMap,
          getTilesets: () => data.tilesets,
          getAssets: () => ({
            characters: data.characters,
            items: data.items,
            tilesets: data.tilesets,
            gameEvents: data.gameEvents,
            maps: data.maps,
            mapGrids: data.mapGrids,
          }),
        },
        elapsedMs,
      );
    });

    return () => {
      controller.stop();
      setGridNavigationHandlers(controller, null);
      unInitPanzoom(controller);
    };
  }, [controller, hasMap]);

  if (!map) {
    return (
      <div
        style={{
          color: '#858585',
          fontSize: '14px',
          textAlign: 'center',
          marginTop: '50px',
        }}
      >
        Select a map from the dropdown or create a new one to get started.
      </div>
    );
  }

  return (
    <div
      style={{
        display: 'flex',
        height: '100%',
        width: '100%',
        overflow: 'hidden',
      }}
    >
      {/* Left Column: Tile-related tools */}
      <div className="tile-editor-sidebar">
        {/* <Minimap map={map} /> */}
        {editorState.selectedMapName && (
          <>
            <TerrainToolPanel
              controller={controller}
              editorState={editorState}
            />
            <ToolsPanel
              controller={controller}
              editorState={editorState}
              map={map}
              onMapUpdate={onMapUpdate}
              onOpenMapAndSelectTile={onOpenMapAndSelectTile}
            />
          </>
        )}
      </div>

      {/* Center Column: Map Canvas and Tile Picker */}
      <div
        style={{
          flex: 1,
          display: 'flex',
          flexDirection: 'column',
          overflow: 'hidden',
        }}
      >
        <div
          style={{
            flex: 1,
            position: 'relative',
            backgroundColor: '#252526',
            overflow: 'hidden',
          }}
        >
          {editorState.selectedMapName && (
            <MapToolsOverlay
              controller={controller}
              editorState={editorState}
            />
          )}
          <MapCanvas
            canvasRef={mapCanvasRef}
            width={map.width * map.spriteWidth}
            height={map.height * map.spriteHeight}
          />
        </div>
        {editorState.selectedMapName && (
          <TilePicker controller={controller} editorState={editorState} />
        )}
      </div>

      {/* Right Column: Non-tile controls (grid, layers, find on map) */}
      <div className="tile-editor-sidebar tile-editor-sidebar--right">
        {editorState.selectedMapName && (
          <LayersPanel
            controller={controller}
            editorState={editorState}
            map={map}
            onMapUpdate={onMapUpdate}
          />
        )}
      </div>
    </div>
  );
}

// Stub component for Minimap
function Minimap({ map }: { map: CarcerMapTemplate }) {
  return (
    <div
      style={{
        height: '200px',
        padding: '15px',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        borderBottom: '1px solid #3e3e42',
      }}
    >
      <div style={{ color: '#858585', fontSize: '14px', textAlign: 'center' }}>
        TODO: Minimap
        <br />
        Map: {map.label} ({map.width} × {map.height})
      </div>
    </div>
  );
}
