import {
  CarcerMapTileTemplate,
  ItemTemplate,
  TilesetTemplate,
} from '../../../types/assets';
import { ItemSearchInput } from './ItemSearchInput';

interface DoorLockSectionProps {
  selectedTile: CarcerMapTileTemplate;
  tilesets: TilesetTemplate[];
  items: ItemTemplate[];
  updateTile: (updater: (tile: CarcerMapTileTemplate) => void) => void;
}

const fieldLabelStyle = {
  display: 'block',
  color: '#858585',
  fontSize: '11px',
  marginBottom: '4px',
} as const;

const numberInputStyle = {
  width: '100%',
  padding: '6px 8px',
  border: '1px solid #3e3e42',
  backgroundColor: '#1e1e1e',
  color: '#ffffff',
  fontSize: '12px',
  borderRadius: '4px',
  boxSizing: 'border-box',
} as const;

function clampLockLevel(value: number): number {
  if (!Number.isFinite(value)) {
    return 0;
  }
  return Math.max(0, Math.min(100, Math.trunc(value)));
}

function tileIsDoor(
  tile: CarcerMapTileTemplate,
  tilesets: TilesetTemplate[]
): boolean {
  if (!tile.tilesetName) {
    return false;
  }
  const tileset = tilesets.find((entry) => entry.name === tile.tilesetName);
  return tileset?.tiles[tile.tileId]?.isDoor === true;
}

export function DoorLockSection({
  selectedTile,
  tilesets,
  items,
  updateTile,
}: DoorLockSectionProps) {
  if (!tileIsDoor(selectedTile, tilesets)) {
    return null;
  }

  const locked = selectedTile.doorLock != null;
  const lockLevel = clampLockLevel(selectedTile.doorLock?.lockLevel ?? 0);
  const keyItemName = selectedTile.doorLock?.keyItem;
  const keyItem = keyItemName
    ? items.find((item) => item.name === keyItemName)
    : undefined;

  const setLocked = (nextLocked: boolean) => {
    updateTile((tile) => {
      if (!nextLocked) {
        delete tile.doorLock;
        return;
      }
      if (!tile.doorLock) {
        tile.doorLock = { lockLevel: 0 };
      }
    });
  };

  const setLockLevel = (raw: number) => {
    const nextLevel = clampLockLevel(raw);
    updateTile((tile) => {
      if (!tile.doorLock) {
        return;
      }
      const keyItem = tile.doorLock.keyItem;
      tile.doorLock =
        nextLevel === 0 && keyItem
          ? { lockLevel: nextLevel, keyItem }
          : { lockLevel: nextLevel };
    });
  };

  const setKeyItem = (itemName: string) => {
    updateTile((tile) => {
      if (!tile.doorLock || clampLockLevel(tile.doorLock.lockLevel) !== 0) {
        return;
      }
      tile.doorLock = { lockLevel: 0, keyItem: itemName };
    });
  };

  const clearKeyItem = () => {
    updateTile((tile) => {
      if (!tile.doorLock || clampLockLevel(tile.doorLock.lockLevel) !== 0) {
        return;
      }
      tile.doorLock = { lockLevel: 0 };
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
          color: '#858585',
          fontSize: '11px',
          textTransform: 'uppercase',
          fontWeight: 'bold',
          marginBottom: '10px',
        }}
      >
        Door Lock
      </div>

      <select
        value={locked ? 'locked' : 'unlocked'}
        onChange={(e) => setLocked(e.target.value === 'locked')}
        style={numberInputStyle}
      >
        <option value="unlocked">Unlocked</option>
        <option value="locked">Locked</option>
      </select>

      {locked && (
        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            gap: '8px',
            marginTop: '8px',
          }}
        >
          <div>
            <label style={fieldLabelStyle}>Level</label>
            <input
              type="number"
              min={0}
              max={100}
              step={1}
              value={lockLevel}
              onChange={(e) => {
                const parsed = parseInt(e.target.value, 10);
                setLockLevel(Number.isFinite(parsed) ? parsed : 0);
              }}
              style={numberInputStyle}
            />
          </div>

          {lockLevel === 0 && (
            <div>
              <label style={fieldLabelStyle}>Key</label>
              {keyItemName && (
                <div
                  style={{
                    display: 'flex',
                    justifyContent: 'space-between',
                    alignItems: 'center',
                    gap: '8px',
                    marginBottom: '8px',
                  }}
                >
                  <div style={{ minWidth: 0 }}>
                    <div
                      style={{
                        color: '#ffffff',
                        fontSize: '12px',
                        overflow: 'hidden',
                        textOverflow: 'ellipsis',
                        whiteSpace: 'nowrap',
                      }}
                    >
                      {keyItem?.label || keyItemName}
                    </div>
                    <div
                      style={{
                        color: '#858585',
                        fontSize: '10px',
                        marginTop: '2px',
                      }}
                    >
                      {keyItemName}
                    </div>
                  </div>
                  <button
                    type="button"
                    onClick={clearKeyItem}
                    style={{
                      padding: '4px 8px',
                      border: '1px solid #3e3e42',
                      backgroundColor: '#3e3e42',
                      color: '#ffffff',
                      cursor: 'pointer',
                      fontSize: '11px',
                      borderRadius: '4px',
                      flexShrink: 0,
                    }}
                  >
                    Clear
                  </button>
                </div>
              )}
              <ItemSearchInput
                items={items}
                placeholder="Search key item..."
                dropdownPlacement="below"
                onSelect={setKeyItem}
              />
            </div>
          )}
        </div>
      )}
    </div>
  );
}
