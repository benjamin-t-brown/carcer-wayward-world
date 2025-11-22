import { CarcerMapTileTemplate, GameEvent } from '../../types/assets';
import { EventSearchInput } from './EventSearchInput';
import { OverrideCheckbox } from './OverrideCheckbox';

interface EventTriggerSectionProps {
  selectedTile: CarcerMapTileTemplate;
  gameEvents: GameEvent[];
  updateTile: (updater: (tile: CarcerMapTileTemplate) => void) => void;
}

export function EventTriggerSection({
  selectedTile,
  gameEvents,
  updateTile,
}: EventTriggerSectionProps) {
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
  );
}

