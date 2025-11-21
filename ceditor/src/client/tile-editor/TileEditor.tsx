import { useEffect, useRef } from 'react';
import { CarcerMapTemplate } from '../components/MapTemplateForm';
import { useRenderLoop } from './useRenderLoop';
import { MapCanvas } from './MapCanvas';
import { initPanzoom, unInitPanzoom } from './editorEvents';
import { loop } from './loop';
import { EditorState, getEditorState } from './editorState';
import { TilePicker } from './TilePicker';
import { ToolsPanel } from './ToolsPanel';
import { useReRender } from '../hooks/useReRender';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { useAssets } from '../contexts/AssetsContext';

interface TileEditorProps {
  map?: CarcerMapTemplate;
  onMapUpdate: (map: CarcerMapTemplate) => void;
}

let prevTs = performance.now();

export function TileEditor({ map, onMapUpdate }: TileEditorProps) {
  const mapCanvasRef = useRef<HTMLCanvasElement>(null);
  const mapRef = useRef<CarcerMapTemplate | undefined>(undefined);
  const editorState = useRef<EditorState | undefined>(undefined);
  const { sprites } = useSDL2WAssets();
  const { tilesets, characters, items, gameEvents, maps } = useAssets();
  const reRender = useReRender();

  console.log('re render tile editor');

  // hack im lazy
  (window as any).reRenderTileEditor = reRender;

  useEffect(() => {
    mapRef.current = map;
  }, [map]);
  useEffect(() => {
    editorState.current = getEditorState();
  }, [editorState]);

  useEffect(() => {
    console.log('initPanzoom');
    initPanzoom({
      getCanvas: () => mapCanvasRef.current as HTMLCanvasElement,
      getMapData: () => mapRef.current as CarcerMapTemplate,
      getEditorState: () => editorState.current as EditorState,
      getTilesets: () => tilesets,
    });
    return () => {
      console.log('unInitPanzoom');
      unInitPanzoom();
    };
  }, []);

  useRenderLoop((ts) => {
    if (mapCanvasRef.current && mapRef.current && editorState.current) {
      loop(
        {
          getCanvas: () => mapCanvasRef.current as HTMLCanvasElement,
          getMapData: () => mapRef.current as CarcerMapTemplate,
          getEditorState: () => editorState.current as EditorState,
          getSprites: () => sprites,
          getTilesets: () => tilesets,
          getAssets: () => ({
            characters: characters,
            items: items,
            tilesets: tilesets,
            gameEvents: gameEvents,
            maps: maps,
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
      {/* Left Column: Minimap and Tools */}
      <div
        style={{
          width: '250px',
          borderRight: '1px solid #3e3e42',
          backgroundColor: '#1e1e1e',
          display: 'flex',
          flexDirection: 'column',
          overflow: 'hidden',
        }}
      >
        <Minimap map={map} />
        {editorState.current && (
          <ToolsPanel
            editorState={editorState.current}
            map={map}
            onMapUpdate={onMapUpdate}
          />
        )}
      </div>

      {/* Right Column: Map Canvas and Tile Picker */}
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
          <MapCanvas
            canvasRef={mapCanvasRef}
            width={map.width * map.spriteWidth}
            height={map.height * map.spriteHeight}
          />
        </div>
        {editorState.current && (
          <TilePicker editorState={editorState.current} />
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
