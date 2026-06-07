import { CarcerMapTileTemplate, ItemTemplate } from '../../../types/assets';
import {
  moveMapTileItem,
  removeMapTileItem,
  setMapTileItemQuantity,
} from '../../mapTileItems';

function ReorderButton({
  label,
  disabled,
  onClick,
  title,
}: {
  label: string;
  disabled: boolean;
  onClick: () => void;
  title: string;
}) {
  return (
    <button
      type="button"
      disabled={disabled}
      onClick={onClick}
      title={title}
      style={{
        padding: '2px 4px',
        minWidth: '22px',
        height: '22px',
        border: '1px solid #3e3e42',
        borderRadius: '4px',
        backgroundColor: disabled ? '#2a2a2a' : '#3e3e42',
        color: disabled ? '#555555' : '#cccccc',
        cursor: disabled ? 'not-allowed' : 'pointer',
        fontSize: '12px',
        lineHeight: 1,
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
      }}
    >
      {label}
    </button>
  );
}

interface ItemsListProps {
  selectedTile: CarcerMapTileTemplate;
  items: ItemTemplate[];
  updateTile: (updater: (tile: CarcerMapTileTemplate) => void) => void;
}

export function ItemsList({
  selectedTile,
  items,
  updateTile,
}: ItemsListProps) {
  const tileItems = selectedTile.items;

  if (tileItems.length === 0) {
    return null;
  }

  return (
    <div
      style={{
        marginTop: '10px',
        display: 'flex',
        flexDirection: 'column',
        gap: '6px',
      }}
    >
      {tileItems.map((entry, index) => {
        const { name: itemName, quantity } = entry;
        const item = items.find((i) => i.name === itemName);
        const stackable = Boolean(item?.stackable);

        return (
          <div
            key={`${itemName}-${index}-${selectedTile.tileId}`}
            style={{
              display: 'flex',
              justifyContent: 'space-between',
              alignItems: 'flex-start',
              gap: '8px',
              padding: '6px 8px',
              backgroundColor: '#1e1e1e',
              borderRadius: '4px',
              border: '1px solid #3e3e42',
            }}
          >
            <div
              style={{
                display: 'flex',
                flexDirection: 'column',
                gap: '2px',
                flexShrink: 0,
              }}
            >
              <ReorderButton
                label="↑"
                title="Move up"
                disabled={index === 0}
                onClick={() => {
                  updateTile((tile) => {
                    tile.items = moveMapTileItem(tile.items, index, 'up');
                  });
                }}
              />
              <ReorderButton
                label="↓"
                title="Move down"
                disabled={index === tileItems.length - 1}
                onClick={() => {
                  updateTile((tile) => {
                    tile.items = moveMapTileItem(tile.items, index, 'down');
                  });
                }}
              />
            </div>
            <div style={{ flex: 1, minWidth: 0 }}>
              <div style={{ color: '#ffffff', fontSize: '12px' }}>
                {item?.label || itemName}
              </div>
              {stackable ? (
                <div
                  style={{
                    display: 'flex',
                    alignItems: 'center',
                    gap: '6px',
                    marginTop: '6px',
                  }}
                >
                  <span
                    style={{
                      color: '#858585',
                      fontSize: '10px',
                      textTransform: 'uppercase',
                    }}
                  >
                    Qty
                  </span>
                  <input
                    type="number"
                    min={1}
                    step={1}
                    value={quantity}
                    title="Stack quantity"
                    onChange={(e) => {
                      const parsed = parseInt(e.target.value, 10);
                      const nextQty =
                        Number.isFinite(parsed) && parsed > 0 ? parsed : 1;
                      updateTile((tile) => {
                        tile.items = setMapTileItemQuantity(
                          tile.items,
                          itemName,
                          nextQty
                        );
                      });
                    }}
                    style={{
                      width: '44px',
                      padding: '2px 4px',
                      border: '1px solid #3e3e42',
                      borderRadius: '4px',
                      backgroundColor: '#252526',
                      color: '#ffffff',
                      fontSize: '12px',
                      textAlign: 'center',
                      boxSizing: 'border-box',
                    }}
                  />
                </div>
              ) : null}
              {item && (
                <>
                  <div
                    style={{
                      color: '#858585',
                      fontSize: '10px',
                      marginTop: '4px',
                    }}
                  >
                    {item.name}
                  </div>
                  <div
                    style={{
                      color: '#858585',
                      fontSize: '10px',
                      marginTop: '2px',
                    }}
                  >
                    {item.itemType}
                  </div>
                </>
              )}
            </div>
            <div
              style={{
                display: 'flex',
                flexDirection: 'column',
                gap: '4px',
                alignItems: 'center',
                flexShrink: 0,
              }}
            >
              <button
                onClick={(e) => {
                  e.stopPropagation();
                  const url = `${window.location.origin}${
                    window.location.pathname
                  }#/editor/itemTemplates?item=${encodeURIComponent(itemName)}`;
                  window.open(url, '_blank');
                }}
                style={{
                  padding: '4px 6px',
                  border: 'none',
                  backgroundColor: 'transparent',
                  color: '#666666',
                  cursor: 'pointer',
                  fontSize: '14px',
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                  transition: 'color 0.2s',
                }}
                onMouseEnter={(e) => {
                  e.currentTarget.style.color = '#4ec9b0';
                }}
                onMouseLeave={(e) => {
                  e.currentTarget.style.color = '#666666';
                }}
                title="Edit item in new tab"
              >
                🔗
              </button>
              <button
                onClick={() => {
                  updateTile((tile) => {
                    tile.items = removeMapTileItem(tile.items, itemName);
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
                title="Remove item"
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
          </div>
        );
      })}
    </div>
  );
}
