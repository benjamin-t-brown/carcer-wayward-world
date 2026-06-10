import { useMemo } from 'react';
import { CarcerMapTemplate, CarcerMapTileTemplate } from '../../../types/assets';
import { SearchSelect } from '../../../elements/SearchSelect';
import { collectMarkerNamesOnMap } from '../../mapLocate';
import { OpenMapAndSelectTileArgs } from '../../TileEditor';

interface TravelTriggerSectionProps {
  selectedTile: CarcerMapTileTemplate;
  maps: CarcerMapTemplate[];
  updateTile: (updater: (tile: CarcerMapTileTemplate) => void) => void;
  onOpenMapAndSelectTile?: (args: OpenMapAndSelectTileArgs) => void;
}

const fieldLabelStyle = {
  display: 'block',
  color: '#858585',
  fontSize: '11px',
  marginBottom: '4px',
} as const;

export function TravelTriggerSection({
  selectedTile,
  maps,
  updateTile,
  onOpenMapAndSelectTile,
}: TravelTriggerSectionProps) {
  const destinationMapName =
    selectedTile.travelTrigger?.destinationMapName ?? '';

  const destinationMap = useMemo(
    () => maps.find((m) => m.name === destinationMapName),
    [maps, destinationMapName]
  );

  const markerNames = useMemo(
    () => (destinationMap ? collectMarkerNamesOnMap(destinationMap) : []),
    [destinationMap]
  );

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
            <label style={fieldLabelStyle}>Destination Map Name</label>
            <SearchSelect
              id="travel-trigger-destination-map"
              value={selectedTile.travelTrigger.destinationMapName}
              onChange={(mapName) => {
                updateTile((tile) => {
                  if (!tile.travelTrigger) {
                    return;
                  }
                  tile.travelTrigger.destinationMapName = mapName;
                  if (!mapName) {
                    tile.travelTrigger.destinationMarkerName = '';
                    return;
                  }
                  const nextMap = maps.find((m) => m.name === mapName);
                  const names = nextMap
                    ? collectMarkerNamesOnMap(nextMap)
                    : [];
                  if (
                    !names.includes(tile.travelTrigger.destinationMarkerName)
                  ) {
                    tile.travelTrigger.destinationMarkerName = '';
                  }
                });
              }}
              items={maps}
              getItemKey={(m) => m.name}
              getItemLabel={(m) => m.label?.trim() || m.name}
              searchFields={(m) => [m.name, m.label ?? '']}
              placeholder="Search maps..."
              emptyLabel="Select a map..."
              renderItem={(m) => (
                <>
                  <div style={{ fontWeight: 600 }}>
                    {m.label?.trim() || m.name}
                  </div>
                  {m.label?.trim() && m.label.trim() !== m.name ? (
                    <div
                      style={{
                        fontSize: '10px',
                        color: '#858585',
                        marginTop: '2px',
                      }}
                    >
                      {m.name}
                    </div>
                  ) : null}
                </>
              )}
            />
          </div>
          <div>
            <label style={fieldLabelStyle}>Destination Marker Name</label>
            <SearchSelect
              id="travel-trigger-destination-marker"
              value={selectedTile.travelTrigger.destinationMarkerName}
              onChange={(markerName) => {
                updateTile((tile) => {
                  if (tile.travelTrigger) {
                    tile.travelTrigger.destinationMarkerName = markerName;
                  }
                });
              }}
              items={markerNames}
              getItemKey={(name) => name}
              getItemLabel={(name) => name}
              searchFields={(name) => [name]}
              placeholder={
                destinationMap
                  ? 'Search markers on map...'
                  : 'Select a destination map first'
              }
              emptyLabel="(no marker)"
              renderItem={(name) => (
                <div style={{ fontFamily: 'monospace', fontSize: '12px' }}>
                  {name}
                </div>
              )}
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
                onOpenMapAndSelectTile({
                  mapName: destinationMapName,
                  // level: getEditorState().currentLevel,
                  level: undefined,
                  markerName: destinationMarkerName || undefined,
                  pos: { x: destinationX, y: destinationY },
                });
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
