import { useEffect, useRef, useState, useMemo } from 'react';
import { EditorState } from './editorState';
import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
  TileOverrides,
} from '../components/MapTemplateForm';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { useAssets } from '../contexts/AssetsContext';
import { CharacterTemplate } from '../components/CharacterTemplateForm';
import { ItemTemplate } from '../components/ItemTemplateForm';
import { getSpriteNameFromTile } from './draw';
import { Sprite } from '../elements/Sprite';

interface SelectedTileInfoProps {
  editorState: EditorState;
  map: CarcerMapTemplate;
  onMapUpdate: (map: CarcerMapTemplate) => void;
}

function OverrideCheckbox({
  value,
  label,
  onChange,
}: {
  value: boolean | undefined;
  label: string;
  onChange: (newValue: boolean | undefined) => void;
}) {
  // Treat undefined as false for display and toggle purposes
  const isChecked = value === true;

  const handleChange = () => {
    // Toggle between true and false only (treating undefined as false)
    const newValue = !isChecked;
    onChange(newValue);
  };

  return (
    <label
      style={{
        display: 'flex',
        alignItems: 'center',
        gap: '8px',
        cursor: 'pointer',
        fontSize: '12px',
      }}
    >
      <input
        type="checkbox"
        checked={isChecked}
        onChange={handleChange}
        style={{
          cursor: 'pointer',
          width: '16px',
          height: '16px',
        }}
      />
      <span style={{ color: '#ffffff' }}>{label}</span>
      <span
        style={{
          color: isChecked ? '#4ec9b0' : '#ff6b6b',
          fontSize: '10px',
          marginLeft: 'auto',
        }}
      >
        {isChecked ? 'true' : 'false'}
      </span>
    </label>
  );
}

function ItemSearchInput({
  items,
  onSelect,
  placeholder = 'Search items...',
}: {
  items: ItemTemplate[];
  onSelect: (itemName: string) => void;
  placeholder?: string;
}) {
  const [searchTerm, setSearchTerm] = useState('');
  const [isOpen, setIsOpen] = useState(false);
  const inputRef = useRef<HTMLInputElement>(null);
  const dropdownRef = useRef<HTMLDivElement>(null);

  const filteredItems = useMemo(() => {
    if (!searchTerm.trim()) return items;
    const term = searchTerm.toLowerCase();
    return items.filter(
      (item) =>
        item.name.toLowerCase().includes(term) ||
        item.label.toLowerCase().includes(term) ||
        item.itemType.toLowerCase().includes(term)
    );
  }, [items, searchTerm]);

  useEffect(() => {
    const handleClickOutside = (event: MouseEvent) => {
      if (
        dropdownRef.current &&
        !dropdownRef.current.contains(event.target as Node) &&
        inputRef.current &&
        !inputRef.current.contains(event.target as Node)
      ) {
        setIsOpen(false);
      }
    };

    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  const handleSelect = (itemName: string) => {
    onSelect(itemName);
    setSearchTerm('');
    setIsOpen(false);
  };

  return (
    <div style={{ position: 'relative', width: '100%' }}>
      <input
        ref={inputRef}
        type="text"
        value={searchTerm}
        onChange={(e) => {
          setSearchTerm(e.target.value);
          setIsOpen(true);
        }}
        onFocus={() => setIsOpen(true)}
        placeholder={placeholder}
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
      {isOpen && filteredItems.length > 0 && (
        <div
          ref={dropdownRef}
          style={{
            position: 'absolute',
            top: '100%',
            left: 0,
            right: 0,
            marginTop: '4px',
            maxHeight: '200px',
            overflowY: 'auto',
            backgroundColor: '#252526',
            border: '1px solid #3e3e42',
            borderRadius: '4px',
            zIndex: 1000,
            boxShadow: '0 4px 8px rgba(0, 0, 0, 0.3)',
          }}
        >
          {filteredItems.map((item) => (
            <div
              key={item.name}
              onClick={() => handleSelect(item.name)}
              style={{
                padding: '8px 12px',
                cursor: 'pointer',
                fontSize: '12px',
                color: '#ffffff',
                borderBottom: '1px solid #3e3e42',
                transition: 'background-color 0.2s',
              }}
              onMouseEnter={(e) => {
                e.currentTarget.style.backgroundColor = '#3e3e42';
              }}
              onMouseLeave={(e) => {
                e.currentTarget.style.backgroundColor = 'transparent';
              }}
            >
              <div style={{ fontWeight: 'bold' }}>{item.label}</div>
              <div
                style={{
                  fontSize: '10px',
                  color: '#858585',
                  marginTop: '2px',
                }}
              >
                {item.name} • {item.itemType}
              </div>
            </div>
          ))}
        </div>
      )}
    </div>
  );
}

function CharacterSearchInput({
  characters,
  onSelect,
  placeholder = 'Search characters...',
}: {
  characters: CharacterTemplate[];
  onSelect: (characterName: string) => void;
  placeholder?: string;
}) {
  const [searchTerm, setSearchTerm] = useState('');
  const [isOpen, setIsOpen] = useState(false);
  const inputRef = useRef<HTMLInputElement>(null);
  const dropdownRef = useRef<HTMLDivElement>(null);

  const filteredCharacters = useMemo(() => {
    if (!searchTerm.trim()) return characters;
    const term = searchTerm.toLowerCase();
    return characters.filter(
      (char) =>
        char.name.toLowerCase().includes(term) ||
        char.label.toLowerCase().includes(term) ||
        char.type.toLowerCase().includes(term)
    );
  }, [characters, searchTerm]);

  useEffect(() => {
    const handleClickOutside = (event: MouseEvent) => {
      if (
        dropdownRef.current &&
        !dropdownRef.current.contains(event.target as Node) &&
        inputRef.current &&
        !inputRef.current.contains(event.target as Node)
      ) {
        setIsOpen(false);
      }
    };

    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  const handleSelect = (characterName: string) => {
    onSelect(characterName);
    setSearchTerm('');
    setIsOpen(false);
  };

  return (
    <div style={{ position: 'relative', width: '100%' }}>
      <input
        ref={inputRef}
        type="text"
        value={searchTerm}
        onChange={(e) => {
          setSearchTerm(e.target.value);
          setIsOpen(true);
        }}
        onFocus={() => setIsOpen(true)}
        placeholder={placeholder}
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
      {isOpen && filteredCharacters.length > 0 && (
        <div
          ref={dropdownRef}
          style={{
            position: 'absolute',
            top: '100%',
            left: 0,
            right: 0,
            marginTop: '4px',
            maxHeight: '200px',
            overflowY: 'auto',
            backgroundColor: '#252526',
            border: '1px solid #3e3e42',
            borderRadius: '4px',
            zIndex: 1000,
            boxShadow: '0 4px 8px rgba(0, 0, 0, 0.3)',
          }}
        >
          {filteredCharacters.map((character) => (
            <div
              key={character.name}
              onClick={() => handleSelect(character.name)}
              style={{
                padding: '8px 12px',
                cursor: 'pointer',
                fontSize: '12px',
                color: '#ffffff',
                borderBottom: '1px solid #3e3e42',
                transition: 'background-color 0.2s',
              }}
              onMouseEnter={(e) => {
                e.currentTarget.style.backgroundColor = '#3e3e42';
              }}
              onMouseLeave={(e) => {
                e.currentTarget.style.backgroundColor = 'transparent';
              }}
            >
              <div style={{ fontWeight: 'bold' }}>{character.label}</div>
              <div
                style={{
                  fontSize: '10px',
                  color: '#858585',
                  marginTop: '2px',
                }}
              >
                {character.name} • {character.type}
              </div>
            </div>
          ))}
        </div>
      )}
    </div>
  );
}

export function SelectedTileInfo({
  editorState,
  map,
  onMapUpdate,
}: SelectedTileInfoProps) {
  const { sprites } = useSDL2WAssets();
  const { characters, items } = useAssets();
  const selectedTileInd = editorState.selectedTileInd;

  const updateTile = (updater: (tile: CarcerMapTileTemplate) => void) => {
    if (selectedTileInd < 0 || selectedTileInd >= map.tiles.length) {
      return;
    }

    const updatedTiles = [...map.tiles];
    const updatedTile = { ...updatedTiles[selectedTileInd] };
    updater(updatedTile);
    updatedTiles[selectedTileInd] = updatedTile;

    onMapUpdate({
      ...map,
      tiles: updatedTiles,
    });
  };

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
        tile.tileOverrides[key] = value;
      }
    });
  };

  if (selectedTileInd < 0 || selectedTileInd >= map.tiles.length) {
    return (
      <div
        style={{
          marginTop: '20px',
          padding: '10px',
          backgroundColor: '#2d2d30',
          borderRadius: '4px',
          border: '1px solid #3e3e42',
        }}
      >
        <div
          style={{
            color: '#858585',
            fontSize: '11px',
            textTransform: 'uppercase',
            fontWeight: 'bold',
            marginBottom: '8px',
          }}
        >
          Selected Tile
        </div>
        <div style={{ color: '#858585', fontSize: '12px' }}>
          No tile selected
        </div>
      </div>
    );
  }

  const selectedTile = map.tiles[selectedTileInd];
  const x = selectedTile.x;
  const y = selectedTile.y;
  const spriteName = getSpriteNameFromTile(selectedTile);
  const sprite = sprites.find((s) => s.name === spriteName);

  return (
    <div
      style={{
        marginTop: '20px',
        padding: '10px',
        backgroundColor: '#2d2d30',
        borderRadius: '4px',
        border: '1px solid #3e3e42',
      }}
    >
      <div
        style={{
          color: '#858585',
          fontSize: '11px',
          textTransform: 'uppercase',
          fontWeight: 'bold',
          marginBottom: '8px',
        }}
      >
        Selected Tile
      </div>
      <div
        style={{
          display: 'flex',
          flexDirection: 'column',
          gap: '6px',
          fontSize: '12px',
        }}
      >
        <div style={{ display: 'flex', justifyContent: 'space-between' }}>
          <span style={{ color: '#858585' }}>Index:</span>
          <span style={{ color: '#ffffff' }}>{selectedTileInd}</span>
        </div>
        <div style={{ display: 'flex', justifyContent: 'space-between' }}>
          <span style={{ color: '#858585' }}>Coordinates:</span>
          <span style={{ color: '#ffffff' }}>
            ({x}, {y})
          </span>
        </div>
        <div style={{ display: 'flex', justifyContent: 'space-between' }}>
          <span style={{ color: '#858585' }}>Sprite:</span>
          <span
            style={{
              color: '#ffffff',
              maxWidth: '150px',
              overflow: 'hidden',
              textOverflow: 'ellipsis',
              whiteSpace: 'nowrap',
            }}
            title={spriteName}
          >
            {spriteName || 'None'}
          </span>
        </div>
        {sprite && (
          <div
            style={{
              marginTop: '8px',
              display: 'flex',
              justifyContent: 'center',
              alignItems: 'center',
              padding: '8px',
              backgroundColor: '#1e1e1e',
              borderRadius: '4px',
            }}
          >
            <Sprite sprite={sprite} scale={2} maxWidth={64} />
          </div>
        )}
      </div>

      {/* Tile Overrides Section */}
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

      {/* Characters Section */}
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
          Characters
        </div>

        {/* Character Search Input */}
        <CharacterSearchInput
          characters={characters}
          onSelect={(characterName) => {
            updateTile((tile) => {
              if (!tile.characters.includes(characterName)) {
                tile.characters = [...tile.characters, characterName];
              }
            });
          }}
        />

        {/* Current Characters List */}
        {selectedTile.characters.length > 0 && (
          <div
            style={{
              marginTop: '10px',
              display: 'flex',
              flexDirection: 'column',
              gap: '6px',
            }}
          >
            {selectedTile.characters.map((characterName) => {
              const character = characters.find(
                (c) => c.name === characterName
              );
              return (
                <div
                  key={characterName + selectedTile.tileId}
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
                            (name) => name !== characterName
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
        )}
      </div>

      {/* Items Section */}
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
          Items
        </div>

        {/* Item Search Input */}
        <ItemSearchInput
          items={items}
          onSelect={(itemName) => {
            updateTile((tile) => {
              if (!tile.items.includes(itemName)) {
                tile.items = [...tile.items, itemName];
              }
            });
          }}
        />

        {/* Current Items List */}
        {selectedTile.items.length > 0 && (
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
        )}
      </div>
    </div>
  );
}
