import { useEffect, useRef } from 'react';
import { CarcerMapTemplate } from '../types/assets';
import { useRenderLoop } from '../hooks/useRenderLoop';
import { MapCanvas } from './react-components/MapCanvas';
import {
  GridSlotCreateRequest,
  initPanzoom,
  setGridNavigationHandlers,
  unInitPanzoom,
} from './editorEvents';
import { loop } from './loop';
import { EditorState, getEditorState } from './editorState';
import { TilePicker } from './react-components/TilePicker';
import { ToolsPanel } from './react-components/ToolsPanel';
import { MapToolsOverlay } from './react-components/MapToolsOverlay';
import { useReRender } from '../hooks/useReRender';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { useAssets } from '../contexts/AssetsContext';
import { undo } from './paintTools';
import { LayersPanel } from './react-components/LayersPanel';
import { TerrainToolPanel } from './TerrainToolPanel';

interface TileEditorProps {
  map?: CarcerMapTemplate;
  onMapUpdate: (map: CarcerMapTemplate) => void;
  onOpenMapAndSelectTile?: (args: OpenMapAndSelectTileArgs) => void;
  onNavigateToGridMap?: (mapName: string) => void;
  onCreateGridMap?: (request: GridSlotCreateRequest) => void;
}

export interface OpenMapAndSelectTileArgs {
  mapName: string;
  level?: number;
  markerName?: string;
  pos?: { x: number; y: number };
}

let prevTs = performance.now();

export function TileEditor({
  map,
  onMapUpdate,
  onOpenMapAndSelectTile,
  onNavigateToGridMap,
  onCreateGridMap,
}: TileEditorProps) {
  const mapCanvasRef = useRef<HTMLCanvasElement>(null);
  const mapRef = useRef<CarcerMapTemplate | undefined>(undefined);
  const editorState = useRef<EditorState | undefined>(undefined);
  const { sprites, spriteMap } = useSDL2WAssets();
  const { tilesets, characters, items, gameEvents, maps, mapGrids } =
    useAssets();
  const gridNavigationRef = useRef({
    maps,
    mapGrids,
    onNavigateToGridMap,
    onCreateGridMap,
  });
  const reRender = useReRender();

  gridNavigationRef.current = {
    maps,
    mapGrids,
    onNavigateToGridMap,
    onCreateGridMap,
  };

  // console.log('re render tile editor');

  // hack im lazy
  (window as any).reRenderTileEditor = reRender;

  useEffect(() => {
    mapRef.current = map;
  }, [map]);
  useEffect(() => {
    editorState.current = getEditorState();
    editorState.current.tilesets = tilesets;
  }, [editorState]);

  useEffect(() => {
    console.log('initPanzoom');
    initPanzoom({
      getCanvas: () => mapCanvasRef.current as HTMLCanvasElement,
      getMapData: () => mapRef.current as CarcerMapTemplate,
      getEditorState: () => editorState.current as EditorState,
      getTilesets: () => tilesets,
    });
    setGridNavigationHandlers({
      getMaps: () => gridNavigationRef.current.maps,
      getMapGrids: () => gridNavigationRef.current.mapGrids,
      onNavigateToGridMap: (mapName) => {
        gridNavigationRef.current.onNavigateToGridMap?.(mapName);
      },
      onCreateGridMap: (request) => {
        gridNavigationRef.current.onCreateGridMap?.(request);
      },
    });
    return () => {
      console.log('unInitPanzoom');
      setGridNavigationHandlers(null);
      unInitPanzoom();
    };
  }, []);

  // Handle Ctrl+Z for undo
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      // Check if Ctrl+Z (or Cmd+Z on Mac) is pressed
      if ((e.ctrlKey || e.metaKey) && e.key === 'z' && !e.shiftKey) {
        // Check if we're not focused on an input field
        const activeElement = document.activeElement;
        const isInputFocused =
          activeElement &&
          (activeElement.tagName === 'INPUT' ||
            activeElement.tagName === 'TEXTAREA' ||
            activeElement.getAttribute('contenteditable') === 'true');

        if (!isInputFocused && mapRef.current && editorState.current) {
          e.preventDefault();
          const success = undo(mapRef.current, editorState.current);
          if (success) {
            // Trigger map update to reflect changes
            onMapUpdate({ ...mapRef.current });
          }
        }
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => {
      window.removeEventListener('keydown', handleKeyDown);
    };
  }, [onMapUpdate]);

  useRenderLoop((ts) => {
    if (mapCanvasRef.current && mapRef.current && editorState.current) {
      loop(
        {
          getCanvas: () => mapCanvasRef.current as HTMLCanvasElement,
          getMapData: () => mapRef.current as CarcerMapTemplate,
          getEditorState: () => editorState.current as EditorState,
          getSprites: () => sprites,
          getSpriteMap: () => spriteMap,
          getTilesets: () => tilesets,
          getAssets: () => ({
            characters: characters,
            items: items,
            tilesets: tilesets,
            gameEvents: gameEvents,
            maps: maps,
            mapGrids: mapGrids,
          }),
        },
        ts - prevTs
      );
    }
    prevTs = ts;
  });

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
        {editorState.current && editorState.current.selectedMapName && (
          <>
            <TerrainToolPanel editorState={editorState.current} />
            <ToolsPanel
              editorState={editorState.current}
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
          {editorState.current && editorState.current.selectedMapName && (
            <MapToolsOverlay editorState={editorState.current} />
          )}
          <MapCanvas
            canvasRef={mapCanvasRef}
            width={map.width * map.spriteWidth}
            height={map.height * map.spriteHeight}
          />
        </div>
        {editorState.current && editorState.current.selectedMapName && (
          <TilePicker editorState={editorState.current} />
        )}
      </div>

      {/* Right Column: Non-tile controls (grid, layers, find on map) */}
      <div className="tile-editor-sidebar tile-editor-sidebar--right">
        {editorState.current && editorState.current.selectedMapName && (
          <LayersPanel
            editorState={editorState.current}
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
