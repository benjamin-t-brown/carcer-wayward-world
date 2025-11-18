import { CarcerMapTemplate } from '../components/MapTemplateForm';

interface TileEditorProps {
  map: CarcerMapTemplate;
  onMapUpdate: (map: CarcerMapTemplate) => void;
}

export function TileEditor({ map, onMapUpdate }: TileEditorProps) {
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
        <MapCanvas map={map} />
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

// Stub component for Map Canvas
function MapCanvas({ map }: { map: CarcerMapTemplate }) {
  return (
    <div
      style={{
        flex: 1,
        position: 'relative',
        backgroundColor: '#252526',
        overflow: 'hidden',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
      }}
    >
      <div style={{ color: '#858585', fontSize: '14px', textAlign: 'center' }}>
        TODO: Map Canvas
        <br />
        Pan-zoom enabled canvas for editing tiles
        <br />
        Map: {map.label} ({map.width} × {map.height})
      </div>
    </div>
  );
}

// Stub component for Tile Picker
function TilePicker() {
  return (
    <div
      style={{
        height: '200px',
        minHeight: '150px',
        maxHeight: '400px',
        borderTop: '1px solid #3e3e42',
        backgroundColor: '#1e1e1e',
        padding: '15px',
        overflow: 'auto',
        resize: 'vertical',
      }}
    >
      <div style={{ color: '#858585', fontSize: '14px' }}>
        TODO: Tile Picker
        <br />
        Resizable region for selecting tilesets and tiles
      </div>
    </div>
  );
}

