import { useMemo, useState } from 'react';
import {
  CarcerMapTileTemplate,
  GameEvent,
  TilesetTemplate,
} from '../../../types/assets';
import { useAssets } from '../../../contexts/AssetsContext';
import { isMapTileWalkable } from '../../mapTileWalkability';
import { EventSearchInput } from './EventSearchInput';
import { OverrideCheckbox } from './OverrideCheckbox';
import { CreateSignModal, CreateSignModalResult } from './CreateSignModal';
import { createSignGameEvent } from './createSignGameEvent';

interface EventTriggerSectionProps {
  selectedTile: CarcerMapTileTemplate;
  tilesets: TilesetTemplate[];
  gameEvents: GameEvent[];
  mapName: string;
  updateTile: (updater: (tile: CarcerMapTileTemplate) => void) => void;
}

export function EventTriggerSection({
  selectedTile,
  tilesets,
  gameEvents,
  mapName,
  updateTile,
}: EventTriggerSectionProps) {
  const { setGameEvents, saveGameEvents } = useAssets();
  const [isCreateSignOpen, setIsCreateSignOpen] = useState(false);
  const [isCreatingSign, setIsCreatingSign] = useState(false);

  const existingEventIds = useMemo(
    () => new Set(gameEvents.map((event) => event.id)),
    [gameEvents]
  );

  const assignEventToTile = (
    eventId: string,
    options?: { forceLookTrigger?: boolean }
  ) => {
    const walkable = isMapTileWalkable(selectedTile, tilesets);
    updateTile((tile) => {
      tile.eventTrigger = {
        eventId,
        isNonCombatTrigger: true,
        isLookTrigger: options?.forceLookTrigger ? true : !walkable,
      };
    });
  };

  const handleCreateSign = async (result: CreateSignModalResult) => {
    if (existingEventIds.has(result.triggerId)) {
      return;
    }

    const newEvent = createSignGameEvent({
      id: result.triggerId,
      title: result.title,
      contents: result.contents,
    });
    const nextEvents = [...gameEvents, newEvent].sort((a, b) =>
      a.id.localeCompare(b.id)
    );

    setIsCreatingSign(true);
    try {
      await saveGameEvents(nextEvents);
      setGameEvents(nextEvents);
      assignEventToTile(result.triggerId, { forceLookTrigger: true });
      setIsCreateSignOpen(false);
    } catch (err) {
      console.error('Failed to save sign event:', err);
      alert(
        `Failed to save sign event: ${
          err instanceof Error ? err.message : 'Unknown error'
        }`
      );
    } finally {
      setIsCreatingSign(false);
    }
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
        Event Trigger
      </div>

      {!selectedTile.eventTrigger && (
        <>
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
                fontSize: '10px',
                textTransform: 'uppercase',
                fontWeight: 'bold',
              }}
            >
              Quick create
            </div>
            <button
              onClick={() => setIsCreateSignOpen(true)}
              disabled={isCreatingSign}
              style={{
                padding: '4px 8px',
                border: '1px solid #3e3e42',
                backgroundColor: isCreatingSign ? '#2a2a2a' : '#3e3e42',
                color: '#ffffff',
                cursor: isCreatingSign ? 'default' : 'pointer',
                fontSize: '11px',
                borderRadius: '4px',
                transition: 'background-color 0.2s',
                opacity: isCreatingSign ? 0.6 : 1,
              }}
              onMouseEnter={(e) => {
                if (!isCreatingSign) {
                  e.currentTarget.style.backgroundColor = '#4a4a4a';
                }
              }}
              onMouseLeave={(e) => {
                if (!isCreatingSign) {
                  e.currentTarget.style.backgroundColor = '#3e3e42';
                }
              }}
            >
              Create Sign
            </button>
          </div>

          <EventSearchInput
            events={gameEvents}
            onSelect={(eventId) => {
              const event = gameEvents.find((e) => e.id === eventId);
              if (event && event.eventType === 'MODAL') {
                assignEventToTile(eventId);
              }
            }}
            placeholder="Search modal events..."
          />
        </>
      )}

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

      <CreateSignModal
        isOpen={isCreateSignOpen}
        mapName={mapName}
        existingEventIds={existingEventIds}
        onConfirm={handleCreateSign}
        onCancel={() => {
          if (!isCreatingSign) {
            setIsCreateSignOpen(false);
          }
        }}
      />
    </div>
  );
}
