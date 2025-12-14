import { useState, useEffect, useRef } from 'react';
import { CardList } from '../components/CardList';
import { GameEvent } from '../types/assets';
import { createDefaultGameEvent } from '../components/GameEventForm';
import { CreateGameEventModal } from '../components/CreateGameEventModal';
import { EditGameEventModal } from '../components/EditGameEventModal';
import { SpecialEventEditor } from '../special-event-editor/SpecialEventEditor';
import { VariableEditorModal } from '../special-event-editor/modals/VariableEditorModal';
import { Button } from '../elements/Button';
import { Notification } from '../elements/Notification';
import { useAssets } from '../contexts/AssetsContext';
import { trimStrings } from '../utils/jsonUtils';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { Sprite } from '../elements/Sprite';
import {
  getEditorState,
  saveEditorStateForGameEvent,
  syncGameEventFromEditorState,
  updateEditorState,
  updateEditorStateNoReRender,
} from '../special-event-editor/seEditorState';
import { EventRunnerModal } from '../special-event-editor/eventRunner/EventRunnerModal';
import { DeleteModal } from '../elements/DeleteModal';
import { ValidationMenuButton } from '../special-event-editor/react-components/ValidationMenuButton';
import { useReRender } from '../hooks/useReRender';
import { EventRunner } from '../special-event-editor/eventRunner/EventRunner';

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
}

interface SpecialEventsProps {
  routeParams?: URLSearchParams;
}

export function SpecialEvents({ routeParams }: SpecialEventsProps = {}) {
  const { spriteMap } = useSDL2WAssets();
  const { gameEvents, setGameEvents, saveGameEvents } = useAssets();
  // const [editGameEventIndex, setEditGameEventIndex] = useState<number>(-1);
  const [searchTerm, setSearchTerm] = useState('');
  const [notifications, setNotifications] = useState<NotificationState[]>([]);
  const [showCreateModal, setShowCreateModal] = useState(false);
  const [showVariableEditorModal, setShowVariableEditorModal] = useState(false);
  const [showEditGameEventModal, setShowEditGameEventModal] = useState(false);
  const [showEventRunnerModal, setShowEventRunnerModal] = useState(false);
  const [showDeleteConfirm, setShowDeleteConfirm] = useState(false);
  const [deleteConfirmMessage, setDeleteConfirmMessage] = useState('');
  const [deleteConfirmId, setDeleteConfirmId] = useState<string | null>(null);
  const [eventRunner, setEventRunner] = useState<EventRunner | undefined>(
    undefined
  );
  const reRender = useReRender();

  const notificationIdRef = useRef(0);

  const showNotification = (message: string, type: 'success' | 'error') => {
    const id = notificationIdRef.current++;
    setNotifications((prev) => [...prev, { message, type, id }]);
  };

  const removeNotification = (id: number) => {
    setNotifications((prev) => prev.filter((n) => n.id !== id));
  };

  const selectGameEvent = (gameEventId: string) => {
    const prevGameEventId = getEditorState().gameEventId;
    const prevGameEvent = gameEvents.find(
      (gameEvent) => gameEvent.id === prevGameEventId
    );
    if (prevGameEvent) {
      syncGameEventFromEditorState(prevGameEvent);
    }
    updateEditorStateNoReRender({ gameEventId: gameEventId });
    reRender();
  };

  // Filter game events based on search term
  const filteredGameEvents = gameEvents.filter(
    (gameEvent) =>
      gameEvent.id.toLowerCase().includes(searchTerm.toLowerCase()) ||
      gameEvent.title.toLowerCase().includes(searchTerm.toLowerCase()) ||
      gameEvent.eventType.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const handleGameEventClick = (gameEventId: string) => {
    const currentEditorState = getEditorState();
    if (currentEditorState.gameEventId) {
      saveEditorStateForGameEvent(currentEditorState.gameEventId);
    }

    // Clear node selection when switching game events
    currentEditorState.selectedNodeIds.clear();
    currentEditorState.selectionRect = null;

    selectGameEvent(gameEventId);
  };

  const handleClone = (gameEventId: string) => {
    const originalGameEvent = gameEvents.find(
      (gameEvent) => gameEvent.id === gameEventId
    );

    if (!originalGameEvent) {
      return;
    }

    const clonedGameEvent: GameEvent = JSON.parse(
      JSON.stringify(originalGameEvent)
    );
    clonedGameEvent.id = clonedGameEvent.id + '_copy';
    const newGameEvents = gameEvents.slice();
    newGameEvents.push(clonedGameEvent);
    setGameEvents(newGameEvents);
    selectGameEvent(clonedGameEvent.id);
    showNotification('Game event cloned!', 'success');
  };

  const deleteGameEvent = (gameEventId: string) => {
    const newGameEvents = gameEvents.filter(
      (gameEvent) => gameEvent.id !== gameEventId
    );

    // delete imports from other events
    for (const event of newGameEvents) {
      event.vars = event.vars.filter(
        (variable) => variable.importFrom !== gameEventId
      );
    }
    setGameEvents(newGameEvents);
    if (getEditorState().gameEventId === gameEventId) {
      selectGameEvent('');
    }
  };

  const handleDeleteClick = (gameEventId: string) => {
    const eventsThatImportThisEvent = gameEvents.filter((gameEvent) =>
      gameEvent.vars.some((variable) => variable.importFrom === gameEventId)
    );

    setShowDeleteConfirm(true);
    setDeleteConfirmId(gameEventId);
    setDeleteConfirmMessage(
      eventsThatImportThisEvent.length > 0
        ? `Are you sure you want to delete this game event? ${eventsThatImportThisEvent.length} events import this event.`
        : `Are you sure you want to delete this game event?`
    );
  };

  const handleCreateNew = () => {
    setShowCreateModal(true);
  };

  const handleCreateConfirm = (newGameEvent: GameEvent) => {
    // Check for duplicate IDs
    const duplicateId = gameEvents.find((ge) => ge.id === newGameEvent.id);
    if (duplicateId) {
      showNotification(
        `A game event with ID "${newGameEvent.id}" already exists. Please use a different ID.`,
        'error'
      );
      return;
    }

    const newGameEvents = [...gameEvents, newGameEvent];
    setGameEvents(newGameEvents);
    selectGameEvent(newGameEvent.id);
    setShowCreateModal(false);
    setSearchTerm('');
    showNotification('Game event created!', 'success');
  };

  const handleCreateCancel = () => {
    setShowCreateModal(false);
  };

  // // Check for event query parameter on mount
  // useEffect(() => {
  //   if (routeParams) {
  //     const eventId = routeParams.get('event');
  //     if (eventId) {
  //       const index = gameEvents.findIndex((e) => e.id === eventId);
  //       if (index >= 0) {
  //         setEditGameEventIndex(index);
  //       }
  //     }
  //   }
  //   // eslint-disable-next-line react-hooks/exhaustive-deps
  // }, [gameEvents, routeParams]);

  const validateGameEvents = (): { isValid: boolean; error?: string } => {
    const errors: string[] = [];
    const idCounts = new Map<string, number>();
    const gameEventsWithMissingFields: string[] = [];
    const gameEventsWithChildErrors: string[] = [];

    gameEvents.forEach((gameEvent, index) => {
      const missingFields: string[] = [];

      // Check required string fields
      if (!gameEvent.id || gameEvent.id.trim() === '') {
        missingFields.push('id');
      }
      if (!gameEvent.title || gameEvent.title.trim() === '') {
        missingFields.push('title');
      }
      if (!gameEvent.eventType || gameEvent.eventType.trim() === '') {
        missingFields.push('eventType');
      }
      if (!gameEvent.icon || gameEvent.icon.trim() === '') {
        missingFields.push('icon');
      }

      if (missingFields.length > 0) {
        const gameEventIdentifier =
          gameEvent.id || `Game event at index ${index}`;
        gameEventsWithMissingFields.push(
          `${gameEventIdentifier}: missing ${missingFields.join(', ')}`
        );
      }

      // Track ids for duplicate checking
      if (gameEvent.id && gameEvent.id.trim()) {
        const count = idCounts.get(gameEvent.id) || 0;
        idCounts.set(gameEvent.id, count + 1);
      }

      // Validate children
      if (gameEvent.children && gameEvent.children.length > 0) {
        const childIdCounts = new Map<string, number>();
        const childrenWithoutIds: number[] = [];
        const duplicateChildIds: string[] = [];

        gameEvent.children.forEach((child: any, childIndex: number) => {
          if (!child.id || child.id.trim() === '') {
            childrenWithoutIds.push(childIndex);
          } else {
            const count = childIdCounts.get(child.id) || 0;
            childIdCounts.set(child.id, count + 1);
          }
        });

        // Check for duplicate child ids
        childIdCounts.forEach((count, id) => {
          if (count > 1) {
            duplicateChildIds.push(id);
          }
        });

        const gameEventIdentifier =
          gameEvent.id || `Game event at index ${index}`;
        if (childrenWithoutIds.length > 0) {
          gameEventsWithChildErrors.push(
            `${gameEventIdentifier}: children at indices [${childrenWithoutIds.join(
              ', '
            )}] are missing ids`
          );
        }
        if (duplicateChildIds.length > 0) {
          gameEventsWithChildErrors.push(
            `${gameEventIdentifier}: duplicate child ids found: ${duplicateChildIds.join(
              ', '
            )}`
          );
        }
      }
    });

    // Check for duplicate ids
    const duplicateIds: string[] = [];
    idCounts.forEach((count, id) => {
      if (count > 1) {
        duplicateIds.push(id);
      }
    });

    if (duplicateIds.length > 0) {
      errors.push(`Duplicate game event ids found: ${duplicateIds.join(', ')}`);
    }

    if (gameEventsWithMissingFields.length > 0) {
      errors.push(
        `Game events with missing required fields:\n${gameEventsWithMissingFields.join(
          '\n'
        )}`
      );
    }

    if (gameEventsWithChildErrors.length > 0) {
      errors.push(
        `Game events with child validation errors:\n${gameEventsWithChildErrors.join(
          '\n'
        )}`
      );
    }

    if (errors.length > 0) {
      return {
        isValid: false,
        error: errors.join('\n\n'),
      };
    }

    return { isValid: true };
  };

  const handleSaveAll = async () => {
    const validation = validateGameEvents();
    if (!validation.isValid) {
      showNotification(validation.error || 'Validation failed', 'error');
      return;
    }

    const currentEditorState = getEditorState();
    const currentGameEvent = gameEvents.find(
      (gameEvent) => gameEvent.id === currentEditorState.gameEventId
    );
    if (currentGameEvent) {
      syncGameEventFromEditorState(currentGameEvent, currentEditorState);
    }

    const trimmedGameEvents = trimStrings(gameEvents);

    const sortedGameEvents = trimmedGameEvents.sort((a, b) => {
      return a.id.localeCompare(b.id);
    });

    console.log('save game events', currentGameEvent, sortedGameEvents);

    try {
      await saveGameEvents(sortedGameEvents);
      showNotification('Game events saved successfully!', 'success');
    } catch (err) {
      showNotification(
        `Error saving: ${err instanceof Error ? err.message : 'Unknown error'}`,
        'error'
      );
    }
  };

  // Global hotkey: Ctrl+S to save
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      // Check for Ctrl+S (Windows/Linux) or Cmd+S (Mac)
      if ((e.ctrlKey || e.metaKey) && e.key === 's') {
        e.preventDefault();
        handleSaveAll();
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => {
      window.removeEventListener('keydown', handleKeyDown);
    };
  }); // Include dependencies

  const currentGameEvent = gameEvents.find(
    (gameEvent) => gameEvent.id === getEditorState().gameEventId
  );
  const selectedIndex = filteredGameEvents.findIndex(
    (gameEvent) => gameEvent.id === getEditorState().gameEventId
  );

  return (
    <div className="container">
      <div className="editor-header">
        <Button variant="back" onClick={() => (window.location.hash = '#/')}>
          ← Back
        </Button>
        <h1>Special Events Editor</h1>
        <Button variant="primary" onClick={handleSaveAll}>
          Save All
        </Button>
      </div>

      <div className="editor-content">
        <div className="editor-sidebar">
          <div className="search-box">
            <input
              type="text"
              value={searchTerm}
              onChange={(e) => setSearchTerm(e.target.value)}
              placeholder="Search game events..."
            />
          </div>
          <div className="item-actions">
            <Button variant="primary" onClick={handleCreateNew}>
              + New Game Event
            </Button>
          </div>
          <CardList
            items={filteredGameEvents.map((ge) => ({
              ...ge,
              name: ge.id,
              label: ge.title,
            }))}
            onItemClick={(filteredIndex) =>
              handleGameEventClick(filteredGameEvents[filteredIndex].id)
            }
            onClone={(filteredIndex) =>
              handleClone(filteredGameEvents[filteredIndex].id)
            }
            onDelete={(filteredIndex) =>
              handleDeleteClick(filteredGameEvents[filteredIndex].id)
            }
            selectedIndex={selectedIndex > -1 ? selectedIndex : null}
            renderAdditionalInfo={(item) => {
              const gameEvent = item as unknown as GameEvent;
              const sprite = spriteMap[gameEvent.icon];
              return (
                <>
                  <div
                    className="item-info"
                    style={{
                      display: 'flex',
                      alignItems: 'center',
                      gap: '8px',
                    }}
                  >
                    {sprite && (
                      <div style={{ display: 'inline-block' }}>
                        <Sprite sprite={sprite} scale={0.5} />
                      </div>
                    )}
                    <span className="item-type">{gameEvent.eventType}</span>
                  </div>
                </>
              );
            }}
            emptyMessage="No game events found"
          />
        </div>

        <div className="editor-main" style={{ position: 'relative' }}>
          {currentGameEvent ? (
            <>
              <SpecialEventEditor
                gameEvent={currentGameEvent}
                onUpdateGameEvent={(updatedGameEvent) => {
                  const newGameEvents = [...gameEvents];
                  const index = newGameEvents.findIndex(
                    (gameEvent) => gameEvent.id === updatedGameEvent.id
                  );
                  if (index > -1) {
                    newGameEvents[index] = updatedGameEvent;
                  }
                  setGameEvents(newGameEvents);
                }}
              />
              <div
                style={{
                  position: 'absolute',
                  top: '4px',
                  left: '4px',
                  display: 'flex',
                  alignItems: 'center',
                  gap: '10px',
                }}
              >
                <Button
                  variant="small"
                  onClick={() => setShowEditGameEventModal(true)}
                >
                  <span
                    role="img"
                    aria-label="Edit Game Event"
                    style={{ marginRight: '6px' }}
                  >
                    ⚙️
                  </span>
                  Edit Game Event
                </Button>
                <Button
                  variant="small"
                  onClick={() => setShowVariableEditorModal(true)}
                >
                  📝 Edit Variables
                </Button>
                <Button
                  variant="small"
                  onClick={() => {
                    const currentEditorState = getEditorState();
                    syncGameEventFromEditorState(
                      currentGameEvent,
                      currentEditorState
                    );
                    getEditorState().runnerErrors = [];
                    const gameEvent = gameEvents.find(
                      (gameEvent) =>
                        gameEvent.id === currentEditorState.gameEventId
                    );
                    if (gameEvent) {
                      const runner = new EventRunner({}, gameEvent, gameEvents);
                      setEventRunner(runner);
                      console.log('run root');
                      runner.advance(runner.currentNodeId, {
                        onceKeysToCommit: [],
                        execStr: '',
                      });
                      setTimeout(() => {
                        setShowEventRunnerModal(true);
                      }, 100);
                    }
                  }}
                >
                  <span
                    role="img"
                    aria-label="Play"
                    style={{ marginRight: '6px' }}
                  >
                    ▶️
                  </span>
                  Run Event
                </Button>
                <ValidationMenuButton currentGameEvent={currentGameEvent} />
              </div>
            </>
          ) : (
            <div
              style={{
                color: '#858585',
                fontSize: '14px',
                textAlign: 'center',
                marginTop: '50px',
              }}
            >
              Select a game event from the list to edit, or create a new one.
            </div>
          )}
        </div>
      </div>

      {/* Create Modal */}
      {showCreateModal && (
        <CreateGameEventModal
          isOpen={showCreateModal}
          onConfirm={handleCreateConfirm}
          onCancel={handleCreateCancel}
        />
      )}

      {/* Edit Game Event Modal */}
      {currentGameEvent && (
        <EditGameEventModal
          isOpen={showEditGameEventModal}
          gameEvent={currentGameEvent}
          onConfirm={(updatedGameEvent) => {
            const newGameEvents = [...gameEvents];
            const index = newGameEvents.findIndex(
              (gameEvent) => gameEvent.id === updatedGameEvent.id
            );
            if (index > -1) {
              newGameEvents[index] = updatedGameEvent;
            }
            setGameEvents(newGameEvents);
            setShowEditGameEventModal(false);
          }}
          onCancel={() => setShowEditGameEventModal(false)}
        />
      )}

      {/* Variable Editor Modal */}
      {currentGameEvent && (
        <VariableEditorModal
          isOpen={showVariableEditorModal}
          gameEvent={currentGameEvent}
          onConfirm={(updatedGameEvent) => {
            const newGameEvents = [...gameEvents];
            const index = newGameEvents.findIndex(
              (gameEvent) => gameEvent.id === updatedGameEvent.id
            );
            if (index > -1) {
              newGameEvents[index] = updatedGameEvent;
            }
            setGameEvents(newGameEvents);
          }}
          onCancel={() => setShowVariableEditorModal(false)}
        />
      )}

      {/* Event Runner Modal */}
      {showEventRunnerModal && currentGameEvent && (
        <EventRunnerModal
          isOpen={showEventRunnerModal}
          eventRunner={eventRunner}
          gameEvent={currentGameEvent || null}
          gameEvents={gameEvents}
          onCancel={() => {
            setShowEventRunnerModal(false);
            setEventRunner(undefined);
          }}
        />
      )}

      {/* Notifications */}
      {notifications.map((notification) => (
        <Notification
          key={notification.id}
          message={notification.message}
          type={notification.type}
          onClose={() => removeNotification(notification.id)}
        />
      ))}

      <DeleteModal
        isOpen={showDeleteConfirm}
        message={deleteConfirmMessage}
        onConfirm={() => {
          deleteGameEvent(deleteConfirmId ?? '');
          setShowDeleteConfirm(false);
        }}
        onCancel={() => setShowDeleteConfirm(false)}
      />
    </div>
  );
}
