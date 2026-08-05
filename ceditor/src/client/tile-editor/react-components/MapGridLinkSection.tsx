import { useEffect, useMemo, useState } from 'react';
import { CarcerMapTemplate } from '../../types/assets';
import { OptionSelect } from '../../elements/OptionSelect';
import { Button } from '../../elements/Button';
import { useAssets } from '../../contexts/AssetsContext';
import { findAllMapGridPlacements } from '../../utils/mapGridIndex';
import { openMapGridEditorInNewTab } from '../../utils/mapTabsStorage';

interface MapGridLinkSectionProps {
  map: CarcerMapTemplate;
}

export function MapGridLinkSection({ map }: MapGridLinkSectionProps) {
  const { mapGrids } = useAssets();
  const placements = useMemo(
    () => findAllMapGridPlacements(map.name, mapGrids),
    [map.name, mapGrids]
  );
  const gridOptions = useMemo(
    () =>
      placements.map(({ grid }) => ({
        value: grid.name,
        label: grid.label?.trim() || grid.name,
      })),
    [placements]
  );
  const [selectedGridName, setSelectedGridName] = useState(
    () => gridOptions[0]?.value ?? ''
  );

  useEffect(() => {
    if (gridOptions.length === 0) {
      setSelectedGridName('');
      return;
    }
    if (!gridOptions.some((option) => option.value === selectedGridName)) {
      setSelectedGridName(gridOptions[0].value);
    }
  }, [gridOptions, selectedGridName]);

  const hasGrids = gridOptions.length > 0;

  return (
    <div
      style={{
        marginTop: '12px',
        paddingTop: '12px',
        borderTop: '1px solid #3e3e42',
        fontSize: '11px',
      }}
    >
      <OptionSelect
        id="map-grid-link-select"
        label="Map Grid"
        value={selectedGridName}
        onChange={setSelectedGridName}
        options={
          hasGrids
            ? gridOptions
            : [{ value: '', label: 'Not in any map grid' }]
        }
        disabled={!hasGrids}
        style={{ marginBottom: '8px', fontSize: '11px' }}
        inputStyle={{ width: '100%', fontSize: '11px' }}
      />
      <Button
        variant="small"
        disabled={!hasGrids || !selectedGridName}
        onClick={() => {
          if (selectedGridName) {
            openMapGridEditorInNewTab(selectedGridName);
          }
        }}
        style={{ width: '100%', fontSize: '11px' }}
      >
        🔗 Edit Map Grid
      </Button>
    </div>
  );
}
