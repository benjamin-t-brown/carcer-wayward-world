import { useMemo, useState } from 'react';
import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { MapPicker } from '../elements/MapPicker';
import { MapPreview } from '../elements/MapPreview';
import { Button } from '../elements/Button';
import { useAssets } from '../contexts/AssetsContext';
import {
  MapGridTemplate,
  createDefaultMapGridTemplate,
  resizeMapGridCells,
} from '../types/assets';
import { EditorEmptyState } from './EditorEmptyState';
import { openMapEditorInNewTab } from '../utils/mapTabsStorage';

export { createDefaultMapGridTemplate };

interface MapGridTemplateFormProps {
  mapGrid: MapGridTemplate | undefined;
  updateMapGrid: (mapGrid: MapGridTemplate) => void;
}

function clampPositiveInt(value: number, fallback: number): number {
  const n = Math.floor(value);
  return Number.isFinite(n) && n >= 1 ? n : fallback;
}

export function MapGridTemplateForm({
  mapGrid,
  updateMapGrid,
}: MapGridTemplateFormProps) {
  const { maps } = useAssets();
  const [pickingCell, setPickingCell] = useState<{ x: number; y: number } | null>(
    null
  );

  const mapsByName = useMemo(() => {
    const byName: Record<string, (typeof maps)[number]> = {};
    for (const map of maps) {
      byName[map.name] = map;
    }
    return byName;
  }, [maps]);

  if (!mapGrid) {
    return (
      <EditorEmptyState message="Select a map grid from the list or create a new one." />
    );
  }

  const updateField = <K extends keyof MapGridTemplate>(
    field: K,
    value: MapGridTemplate[K]
  ) => {
    updateMapGrid({ ...mapGrid, [field]: value });
  };

  const updateGridSize = (gridWidth: number, gridHeight: number) => {
    const nextWidth = clampPositiveInt(gridWidth, mapGrid.gridWidth);
    const nextHeight = clampPositiveInt(gridHeight, mapGrid.gridHeight);
    updateMapGrid({
      ...mapGrid,
      gridWidth: nextWidth,
      gridHeight: nextHeight,
      cells: resizeMapGridCells(mapGrid.cells, nextWidth, nextHeight),
    });
  };

  const updateCell = (x: number, y: number, mapName: string) => {
    const nextCells = mapGrid.cells.map((row, rowY) =>
      row.map((cell, colX) => (rowY === y && colX === x ? mapName : cell))
    );
    updateMapGrid({ ...mapGrid, cells: nextCells });
  };

  const clearCell = (x: number, y: number) => {
    updateCell(x, y, '');
  };

  const assignedCount = mapGrid.cells.flat().filter((name) => name.trim() !== '').length;
  const totalSlots = mapGrid.gridWidth * mapGrid.gridHeight;
  const cellPreviewSize = Math.min(
    100,
    Math.max(48, Math.floor(520 / Math.max(mapGrid.gridWidth, mapGrid.gridHeight)))
  );

  return (
    <div className="item-form map-grid-template-form">
      <h2>Edit Map Grid</h2>
      <form>
        <div className="form-fields-inline">
          <TextInput
            id="map-grid-name"
            name="name"
            label="Name"
            value={mapGrid.name}
            onChange={(value) => updateField('name', value)}
            placeholder="e.g., alinea_region"
            required
          />

          <TextInput
            id="map-grid-label"
            name="label"
            label="Label"
            value={mapGrid.label}
            onChange={(value) => updateField('label', value)}
            placeholder="e.g., Alinea Region"
            required
          />
        </div>

        <div className="form-section">
          <h3>Grid Size</h3>
          <div
            style={{ marginBottom: '12px', fontSize: '11px', color: '#858585' }}
          >
            How many map slots wide and tall this grid contains.
          </div>
          <div className="form-fields-inline">
            <NumberInput
              id="map-grid-width"
              name="gridWidth"
              label="Maps Wide"
              value={mapGrid.gridWidth}
              onChange={(value) =>
                updateGridSize(value, mapGrid.gridHeight)
              }
              min={1}
              required
            />

            <NumberInput
              id="map-grid-height"
              name="gridHeight"
              label="Maps Tall"
              value={mapGrid.gridHeight}
              onChange={(value) =>
                updateGridSize(mapGrid.gridWidth, value)
              }
              min={1}
              required
            />
          </div>
          <div style={{ fontSize: '11px', color: '#858585' }}>
            {totalSlots} slot{totalSlots === 1 ? '' : 's'} total
            {assignedCount > 0 ? ` (${assignedCount} assigned)` : ''}
          </div>
        </div>

        <div className="form-section">
          <h3>Required Map Dimensions</h3>
          <div
            style={{ marginBottom: '12px', fontSize: '11px', color: '#858585' }}
          >
            Only maps with this tile width and height may be placed in the grid.
          </div>
          <div className="form-fields-inline">
            <NumberInput
              id="map-grid-map-width"
              name="mapWidth"
              label="Map Width (tiles)"
              value={mapGrid.mapWidth}
              onChange={(value) =>
                updateField('mapWidth', clampPositiveInt(value, mapGrid.mapWidth))
              }
              min={1}
              required
            />

            <NumberInput
              id="map-grid-map-height"
              name="mapHeight"
              label="Map Height (tiles)"
              value={mapGrid.mapHeight}
              onChange={(value) =>
                updateField('mapHeight', clampPositiveInt(value, mapGrid.mapHeight))
              }
              min={1}
              required
            />
          </div>
        </div>

        <div className="form-section">
          <h3>Map Assignments</h3>
          <div
            style={{ marginBottom: '12px', fontSize: '11px', color: '#858585' }}
          >
            Click a cell to assign a map. Only maps matching the required
            dimensions are shown. Filter by town or outdoor map type in the
            picker.
          </div>
          <div
            style={{
              display: 'grid',
              gridTemplateColumns: `repeat(${mapGrid.gridWidth}, minmax(0, 1fr))`,
              gap: '10px',
              maxWidth: '100%',
              overflowX: 'auto',
            }}
          >
            {mapGrid.cells.map((row, y) =>
              row.map((mapName, x) => {
                const assignedMap = mapName ? mapsByName[mapName] : undefined;
                const isMissing = mapName && !assignedMap;
                return (
                  <div
                    key={`${x}-${y}`}
                    style={{
                      display: 'flex',
                      flexDirection: 'column',
                      alignItems: 'center',
                      gap: '6px',
                      padding: '8px',
                      backgroundColor: '#252526',
                      border: '1px solid #3e3e42',
                      borderRadius: '4px',
                      minWidth: 0,
                    }}
                  >
                    <button
                      type="button"
                      onClick={() => setPickingCell({ x, y })}
                      style={{
                        display: 'flex',
                        flexDirection: 'column',
                        alignItems: 'center',
                        gap: '6px',
                        padding: 0,
                        border: 'none',
                        background: 'transparent',
                        cursor: 'pointer',
                        width: '100%',
                      }}
                    >
                      {assignedMap ? (
                        <MapPreview
                          map={assignedMap}
                          displaySize={cellPreviewSize}
                        />
                      ) : (
                        <div
                          style={{
                            width: cellPreviewSize,
                            height: cellPreviewSize,
                            display: 'flex',
                            alignItems: 'center',
                            justifyContent: 'center',
                            backgroundColor: '#1e1e1e',
                            borderRadius: '4px',
                            color: '#858585',
                            fontSize: '11px',
                            textAlign: 'center',
                            padding: '6px',
                          }}
                        >
                          {isMissing ? 'Missing map' : 'Empty'}
                        </div>
                      )}
                      <div
                        style={{
                          fontSize: '10px',
                          color: isMissing ? '#f48771' : '#d4d4d4',
                          textAlign: 'center',
                          wordBreak: 'break-word',
                          width: '100%',
                        }}
                        title={mapName || `Cell ${x + 1}, ${y + 1}`}
                      >
                        {assignedMap?.label || mapName || `(${x + 1}, ${y + 1})`}
                      </div>
                    </button>
                    {assignedMap && (
                      <div
                        style={{
                          display: 'flex',
                          gap: '6px',
                          flexWrap: 'wrap',
                          justifyContent: 'center',
                        }}
                      >
                        <Button
                          variant="secondary"
                          onClick={() => openMapEditorInNewTab(assignedMap.name)}
                          style={{ fontSize: '10px', padding: '2px 8px' }}
                        >
                          Edit map
                        </Button>
                        <Button
                          variant="danger"
                          onClick={() => clearCell(x, y)}
                          style={{ fontSize: '10px', padding: '2px 8px' }}
                        >
                          Clear
                        </Button>
                      </div>
                    )}
                    {isMissing && (
                      <Button
                        variant="danger"
                        onClick={() => clearCell(x, y)}
                        style={{ fontSize: '10px', padding: '2px 8px' }}
                      >
                        Clear
                      </Button>
                    )}
                  </div>
                );
              })
            )}
          </div>
        </div>
      </form>

      {pickingCell && (
        <MapPicker
          value={mapGrid.cells[pickingCell.y]?.[pickingCell.x] ?? ''}
          onChange={(name) => {
            updateCell(pickingCell.x, pickingCell.y, name);
            setPickingCell(null);
          }}
          requiredWidth={mapGrid.mapWidth}
          requiredHeight={mapGrid.mapHeight}
          isOpen
          hideTrigger
          onOpenChange={(open) => {
            if (!open) {
              setPickingCell(null);
            }
          }}
        />
      )}
    </div>
  );
}
