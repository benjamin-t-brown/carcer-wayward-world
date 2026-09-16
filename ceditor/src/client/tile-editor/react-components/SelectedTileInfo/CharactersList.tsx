import { CarcerMapTileTemplate, CharacterTemplate } from '../../../types/assets';

interface CharactersListProps {
  selectedTile: CarcerMapTileTemplate;
  characters: CharacterTemplate[];
  updateTile: (updater: (tile: CarcerMapTileTemplate) => void) => void;
}

export function CharactersList({
  selectedTile,
  characters,
  updateTile,
}: CharactersListProps) {
  if (selectedTile.characters.length === 0) {
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
      {selectedTile.characters.map((entry, index) => {
        const characterName = entry.name;
        const character = characters.find((c) => c.name === characterName);
        return (
          <div
            key={`${characterName}-${index}-${selectedTile.tileId}`}
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
                {character?.label || characterName}
              </div>
              {character && (
                <>
                  <div
                    style={{
                      color: '#858585',
                      fontSize: '10px',
                      marginTop: '2px',
                    }}
                  >
                    {character.name}
                  </div>
                  <div
                    style={{
                      color: '#858585',
                      fontSize: '10px',
                      marginTop: '2px',
                    }}
                  >
                    {character.type}
                  </div>
                </>
              )}
              <label
                style={{
                  display: 'flex',
                  alignItems: 'center',
                  gap: '6px',
                  marginTop: '6px',
                  cursor: 'pointer',
                  fontSize: '11px',
                  color: '#d4d4d4',
                  width: 'fit-content',
                }}
              >
                <input
                  type="checkbox"
                  checked={Boolean(entry.flipped)}
                  onChange={(e) => {
                    const flipped = e.target.checked;
                    updateTile((tile) => {
                      tile.characters = tile.characters.map((placed, i) =>
                        i === index ? { ...placed, flipped } : placed,
                      );
                    });
                  }}
                />
                Flipped
              </label>
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
                  }#/editor/characterTemplates?character=${encodeURIComponent(
                    characterName
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
                title="Edit character in new tab"
              >
                🔗
              </button>
              <button
                onClick={() => {
                  updateTile((tile) => {
                    tile.characters = tile.characters.filter(
                      (_, i) => i !== index,
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
                title="Remove character"
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
