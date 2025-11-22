import { CarcerMapTileTemplate, ItemTemplate } from '../../types/assets';

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
  if (selectedTile.items.length === 0) {
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
      {selectedTile.items.map((itemName) => {
        const item = items.find((i) => i.name === itemName);
        return (
          <div
            key={itemName + selectedTile.tileId}
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
              <div style={{ color: '#ffffff', fontSize: '12px' }}>
                {item?.label || itemName}
              </div>
              {item && (
                <>
                  <div
                    style={{
                      color: '#858585',
                      fontSize: '10px',
                      marginTop: '2px',
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
              }}
            >
              <button
                onClick={(e) => {
                  e.stopPropagation();
                  const url = `${window.location.origin}${
                    window.location.pathname
                  }#/editor/itemTemplates?item=${encodeURIComponent(
                    itemName
                  )}`;
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
                    tile.items = tile.items.filter(
                      (name) => name !== itemName
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
                title="Remove item"
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
          </div>
        );
      })}
    </div>
  );
}

