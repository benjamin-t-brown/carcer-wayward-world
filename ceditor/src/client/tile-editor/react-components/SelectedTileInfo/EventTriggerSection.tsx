import { useMemo, useState } from 'react';
import {
  CarcerMapTileTemplate,
  GameEvent,
  TILE_OVERLAY_VISIBILITY_OPTIONS,
  TileOverlayVisibility,
  TilesetTemplate,
} from '../../../types/assets';
import { useAssets } from '../../../contexts/AssetsContext';
import { isMapTileWalkable } from '../../mapTileWalkability';
import { EventSearchInput } from './EventSearchInput';
import { OverrideCheckbox } from './OverrideCheckbox';
import { CreateSignModal, CreateSignModalResult } from './CreateSignModal';
import {
  CreateModalEventModal,
  CreateModalEventResult,
} from './CreateModalEventModal';
import { createSignGameEvent } from './createSignGameEvent';
import { createModalGameEvent } from './createModalGameEvent';
import { DeleteTileEventModal } from './DeleteTileEventModal';
import {
  ConditionEditButton,
  EditTileEventConditionModal,
} from './EditTileEventConditionModal';
import { findGameEventReferences } from '../../../utils/gameEventReferences';

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
  const { setGameEvents, saveGameEvents, maps, characters, items } = useAssets();
  const [isCreateSignOpen, setIsCreateSignOpen] = useState(false);
  const [isCreateModalOpen, setIsCreateModalOpen] = useState(false);
  const [isConditionModalOpen, setIsConditionModalOpen] = useState(false);
  const [isCreatingEvent, setIsCreatingEvent] = useState(false);
  const [pendingDeleteEventId, setPendingDeleteEventId] = useState<string | null>(
    null
  );
  const [isDeletingEvent, setIsDeletingEvent] = useState(false);

  const existingEventIds = useMemo(
    () => new Set(gameEvents.map((event) => event.id)),
    [gameEvents]
  );

  const assignEventToTile = (
    eventId: string,
    options?: {
      forceRequiresLook?: boolean;
      requiresNonCombat?: boolean;
      requiresLook?: boolean;
      overlayVisibility?: TileOverlayVisibility;
      condition?: string;
    }
  ) => {
    const walkable = isMapTileWalkable(selectedTile, tilesets);
    const condition = options?.condition?.trim();
    updateTile((tile) => {
      tile.eventTrigger = {
        eventId,
        requiresNonCombat: options?.requiresNonCombat ?? true,
        requiresLook:
          options?.requiresLook ??
          (options?.forceRequiresLook ? true : !walkable),
        overlayVisibility: options?.overlayVisibility ?? 'HIDDEN',
        ...(condition ? { condition } : {}),
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

    setIsCreatingEvent(true);
    try {
      const savedEvents = await saveGameEvents(nextEvents);
      setGameEvents(savedEvents);
      assignEventToTile(result.triggerId, { forceRequiresLook: true });
      setIsCreateSignOpen(false);
    } catch (err) {
      console.error('Failed to save sign event:', err);
      alert(
        `Failed to save sign event: ${
          err instanceof Error ? err.message : 'Unknown error'
        }`
      );
    } finally {
      setIsCreatingEvent(false);
    }
  };

  const handleCreateModal = async (result: CreateModalEventResult) => {
    if (existingEventIds.has(result.triggerId)) {
      return;
    }

    const newEvent = createModalGameEvent(result.triggerId, result.text);
    const nextEvents = [...gameEvents, newEvent].sort((a, b) =>
      a.id.localeCompare(b.id)
    );

    setIsCreatingEvent(true);
    try {
      const savedEvents = await saveGameEvents(nextEvents);
      setGameEvents(savedEvents);
      assignEventToTile(result.triggerId, {
        requiresNonCombat: result.requiresNonCombat,
        requiresLook: result.requiresLook,
        overlayVisibility: result.overlayVisibility,
        condition: result.condition,
      });
      setIsCreateModalOpen(false);
    } catch (err) {
      console.error('Failed to save modal event:', err);
      alert(
        `Failed to save modal event: ${
          err instanceof Error ? err.message : 'Unknown error'
        }`
      );
    } finally {
      setIsCreatingEvent(false);
    }
  };

  const removeTriggerFromTile = () => {
    updateTile((tile) => {
      delete tile.eventTrigger;
    });
    setPendingDeleteEventId(null);
  };

  const pendingEvent = pendingDeleteEventId
    ? gameEvents.find((event) => event.id === pendingDeleteEventId)
    : undefined;

  const otherUseNote = useMemo(() => {
    if (!pendingDeleteEventId || !pendingEvent) {
      return '';
    }
    const references = findGameEventReferences(
      maps,
      characters,
      items,
      gameEvents,
      pendingDeleteEventId
    );
    const parts: string[] = [];
    if (references.tiles.length > 1) {
      parts.push(`${references.tiles.length} tile triggers`);
    }
    if (references.characters.length > 0) {
      parts.push(
        `${references.characters.length} character${
          references.characters.length === 1 ? '' : 's'
        }`
      );
    }
    if (references.items.length > 0) {
      parts.push(
        `${references.items.length} item${references.items.length === 1 ? '' : 's'}`
      );
    }
    if (references.eventImports.length > 0) {
      parts.push(
        `${references.eventImports.length} event import${
          references.eventImports.length === 1 ? '' : 's'
        }`
      );
    }
    if (parts.length === 0) {
      return '';
    }
    return `This event is still used by ${parts.join(', ')}. Deleting it leaves those references behind.`;
  }, [
    pendingDeleteEventId,
    pendingEvent,
    maps,
    characters,
    items,
    gameEvents,
  ]);

  const handleDeleteEventToo = async () => {
    if (!pendingDeleteEventId || isDeletingEvent) {
      return;
    }
    const eventId = pendingDeleteEventId;
    const nextEvents = gameEvents
      .filter((event) => event.id !== eventId)
      .map((event) => ({
        ...event,
        vars: event.vars.filter((variable) => variable.importFrom !== eventId),
      }));

    setIsDeletingEvent(true);
    try {
      const savedEvents = await saveGameEvents(nextEvents);
      setGameEvents(savedEvents);
      removeTriggerFromTile();
    } catch (err) {
      console.error('Failed to delete event:', err);
      alert(
        `Failed to delete event: ${
          err instanceof Error ? err.message : 'Unknown error'
        }`
      );
    } finally {
      setIsDeletingEvent(false);
    }
  };

  return (
    <div
      style={{
        marginTop: '15px',
        paddingTop: '15px',
        borderTop: '1px solid #3e3e42',
        minWidth: 0,
        maxWidth: '100%',
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
              flexWrap: 'wrap',
              justifyContent: 'space-between',
              alignItems: 'center',
              gap: '6px',
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
            <div style={{ display: 'flex', gap: '6px' }}>
              <button
                onClick={() => setIsCreateSignOpen(true)}
                disabled={isCreatingEvent}
                style={{
                  padding: '4px 8px',
                  border: '1px solid #3e3e42',
                  backgroundColor: isCreatingEvent ? '#2a2a2a' : '#3e3e42',
                  color: '#ffffff',
                  cursor: isCreatingEvent ? 'default' : 'pointer',
                  fontSize: '11px',
                  borderRadius: '4px',
                  transition: 'background-color 0.2s',
                  opacity: isCreatingEvent ? 0.6 : 1,
                }}
                onMouseEnter={(e) => {
                  if (!isCreatingEvent) {
                    e.currentTarget.style.backgroundColor = '#4a4a4a';
                  }
                }}
                onMouseLeave={(e) => {
                  if (!isCreatingEvent) {
                    e.currentTarget.style.backgroundColor = '#3e3e42';
                  }
                }}
              >
                Create Sign
              </button>
              <button
                onClick={() => setIsCreateModalOpen(true)}
                disabled={isCreatingEvent}
                style={{
                  padding: '4px 8px',
                  border: '1px solid #3e3e42',
                  backgroundColor: isCreatingEvent ? '#2a2a2a' : '#3e3e42',
                  color: '#ffffff',
                  cursor: isCreatingEvent ? 'default' : 'pointer',
                  fontSize: '11px',
                  borderRadius: '4px',
                  transition: 'background-color 0.2s',
                  opacity: isCreatingEvent ? 0.6 : 1,
                }}
                onMouseEnter={(e) => {
                  if (!isCreatingEvent) {
                    e.currentTarget.style.backgroundColor = '#4a4a4a';
                  }
                }}
                onMouseLeave={(e) => {
                  if (!isCreatingEvent) {
                    e.currentTarget.style.backgroundColor = '#3e3e42';
                  }
                }}
              >
                Create Modal
              </button>
            </div>
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
              alignItems: 'flex-start',
              gap: '6px',
              minWidth: 0,
              maxWidth: '100%',
              boxSizing: 'border-box',
              padding: '6px 8px',
              backgroundColor: '#1e1e1e',
              borderRadius: '4px',
              border: '1px solid #3e3e42',
              marginBottom: '10px',
            }}
          >
            <div style={{ flex: '1 1 0', minWidth: 0 }}>
              {(() => {
                const event = gameEvents.find(
                  (e) => e.id === selectedTile.eventTrigger?.eventId
                );
                return (
                  <>
                    <div
                      style={{
                        color: '#ffffff',
                        fontSize: '12px',
                        overflowWrap: 'anywhere',
                      }}
                    >
                      {event?.title || selectedTile.eventTrigger.eventId}
                    </div>
                    {event && (
                      <>
                        <div
                          style={{
                            color: '#858585',
                            fontSize: '10px',
                            marginTop: '2px',
                            overflowWrap: 'anywhere',
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
                flexShrink: 0,
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
                  const eventId = selectedTile.eventTrigger?.eventId;
                  if (!eventId) {
                    updateTile((tile) => {
                      delete tile.eventTrigger;
                    });
                    return;
                  }
                  setPendingDeleteEventId(eventId);
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
              value={selectedTile.eventTrigger.requiresNonCombat ?? true}
              label="Requires Non Combat"
              onChange={(newValue) => {
                updateTile((tile) => {
                  if (tile.eventTrigger) {
                    tile.eventTrigger.requiresNonCombat = newValue;
                  }
                });
              }}
            />
            <OverrideCheckbox
              value={selectedTile.eventTrigger.requiresLook ?? false}
              label="Requires Look"
              onChange={(newValue) => {
                updateTile((tile) => {
                  if (tile.eventTrigger) {
                    tile.eventTrigger.requiresLook = newValue;
                  }
                });
              }}
            />
            <div>
              <div
                style={{
                  color: '#858585',
                  fontSize: '10px',
                  textTransform: 'uppercase',
                  fontWeight: 'bold',
                  marginBottom: '4px',
                }}
              >
                Condition
              </div>
              <ConditionEditButton
                condition={selectedTile.eventTrigger.condition ?? ''}
                onClick={() => setIsConditionModalOpen(true)}
              />
            </div>
            <div>
              <label
                style={{
                  color: '#858585',
                  fontSize: '10px',
                  textTransform: 'uppercase',
                  fontWeight: 'bold',
                  display: 'block',
                  marginBottom: '4px',
                }}
              >
                Tile Overlay Visibility
              </label>
              <select
                value={
                  selectedTile.eventTrigger.overlayVisibility ?? 'HIDDEN'
                }
                onChange={(e) => {
                  const value = e.target.value as TileOverlayVisibility;
                  updateTile((tile) => {
                    if (tile.eventTrigger) {
                      tile.eventTrigger.overlayVisibility = value;
                    }
                  });
                }}
                style={{
                  width: '100%',
                  padding: '4px 6px',
                  border: '1px solid #3e3e42',
                  backgroundColor: '#1e1e1e',
                  color: '#ffffff',
                  fontSize: '12px',
                  borderRadius: '4px',
                }}
              >
                {TILE_OVERLAY_VISIBILITY_OPTIONS.map((option) => (
                  <option key={option} value={option}>
                    {option}
                  </option>
                ))}
              </select>
            </div>
          </div>
        </>
      )}

      <CreateSignModal
        isOpen={isCreateSignOpen}
        mapName={mapName}
        existingEventIds={existingEventIds}
        onConfirm={handleCreateSign}
        onCancel={() => {
          if (!isCreatingEvent) {
            setIsCreateSignOpen(false);
          }
        }}
      />
      <CreateModalEventModal
        isOpen={isCreateModalOpen}
        mapName={mapName}
        existingEventIds={existingEventIds}
        defaultRequiresLook={!isMapTileWalkable(selectedTile, tilesets)}
        onConfirm={handleCreateModal}
        onCancel={() => {
          if (!isCreatingEvent) {
            setIsCreateModalOpen(false);
          }
        }}
      />
      <EditTileEventConditionModal
        isOpen={isConditionModalOpen}
        condition={selectedTile.eventTrigger?.condition ?? ''}
        gameEvent={
          gameEvents.find(
            (event) => event.id === selectedTile.eventTrigger?.eventId
          ) ?? null
        }
        onConfirm={(condition) => {
          updateTile((tile) => {
            if (!tile.eventTrigger) {
              return;
            }
            if (condition) {
              tile.eventTrigger.condition = condition;
            } else {
              delete tile.eventTrigger.condition;
            }
          });
          setIsConditionModalOpen(false);
        }}
        onCancel={() => setIsConditionModalOpen(false)}
      />
      <DeleteTileEventModal
        isOpen={pendingDeleteEventId !== null}
        eventId={pendingDeleteEventId ?? ''}
        eventTitle={pendingEvent?.title}
        eventExists={Boolean(pendingEvent)}
        otherUseNote={otherUseNote}
        isDeleting={isDeletingEvent}
        onRemoveTriggerOnly={() => {
          if (!isDeletingEvent) {
            removeTriggerFromTile();
          }
        }}
        onRemoveAndDeleteEvent={handleDeleteEventToo}
        onCancel={() => {
          if (!isDeletingEvent) {
            setPendingDeleteEventId(null);
          }
        }}
      />
    </div>
  );
}
