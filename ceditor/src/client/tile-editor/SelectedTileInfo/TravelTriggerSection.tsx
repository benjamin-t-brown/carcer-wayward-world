import { CarcerMapTemplate, CarcerMapTileTemplate } from '../../types/assets';

interface TravelTriggerSectionProps {
  selectedTile: CarcerMapTileTemplate;
  maps: CarcerMapTemplate[];
  updateTile: (updater: (tile: CarcerMapTileTemplate) => void) => void;
  onOpenMapAndSelectTile?: (
    mapName: string,
    markerName?: string,
    x?: number,
    y?: number
  ) => void;
}

export function TravelTriggerSection({
  selectedTile,
  maps,
  updateTile,
  onOpenMapAndSelectTile,
}: TravelTriggerSectionProps) {
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
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
          marginBottom: '10px',
        }}
      >
        <div
          style={{
            color: '#858585',
            fontSize: '11px',
            textTransform: 'uppercase',
            fontWeight: 'bold',
          }}
        >
          Travel Trigger
        </div>
        {!selectedTile.travelTrigger ? (
          <button
            onClick={() => {
              updateTile((tile) => {
                tile.travelTrigger = {
                  destinationMapName: '',
                  destinationMarkerName: '',
                  destinationX: 0,
                  destinationY: 0,
                };
              });
            }}
            style={{
              padding: '4px 8px',
              border: '1px solid #3e3e42',
              backgroundColor: '#3e3e42',
              color: '#ffffff',
              cursor: 'pointer',
              fontSize: '11px',
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
            Create
          </button>
        ) : (
          <button
            onClick={() => {
              updateTile((tile) => {
                delete tile.travelTrigger;
              });
            }}
            style={{
              padding: '4px 8px',
              border: '1px solid #3e3e42',
              backgroundColor: '#5a2a2a',
              color: '#ffffff',
              cursor: 'pointer',
              fontSize: '11px',
              borderRadius: '4px',
              transition: 'background-color 0.2s',
            }}
            onMouseEnter={(e) => {
              e.currentTarget.style.backgroundColor = '#6a3a3a';
            }}
            onMouseLeave={(e) => {
              e.currentTarget.style.backgroundColor = '#5a2a2a';
            }}
          >
            Remove
          </button>
        )}
      </div>

      {selectedTile.travelTrigger && (
        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            gap: '10px',
          }}
        >
          <div>
            <label
              style={{
                display: 'block',
                color: '#858585',
                fontSize: '11px',
                marginBottom: '4px',
              }}
            >
              Destination Map Name
            </label>
            <select
              value={selectedTile.travelTrigger.destinationMapName}
              onChange={(e) => {
                updateTile((tile) => {
                  if (tile.travelTrigger) {
                    tile.travelTrigger.destinationMapName = e.target.value;
                  }
                });
              }}
              style={{
                width: '100%',
                padding: '6px 8px',
                border: '1px solid #3e3e42',
                backgroundColor: '#1e1e1e',
                color: '#ffffff',
                fontSize: '12px',
                borderRadius: '4px',
              }}
            >
              <option value="">Select a map...</option>
              {maps.map((m) => (
                <option key={m.name} value={m.name}>
                  {m.label || m.name}
                </option>
              ))}
            </select>
          </div>
          <div>
            <label
              style={{
                display: 'block',
                color: '#858585',
                fontSize: '11px',
                marginBottom: '4px',
              }}
            >
              Destination Marker Name
            </label>
            <input
              type="text"
              value={selectedTile.travelTrigger.destinationMarkerName}
              onChange={(e) => {
                updateTile((tile) => {
                  if (tile.travelTrigger) {
                    tile.travelTrigger.destinationMarkerName = e.target.value;
                  }
                });
              }}
              placeholder="Marker name..."
              style={{
                width: '100%',
                padding: '6px 8px',
                border: '1px solid #3e3e42',
                backgroundColor: '#1e1e1e',
                color: '#ffffff',
                fontSize: '12px',
                borderRadius: '4px',
              }}
            />
          </div>
          <div
            style={{
              display: 'grid',
              gridTemplateColumns: '1fr 1fr',
              gap: '10px',
            }}
          >
            <div>
              <label
                style={{
                  display: 'block',
                  color: '#858585',
                  fontSize: '11px',
                  marginBottom: '4px',
                }}
              >
                Destination X
              </label>
              <input
                type="number"
                value={selectedTile.travelTrigger.destinationX}
                onChange={(e) => {
                  const value = parseInt(e.target.value) || 0;
                  updateTile((tile) => {
                    if (tile.travelTrigger) {
                      tile.travelTrigger.destinationX = value;
                    }
                  });
                }}
                style={{
                  width: '100%',
                  padding: '6px 8px',
                  border: '1px solid #3e3e42',
                  backgroundColor: '#1e1e1e',
                  color: '#ffffff',
                  fontSize: '12px',
                  borderRadius: '4px',
                }}
              />
            </div>
            <div>
              <label
                style={{
                  display: 'block',
                  color: '#858585',
                  fontSize: '11px',
                  marginBottom: '4px',
                }}
              >
                Destination Y
              </label>
              <input
                type="number"
                value={selectedTile.travelTrigger.destinationY}
                onChange={(e) => {
                  const value = parseInt(e.target.value) || 0;
                  updateTile((tile) => {
                    if (tile.travelTrigger) {
                      tile.travelTrigger.destinationY = value;
                    }
                  });
                }}
                style={{
                  width: '100%',
                  padding: '6px 8px',
                  border: '1px solid #3e3e42',
                  backgroundColor: '#1e1e1e',
                  color: '#ffffff',
                  fontSize: '12px',
                  borderRadius: '4px',
                }}
              />
            </div>
          </div>
        </div>
      )}

      {/* Open Destination Map Button */}
      {selectedTile.travelTrigger &&
        selectedTile.travelTrigger.destinationMapName &&
        onOpenMapAndSelectTile && (
          <div style={{ marginTop: '10px' }}>
            <button
              onClick={() => {
                const {
                  destinationMapName,
                  destinationMarkerName,
                  destinationX,
                  destinationY,
                } = selectedTile.travelTrigger!;
                onOpenMapAndSelectTile(
                  destinationMapName,
                  destinationMarkerName || undefined,
                  destinationX,
                  destinationY
                );
              }}
              style={{
                width: '100%',
                padding: '6px 8px',
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
              title="Open destination map and select tile"
            >
              📍 Open Destination Map
            </button>
          </div>
        )}
    </div>
  );
}

