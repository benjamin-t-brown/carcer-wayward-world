import { useMemo, useState } from 'react';
import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
} from '../../types/assets';
import { Button } from '../../elements/Button';
import { OpenMapAndSelectTileArgs } from '../TileEditor';
import {
  findTravelTriggerReferencesToMarker,
  locateOnCurrentMap,
  type MapMarkerReference,
} from '../mapLocate';

interface MarkersSectionProps {
  map: CarcerMapTemplate;
  maps: CarcerMapTemplate[];
  selectedTile: CarcerMapTileTemplate;
  updateTile: (updater: (tile: CarcerMapTileTemplate) => void) => void;
  onOpenMapAndSelectTile?: (args: OpenMapAndSelectTileArgs) => void;
}

function formatReferenceLabel(
  ref: MapMarkerReference,
  currentMapName: string
): string {
  const position = `Layer ${ref.level} · (${ref.x}, ${ref.y})`;
  if (ref.sourceMapName === currentMapName) {
    return position;
  }
  return `${ref.sourceMapLabel} · ${position}`;
}

function MarkerAccordionItem({
  markerName,
  map,
  maps,
  isOpen,
  onToggle,
  onRemove,
  onOpenMapAndSelectTile,
}: {
  markerName: string;
  map: CarcerMapTemplate;
  maps: CarcerMapTemplate[];
  isOpen: boolean;
  onToggle: () => void;
  onRemove: () => void;
  onOpenMapAndSelectTile?: (args: OpenMapAndSelectTileArgs) => void;
}) {
  const refs = useMemo(
    () => findTravelTriggerReferencesToMarker(map, markerName, maps),
    [map, markerName, maps]
  );

  const handleLocate = (ref: MapMarkerReference) => {
    if (ref.sourceMapName === map.name) {
      locateOnCurrentMap(map, ref);
      return;
    }
    onOpenMapAndSelectTile?.({
      mapName: ref.sourceMapName,
      level: ref.level,
      pos: { x: ref.x, y: ref.y },
    });
  };

  return (
    <div className="tile-editor-search-section tile-editor-marker-accordion">
      <div className="tile-editor-marker-accordion-header">
        <button
          type="button"
          className="tile-editor-search-section-header tile-editor-marker-accordion-toggle"
          onClick={onToggle}
          aria-expanded={isOpen}
        >
          <span
            className="tile-editor-marker-accordion-name"
            title={markerName}
          >
            {markerName}
          </span>
          <span className="tile-editor-marker-accordion-meta">
            {refs.length} trigger{refs.length === 1 ? '' : 's'}
          </span>
          <span className="tile-editor-search-section-chevron" aria-hidden>
            {isOpen ? '▾' : '▸'}
          </span>
        </button>
        <button
          type="button"
          className="tile-editor-marker-remove"
          onClick={onRemove}
          title="Remove marker from this tile"
        >
          <span
            style={{
              filter: 'grayscale(100%) brightness(1.75) sepia(100%)',
            }}
          >
            ✖️
          </span>
        </button>
      </div>
      {isOpen && (
        <div className="tile-editor-search-section-body tile-editor-marker-refs">
          {refs.length === 0 ? (
            <div className="tile-editor-marker-refs-empty">
              No travel triggers point to this marker.
            </div>
          ) : (
            refs.map((ref) => (
              <div
                key={`${ref.sourceMapName}-${ref.level}-${ref.tileIndex}`}
                className="tile-editor-marker-ref-row"
              >
                <span className="tile-editor-marker-ref-label">
                  {formatReferenceLabel(ref, map.name)}
                </span>
                <Button
                  variant="small"
                  type="button"
                  onClick={() => handleLocate(ref)}
                  disabled={
                    ref.sourceMapName !== map.name &&
                    !onOpenMapAndSelectTile
                  }
                >
                  Locate
                </Button>
              </div>
            ))
          )}
        </div>
      )}
    </div>
  );
}

export function MarkersSection({
  map,
  maps,
  selectedTile,
  updateTile,
  onOpenMapAndSelectTile,
}: MarkersSectionProps) {
  const [openMarker, setOpenMarker] = useState<string | null>(null);
  const markers = selectedTile.markers ?? [];

  const toggleMarker = (markerName: string) => {
    setOpenMarker((prev) => (prev === markerName ? null : markerName));
  };

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

      <div style={{ display: 'flex', gap: '8px', marginBottom: '10px' }}>
        <input
          type="text"
          placeholder="Enter marker name..."
          onKeyDown={(e) => {
            if (e.key === 'Enter') {
              const input = e.currentTarget;
              const markerName = input.value.trim();
              if (markerName && !markers.includes(markerName)) {
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
            const input = e.currentTarget
              .previousElementSibling as HTMLInputElement;
            const markerName = input.value.trim();
            if (markerName && !markers.includes(markerName)) {
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

      {markers.length > 0 && (
        <div className="tile-editor-marker-list">
          {markers.map((markerName) => (
            <MarkerAccordionItem
              key={markerName + selectedTile.tileId}
              markerName={markerName}
              map={map}
              maps={maps}
              isOpen={openMarker === markerName}
              onToggle={() => toggleMarker(markerName)}
              onOpenMapAndSelectTile={onOpenMapAndSelectTile}
              onRemove={() => {
                if (openMarker === markerName) {
                  setOpenMarker(null);
                }
                updateTile((tile) => {
                  tile.markers = (tile.markers || []).filter(
                    (name) => name !== markerName
                  );
                });
              }}
            />
          ))}
        </div>
      )}
    </div>
  );
}
