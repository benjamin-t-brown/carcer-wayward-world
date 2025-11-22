import { CarcerMapTileTemplate, TileOverrides } from '../../types/assets';
import { OverrideCheckbox } from './OverrideCheckbox';

interface TileOverridesSectionProps {
  selectedTile: CarcerMapTileTemplate;
  updateTile: (updater: (tile: CarcerMapTileTemplate) => void) => void;
}

export function TileOverridesSection({
  selectedTile,
  updateTile,
}: TileOverridesSectionProps) {
  const handleCreateOverrides = () => {
    updateTile((tile) => {
      tile.tileOverrides = {
        isWalkableOverride: undefined,
        isSeeThroughOverride: undefined,
        isContainerOverride: undefined,
      };
    });
  };

  const handleRemoveOverrides = () => {
    updateTile((tile) => {
      delete tile.tileOverrides;
    });
  };

  const handleToggleOverride = (
    key: keyof TileOverrides,
    value: boolean | undefined
  ) => {
    updateTile((tile) => {
      if (!tile.tileOverrides) {
        tile.tileOverrides = {};
      }
      if (value === undefined) {
        delete tile.tileOverrides[key];
        // If all overrides are removed, remove the object
        // if (
        //   !tile.tileOverrides.isWalkableOverride &&
        //   !tile.tileOverrides.isSeeThroughOverride &&
        //   !tile.tileOverrides.isContainerOverride
        // ) {
        //   delete tile.tileOverrides;
        // }
      } else {
        tile.tileOverrides[key] = value as any;
      }
    });
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
          Tile Overrides
        </div>
        {!selectedTile.tileOverrides ? (
          <button
            onClick={handleCreateOverrides}
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
            onClick={handleRemoveOverrides}
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

      {selectedTile.tileOverrides && (
        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            gap: '8px',
          }}
        >
          {(
            [
              'isWalkableOverride',
              'isSeeThroughOverride',
              'isContainerOverride',
            ] as const
          ).map((key) => {
            const value = selectedTile.tileOverrides?.[key];
            const label = key
              .replace('Override', '')
              .replace(/([A-Z])/g, ' $1')
              .trim();

            return (
              <OverrideCheckbox
                key={key + selectedTile.tileId}
                value={value}
                label={label}
                onChange={(newValue) => handleToggleOverride(key, newValue)}
              />
            );
          })}
        </div>
      )}
    </div>
  );
}

