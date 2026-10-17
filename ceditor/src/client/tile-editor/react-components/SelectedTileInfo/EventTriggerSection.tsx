import { useMemo, useState } from 'react';
import {
  CarcerMapTileTemplate,
  GameEvent,
  TileOverlayVisibility,
  TilesetTemplate,
} from '../../../types/assets';
import { useAssets } from '../../../contexts/AssetsContext';
import { isMapTileWalkable } from '../../mapTileWalkability';
import { EventSearchInput } from './EventSearchInput';
import { CreateSignModal, CreateSignModalResult } from './CreateSignModal';
import {
  CreateModalEventModal,
  CreateModalEventResult,
} from './CreateModalEventModal';
import { createSignGameEvent } from './createSignGameEvent';
import { createModalGameEvent } from './createModalGameEvent';
import { DeleteTileEventModal } from './DeleteTileEventModal';
import {
  EditEventTriggerModal,
  EditEventTriggerResult,
} from './EditEventTriggerModal';
import { findGameEventReferences } from '../../../utils/gameEventReferences';

const DEFAULT_TRIGGER_ACTIVATION_SOUND = 'event';

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
  const [isEditTriggerOpen, setIsEditTriggerOpen] = useState(false);
  const [isCreatingEvent, setIsCreatingEvent] = useState(false);
  const [isSavingTrigger, setIsSavingTrigger] = useState(false);
  const [pendingDeleteEventId, setPendingDeleteEventId] = useState<string | null>(
    null
  );
  const [isDeletingEvent, setIsDeletingEvent] = useState(false);

  const existingEventIds = useMemo(
    () => new Set(gameEvents.map((event) => event.id)),
    [gameEvents]
  );

  const assignedEvent = useMemo(
    () =>
      gameEvents.find(
        (event) => event.id === selectedTile.eventTrigger?.eventId
      ),
    [gameEvents, selectedTile.eventTrigger?.eventId]
  );

  const assignEventToTile = (
    eventId: string,
    options?: {
      forceRequiresLook?: boolean;
      requiresNonCombat?: boolean;
      requiresLook?: boolean;
      requiresUse?: boolean;
      overlayVisibility?: TileOverlayVisibility;
      condition?: string;
      activationSound?: string;
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
        requiresUse: options?.requiresUse ?? false,
        overlayVisibility: options?.overlayVisibility ?? 'HIDDEN',
        ...(condition ? { condition } : {}),
        activationSound:
          options?.activationSound ?? DEFAULT_TRIGGER_ACTIVATION_SOUND,
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
      assignEventToTile(result.triggerId, {
        forceRequiresLook: true,
        activationSound: '',
      });
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

    const newEvent = createModalGameEvent(
      result.triggerId,
      result.text,
      result.title
    );
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
        requiresUse: result.requiresUse,
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
    setIsEditTriggerOpen(false);
  };

  const handleSaveTrigger = async (result: EditEventTriggerResult) => {
    if (!selectedTile.eventTrigger || isSavingTrigger) {
      return;
    }

    setIsSavingTrigger(true);
    try {
      if (assignedEvent && result.title !== assignedEvent.title) {
        const nextEvents = gameEvents.map((event) =>
          event.id === assignedEvent.id
            ? { ...event, title: result.title }
            : event
        );
        const savedEvents = await saveGameEvents(nextEvents);
        setGameEvents(savedEvents);
      }

      updateTile((tile) => {
        if (!tile.eventTrigger) {
          return;
        }
        tile.eventTrigger.requiresNonCombat = result.requiresNonCombat;
        tile.eventTrigger.requiresLook = result.requiresLook;
        tile.eventTrigger.requiresUse = result.requiresUse;
        tile.eventTrigger.activationSound = result.activationSound;
        tile.eventTrigger.overlayVisibility = result.overlayVisibility;
        if (result.condition) {
          tile.eventTrigger.condition = result.condition;
        } else {
          delete tile.eventTrigger.condition;
        }
      });
      setIsEditTriggerOpen(false);
    } catch (err) {
      console.error('Failed to save event trigger:', err);
      alert(
        `Failed to save event trigger: ${
          err instanceof Error ? err.message : 'Unknown error'
        }`
      );
    } finally {
      setIsSavingTrigger(false);
    }
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

  const openSpecialEventsEditor = () => {
    const eventId = selectedTile.eventTrigger?.eventId;
    if (!eventId) {
      return;
    }
    const url = `${window.location.origin}${
      window.location.pathname
    }#/editor/specialEvents?event=${encodeURIComponent(eventId)}`;
    window.open(url, '_blank');
  };

  const triggerDisplayName =
    assignedEvent?.title ||
    selectedTile.eventTrigger?.eventId ||
    'Event trigger';

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
        <div
          style={{
            minWidth: 0,
            maxWidth: '100%',
            boxSizing: 'border-box',
            padding: '6px 8px',
            backgroundColor: '#1e1e1e',
            borderRadius: '4px',
            border: '1px solid #3e3e42',
          }}
        >
          <div
            style={{
              display: 'flex',
              justifyContent: 'space-between',
              alignItems: 'center',
              gap: '6px',
              minWidth: 0,
            }}
          >
            <div style={{ flex: '1 1 0', minWidth: 0 }}>
              <div
                style={{
                  color: '#ffffff',
                  fontSize: '12px',
                  overflowWrap: 'anywhere',
                }}
                title={selectedTile.eventTrigger.eventId}
              >
                {triggerDisplayName}
              </div>
              {!assignedEvent && (
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
              {assignedEvent && assignedEvent.eventType !== 'MODAL' && (
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
            </div>
            <div
              style={{
                display: 'flex',
                gap: '4px',
                alignItems: 'center',
                flexShrink: 0,
              }}
            >
              <button
                onClick={(e) => {
                  e.stopPropagation();
                  openSpecialEventsEditor();
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
                onClick={() => setIsEditTriggerOpen(true)}
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
                title="Edit trigger settings"
              >
                Edit
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
          {selectedTile.eventTrigger.condition?.trim() ? (
            <div
              style={{
                color: '#858585',
                fontSize: '10px',
                marginTop: '6px',
                paddingTop: '6px',
                borderTop: '1px solid #3e3e42',
                overflowWrap: 'anywhere',
                width: '100%',
              }}
              title={selectedTile.eventTrigger.condition}
            >
              {selectedTile.eventTrigger.condition}
            </div>
          ) : null}
        </div>
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
      {selectedTile.eventTrigger && (
        <EditEventTriggerModal
          isOpen={isEditTriggerOpen}
          trigger={selectedTile.eventTrigger}
          eventTitle={assignedEvent?.title ?? selectedTile.eventTrigger.eventId}
          gameEvent={assignedEvent ?? null}
          isSaving={isSavingTrigger}
          onSave={handleSaveTrigger}
          onCancel={() => {
            if (!isSavingTrigger) {
              setIsEditTriggerOpen(false);
            }
          }}
        />
      )}
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
