import { useEffect, useRef } from 'react';
import { CarcerMapTemplate } from '../components/MapTemplateForm';
import { useRenderLoop } from './useRenderLoop';
import { MapCanvas } from './MapCanvas';
import { initPanzoom, unInitPanzoom } from './editorEvents';
import { loop } from './loop';
import { EditorState, getEditorState } from './editorState';
import { TilePicker } from './TilePicker';

interface TileEditorProps {
  map: CarcerMapTemplate;
  onMapUpdate: (map: CarcerMapTemplate) => void;
}

let prevTs = performance.now();

export function TileEditor({ map, onMapUpdate }: TileEditorProps) {
  const mapCanvasRef = useRef<HTMLCanvasElement>(null);
  const mapRef = useRef<CarcerMapTemplate | undefined>(undefined);
  const editorState = useRef<EditorState | undefined>(undefined);

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
        },
        ts - prevTs
      );
    }
    prevTs = ts;
  });

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
        <ToolsPanel />
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
        <TilePicker />
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

// Stub component for Tools Panel
function ToolsPanel() {
  return (
    <div
      style={{
        flex: 1,
        padding: '15px',
        overflow: 'auto',
      }}
    >
      <div style={{ color: '#858585', fontSize: '14px' }}>
        TODO: Tools Panel - Add tool buttons here
      </div>
    </div>
  );
}
