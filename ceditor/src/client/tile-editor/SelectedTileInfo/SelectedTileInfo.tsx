import { CarcerMapTemplate, CarcerMapTileTemplate, TileOverrides } from '../../types/assets';
import { useSDL2WAssets } from '../../contexts/SDL2WAssetsContext';
import { useAssets } from '../../contexts/AssetsContext';
import { getSpriteNameFromTile } from '../draw';
import { Sprite } from '../../elements/Sprite';
import { EditorState } from '../editorState';
import { OverrideCheckbox } from './OverrideCheckbox';
import { ItemSearchInput } from './ItemSearchInput';
import { CharacterSearchInput } from './CharacterSearchInput';
import { EventSearchInput } from './EventSearchInput';

interface SelectedTileInfoProps {
  editorState: EditorState;
  map: CarcerMapTemplate;
  onMapUpdate: (map: CarcerMapTemplate) => void;
}

export function SelectedTileInfo({
  editorState,
  map,
  onMapUpdate,
}: SelectedTileInfoProps) {
  const { spriteMap } = useSDL2WAssets();
  const { characters, items, gameEvents, maps } = useAssets();
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
        tile.tileOverrides[key] = value as any;
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
  const x = selectedTileInd % map.width;
  const y = Math.floor(selectedTileInd / map.width);
  const spriteName = getSpriteNameFromTile(selectedTile);
  const sprite = spriteMap[spriteName];

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

      {/* Event Trigger Section */}
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
          Event Trigger
        </div>

        {/* Event Search Input - only show if no event trigger */}
        {!selectedTile.eventTrigger && (
          <EventSearchInput
            events={gameEvents}
            onSelect={(eventId) => {
              // Validate that the selected event is a MODAL event
              const event = gameEvents.find((e) => e.id === eventId);
              if (event && event.eventType === 'MODAL') {
                updateTile((tile) => {
                  tile.eventTrigger = {
                    eventId,
                    isNonCombatTrigger: true,
                    isLookTrigger: false,
                  };
                });
              }
            }}
            placeholder="Search modal events..."
          />
        )}

        {/* Current Event Trigger */}
        {selectedTile.eventTrigger && (
          <>
            <div
              style={{
                display: 'flex',
                justifyContent: 'space-between',
                alignItems: 'center',
                padding: '6px 8px',
                backgroundColor: '#1e1e1e',
                borderRadius: '4px',
                border: '1px solid #3e3e42',
                marginBottom: '10px',
              }}
            >
              <div style={{ flex: 1 }}>
                {(() => {
                  const event = gameEvents.find(
                    (e) => e.id === selectedTile.eventTrigger?.eventId
                  );
                  return (
                    <>
                      <div style={{ color: '#ffffff', fontSize: '12px' }}>
                        {event?.title || selectedTile.eventTrigger.eventId}
                      </div>
                      {event && (
                        <>
                          <div
                            style={{
                              color: '#858585',
                              fontSize: '10px',
                              marginTop: '2px',
                            }}
                          >
                            {event.id}
                          </div>
                          <div
                            style={{
                              color: '#858585',
                              fontSize: '10px',
                              marginTop: '2px',
                            }}
                          >
                            {event.eventType}
                          </div>
                          {event.eventType !== 'MODAL' && (
                            <div
                              style={{
                                color: '#ff6b6b',
                                fontSize: '10px',
                                marginTop: '2px',
                              }}
                            >
                              ⚠️ Only MODAL events are allowed
                            </div>
                          )}
                        </>
                      )}
                      {!event && (
                        <div
                          style={{
                            color: '#ff6b6b',
                            fontSize: '10px',
                            marginTop: '2px',
                          }}
                        >
                          Event not found
                        </div>
                      )}
                    </>
                  );
                })()}
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
                    const eventId = selectedTile.eventTrigger?.eventId;
                    if (eventId) {
                      const url = `${window.location.origin}${
                        window.location.pathname
                      }#/editor/specialEvents?event=${encodeURIComponent(
                        eventId
                      )}`;
                      window.open(url, '_blank');
                    }
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
                  title="Edit event in new tab"
                >
                  🔗
                </button>
                <button
                  onClick={() => {
                    updateTile((tile) => {
                      delete tile.eventTrigger;
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
                  title="Remove event trigger"
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
            {/* Event Trigger Options */}
            <div
              style={{
                display: 'flex',
                flexDirection: 'column',
                gap: '8px',
              }}
            >
              <OverrideCheckbox
                value={selectedTile.eventTrigger.isNonCombatTrigger}
                label="Non Combat Trigger"
                onChange={(newValue) => {
                  updateTile((tile) => {
                    if (tile.eventTrigger) {
                      tile.eventTrigger.isNonCombatTrigger = newValue;
                    }
                  });
                }}
              />
              <OverrideCheckbox
                value={selectedTile.eventTrigger.isLookTrigger}
                label="Look Trigger"
                onChange={(newValue) => {
                  updateTile((tile) => {
                    if (tile.eventTrigger) {
                      tile.eventTrigger.isLookTrigger = newValue;
                    }
                  });
                }}
              />
            </div>
          </>
        )}
      </div>

      {/* Travel Trigger Section */}
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
              <label
                style={{
                  display: 'block',
                  color: '#858585',
                  fontSize: '11px',
                  marginBottom: '4px',
                }}
              >
                Destination Map Name
              </label>
              <select
                value={selectedTile.travelTrigger.destinationMapName}
                onChange={(e) => {
                  updateTile((tile) => {
                    if (tile.travelTrigger) {
                      tile.travelTrigger.destinationMapName = e.target.value;
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
              >
                <option value="">Select a map...</option>
                {maps.map((m) => (
                  <option key={m.name} value={m.name}>
                    {m.label || m.name}
                  </option>
                ))}
              </select>
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
                Destination Marker Name
              </label>
              <input
                type="text"
                value={selectedTile.travelTrigger.destinationMarkerName}
                onChange={(e) => {
                  updateTile((tile) => {
                    if (tile.travelTrigger) {
                      tile.travelTrigger.destinationMarkerName = e.target.value;
                    }
                  });
                }}
                placeholder="Marker name..."
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
        )}
      </div>
    </div>
  );
}

