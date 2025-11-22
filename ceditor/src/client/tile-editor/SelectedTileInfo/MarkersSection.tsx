import { CarcerMapTileTemplate } from '../../types/assets';

interface MarkersSectionProps {
  selectedTile: CarcerMapTileTemplate;
  updateTile: (updater: (tile: CarcerMapTileTemplate) => void) => void;
}

export function MarkersSection({
  selectedTile,
  updateTile,
}: MarkersSectionProps) {
  return (
    <div
      style={{
        marginTop: '15px',
        paddingTop: '15px',
        borderTop: '1px solid #3e3e42',
      }}
    >
      <div
        style={{
          color: '#858585',
          fontSize: '11px',
          textTransform: 'uppercase',
          fontWeight: 'bold',
          marginBottom: '10px',
        }}
      >
        Markers
      </div>

      {/* Add Marker Input */}
      <div style={{ display: 'flex', gap: '8px', marginBottom: '10px' }}>
        <input
          type="text"
          placeholder="Enter marker name..."
          onKeyDown={(e) => {
            if (e.key === 'Enter') {
              const input = e.currentTarget;
              const markerName = input.value.trim();
              if (markerName && !selectedTile.markers?.includes(markerName)) {
                updateTile((tile) => {
                  tile.markers = [...(tile.markers || []), markerName];
                });
                input.value = '';
              }
            }
          }}
          style={{
            flex: 1,
            padding: '6px 8px',
            border: '1px solid #3e3e42',
            backgroundColor: '#1e1e1e',
            color: '#ffffff',
            fontSize: '12px',
            borderRadius: '4px',
            width: '50%',
          }}
        />
        <button
          onClick={(e) => {
            const input = e.currentTarget.previousElementSibling as HTMLInputElement;
            const markerName = input.value.trim();
            if (markerName && !selectedTile.markers?.includes(markerName)) {
              updateTile((tile) => {
                tile.markers = [...(tile.markers || []), markerName];
              });
              input.value = '';
            }
          }}
          style={{
            padding: '6px 12px',
            border: '1px solid #3e3e42',
            backgroundColor: '#3e3e42',
            color: '#ffffff',
            cursor: 'pointer',
            fontSize: '12px',
            borderRadius: '4px',
            transition: 'background-color 0.2s',
          }}
          onMouseEnter={(e) => {
            e.currentTarget.style.backgroundColor = '#4a4a4a';
          }}
          onMouseLeave={(e) => {
            e.currentTarget.style.backgroundColor = '#3e3e42';
          }}
        >
          Add
        </button>
      </div>

      {/* Current Markers List */}
      {selectedTile.markers?.length > 0 && (
        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            gap: '6px',
          }}
        >
          {selectedTile.markers.map((markerName) => (
            <div
              key={markerName + selectedTile.tileId}
              style={{
                display: 'flex',
                justifyContent: 'space-between',
                alignItems: 'center',
                padding: '6px 8px',
                backgroundColor: '#1e1e1e',
                borderRadius: '4px',
                border: '1px solid #3e3e42',
              }}
            >
              <div style={{ flex: 1 }}>
                <div
                  style={{
                    color: '#ffffff',
                    fontSize: '12px',
                    fontFamily: 'monospace',
                  }}
                >
                  {markerName}
                </div>
              </div>
              <button
                onClick={() => {
                  updateTile((tile) => {
                    tile.markers = (tile.markers || []).filter(
                      (name) => name !== markerName
                    );
                  });
                }}
                style={{
                  padding: '4px 6px',
                  border: '1px solid #3e3e42',
                  backgroundColor: '#5a2a2a',
                  color: '#ffffff',
                  cursor: 'pointer',
                  fontSize: '12px',
                  borderRadius: '4px',
                  transition: 'background-color 0.2s',
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                  minWidth: '24px',
                  height: '24px',
                }}
                onMouseEnter={(e) => {
                  e.currentTarget.style.backgroundColor = '#6a3a3a';
                }}
                onMouseLeave={(e) => {
                  e.currentTarget.style.backgroundColor = '#5a2a2a';
                }}
                title="Remove marker"
              >
                <span
                  style={{
                    filter:
                      'grayscale(100%) brightness(1.75) sepia(100%)',
                  }}
                >
                  ✖️
                </span>
              </button>
            </div>
          ))}
        </div>
      )}
    </div>
  );
}

