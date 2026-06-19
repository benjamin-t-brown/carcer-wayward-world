import { useState, useEffect, useLayoutEffect, useRef } from 'react';
import { CardListAdvanced } from '../components/CardList';
import { EditorSidebar } from '../components/EditorSidebar';
import { ListCardActions } from '../elements/ListCardActions';
import { GameEvent } from '../types/assets';
import { CreateGameEventModal } from '../components/CreateGameEventModal';
import { EditGameEventModal } from '../components/EditGameEventModal';
import { SpecialEventEditor } from '../special-event-editor/SpecialEventEditor';
import { VariableEditorModal } from '../special-event-editor/modals/VariableEditorModal';
import { JsonOutputModal } from '../special-event-editor/modals/JsonOutputModal';
import { Button } from '../elements/Button';
import { EditorHeader } from '../components/EditorHeader';
import { EditorEmptyState } from '../components/EditorEmptyState';
import { Notification } from '../elements/Notification';
import { useAssets } from '../contexts/AssetsContext';
import { trimStrings } from '../utils/jsonUtils';
import {
  centerPanzoomOnNode,
  getEditorState,
  saveEditorStateForGameEvent,
  syncGameEventFromEditorState,
  updateEditorStateNoReRender,
} from '../special-event-editor/seEditorState';
import { EventRunnerModal } from '../special-event-editor/eventRunner/EventRunnerModal';
import { DeleteModal } from '../elements/DeleteModal';
import { ValidationMenuButton } from '../special-event-editor/react-components/ValidationMenuButton';
import { useReRender } from '../hooks/useReRender';
import { EventRunner } from '../special-event-editor/eventRunner/EventRunner';
import { RecentLinks } from '../components/RecentLinks';
import {
  loadEditorSelection,
  resolveSelectionFromRoute,
  saveEditorSelection,
} from '../utils/editorSelectionStorage';

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
}

interface SpecialEventsProps {
  routeParams?: URLSearchParams;
}

const filterGameEvents = (
  gameEvents: GameEvent[],
  args: {
    searchTerm: string;
    eventTypes: ('MODAL' | 'TALK' | 'TRAVEL')[];
  },
) => {
  const { searchTerm, eventTypes } = args;
  // Filter game events based on search term
  const _filteredGameEvents = gameEvents
    .filter(
      (gameEvent) =>
        gameEvent.id.toLowerCase().includes(searchTerm.toLowerCase()) ||
        gameEvent.title.toLowerCase().includes(searchTerm.toLowerCase()) ||
        gameEvent.eventType.toLowerCase().includes(searchTerm.toLowerCase()),
    )
    .sort((a, b) => a.id.localeCompare(b.id));

  const modalEvents: GameEvent[] = [];
  const talkEvents: GameEvent[] = [];
  const travelEvents: GameEvent[] = [];
  for (let i = 0; i < _filteredGameEvents.length; i++) {
    const gameEvent = _filteredGameEvents[i];
    if (gameEvent.eventType === 'MODAL') {
      modalEvents.push(gameEvent);
    } else if (gameEvent.eventType === 'TALK') {
      talkEvents.push(gameEvent);
    } else if (gameEvent.eventType === 'TRAVEL') {
      travelEvents.push(gameEvent);
    }
  }

  const filteredGameEvents: GameEvent[] = [];

  if (eventTypes.includes('TALK')) {
    filteredGameEvents.push(...talkEvents);
  }
  if (eventTypes.includes('MODAL')) {
    filteredGameEvents.push(...modalEvents);
  }
  if (eventTypes.includes('TRAVEL')) {
    filteredGameEvents.push(...travelEvents);
  }
  return filteredGameEvents;
};

export function SpecialEvents({ routeParams }: SpecialEventsProps = {}) {
  const { gameEvents, setGameEvents, saveGameEvents } = useAssets();
  const [searchTerm, setSearchTerm] = useState('');
  const [notifications, setNotifications] = useState<NotificationState[]>([]);
  const [showCreateModal, setShowCreateModal] = useState(false);
  const [showVariableEditorModal, setShowVariableEditorModal] = useState(false);
  const [showJsonOutputModal, setShowJsonOutputModal] = useState(false);
  const [showEditGameEventModal, setShowEditGameEventModal] = useState(false);
  const [showEventRunnerModal, setShowEventRunnerModal] = useState(false);
  const [showDeleteConfirm, setShowDeleteConfirm] = useState(false);
  const [deleteConfirmMessage, setDeleteConfirmMessage] = useState('');
  const [deleteConfirmId, setDeleteConfirmId] = useState<string | null>(null);
  const [eventRunner, setEventRunner] = useState<EventRunner | undefined>(
    undefined,
  );
  const [findNodeQuery, setFindNodeQuery] = useState('');
  const [recentGameEvents, setRecentGameEvents] = useState<string[]>([]);
  const [eventsToShow, setEventsToShow] = useState<
    ('MODAL' | 'TALK' | 'TRAVEL')[]
  >(['MODAL', 'TALK', 'TRAVEL']);
  const reRender = useReRender();

  const notificationIdRef = useRef(0);
  const hasRestoredEventRef = useRef(false);

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
      (gameEvent) => gameEvent.id === prevGameEventId,
    );
    if (prevGameEvent) {
      syncGameEventFromEditorState(prevGameEvent);
    }
    updateEditorStateNoReRender({ gameEventId: gameEventId });
    reRender();
  };

  const addRecentGameEvent = (gameEventId: string) => {
    if (recentGameEvents.some((ge) => ge === gameEventId)) {
      return;
    }
    setRecentGameEvents([gameEventId, ...recentGameEvents]);
  };

  const removeRecentGameEvent = (gameEventId: string) => {
    setRecentGameEvents(recentGameEvents.filter((ge) => ge !== gameEventId));
  };

  const handleGameEventClick = (gameEventId: string) => {
    const currentEditorState = getEditorState();
    if (currentEditorState.gameEventId) {
      saveEditorStateForGameEvent(currentEditorState.gameEventId);
    }

    // Clear node selection when switching game events
    currentEditorState.selectedNodeIds.clear();
    currentEditorState.selectionRect = null;

    selectGameEvent(gameEventId);
    addRecentGameEvent(gameEventId);
    saveEditorSelection('specialEvents', gameEventId);
  };

  useLayoutEffect(() => {
    if (hasRestoredEventRef.current || gameEvents.length === 0) {
      return;
    }
    hasRestoredEventRef.current = true;

    const eventId =
      resolveSelectionFromRoute('specialEvents', routeParams) ??
      loadEditorSelection('specialEvents');
    if (eventId && gameEvents.some((event) => event.id === eventId)) {
      handleGameEventClick(eventId);
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [gameEvents, routeParams]);

  const handleClone = (gameEventId: string) => {
    const originalGameEvent = gameEvents.find(
      (gameEvent) => gameEvent.id === gameEventId,
    );

    if (!originalGameEvent) {
      return;
    }

    const clonedGameEvent: GameEvent = JSON.parse(
      JSON.stringify(originalGameEvent),
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
      (gameEvent) => gameEvent.id !== gameEventId,
    );

    // delete imports from other events
    for (const event of newGameEvents) {
      event.vars = event.vars.filter(
        (variable) => variable.importFrom !== gameEventId,
      );
    }
    setGameEvents(newGameEvents);
    if (getEditorState().gameEventId === gameEventId) {
      selectGameEvent('');
      saveEditorSelection('specialEvents', null);
    }
  };

  const handleDeleteClick = (gameEventId: string) => {
    const eventsThatImportThisEvent = gameEvents.filter((gameEvent) =>
      gameEvent.vars.some((variable) => variable.importFrom === gameEventId),
    );

    setShowDeleteConfirm(true);
    setDeleteConfirmId(gameEventId);
    setDeleteConfirmMessage(
      eventsThatImportThisEvent.length > 0
        ? `Are you sure you want to delete this game event? ${eventsThatImportThisEvent.length} events import this event.`
        : `Are you sure you want to delete this game event?`,
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
        'error',
      );
      return;
    }

    const newGameEvents = [...gameEvents, newGameEvent].sort((a, b) =>
      a.id.localeCompare(b.id),
    );
    setGameEvents(newGameEvents);
    selectGameEvent(newGameEvent.id);
    setShowCreateModal(false);
    setSearchTerm('');
    showNotification('Game event created!', 'success');
  };

  const handleCreateCancel = () => {
    setShowCreateModal(false);
  };

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
          `${gameEventIdentifier}: missing ${missingFields.join(', ')}`,
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
              ', ',
            )}] are missing ids`,
          );
        }
        if (duplicateChildIds.length > 0) {
          gameEventsWithChildErrors.push(
            `${gameEventIdentifier}: duplicate child ids found: ${duplicateChildIds.join(
              ', ',
            )}`,
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
          '\n',
        )}`,
      );
    }

    if (gameEventsWithChildErrors.length > 0) {
      errors.push(
        `Game events with child validation errors:\n${gameEventsWithChildErrors.join(
          '\n',
        )}`,
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
      (gameEvent) => gameEvent.id === currentEditorState.gameEventId,
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
        'error',
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

  // Autosave every 5 minutes
  useEffect(() => {
    const autosave = async () => {
      const currentEditorState = getEditorState();
      if (!currentEditorState || !currentEditorState.gameEventId) {
        return;
      }

      const currentGameEvent = gameEvents.find(
        (gameEvent) => gameEvent.id === currentEditorState.gameEventId,
      );

      if (currentGameEvent) {
        // Create a copy of the game event to avoid mutating the original
        const updatedGameEvent = { ...currentGameEvent };
        // Sync editor state to game event
        syncGameEventFromEditorState(updatedGameEvent, currentEditorState);

        // Update the gameEvents state
        const newGameEvents = [...gameEvents];
        const index = newGameEvents.findIndex(
          (gameEvent) => gameEvent.id === updatedGameEvent.id,
        );
        if (index > -1) {
          newGameEvents[index] = updatedGameEvent;
        }
        setGameEvents(newGameEvents);

        // Save to disk
        try {
          const trimmedGameEvents = trimStrings(newGameEvents);
          const sortedGameEvents = trimmedGameEvents.sort((a, b) =>
            a.id.localeCompare(b.id),
          );
          await saveGameEvents(sortedGameEvents);
          console.log('Autosaved game events');
        } catch (err) {
          console.error('Autosave failed:', err);
        }
      }
    };

    const intervalId = setInterval(autosave, 5 * 60 * 1000); // 5 minutes

    return () => {
      clearInterval(intervalId);
    };
  }, [gameEvents, setGameEvents, saveGameEvents]);

  const filteredGameEvents = filterGameEvents(gameEvents, {
    searchTerm,
    eventTypes: eventsToShow,
  });

  const currentGameEvent = gameEvents.find(
    (gameEvent) => gameEvent.id === getEditorState().gameEventId,
  );
  const selectedIndex = filteredGameEvents.findIndex(
    (gameEvent) => gameEvent.id === getEditorState().gameEventId,
  );

  return (
    <div className="container editor-page" style={{ maxWidth: '100000px' }}>
      <EditorHeader title="Special Events Editor" onSave={handleSaveAll} />

      <div className="editor-page-body">
        <div className="editor-content">
          <EditorSidebar
            createFirst
            createLabel="+ New Game Event"
            onCreate={handleCreateNew}
            afterCreate={
              <RecentLinks
                items={recentGameEvents
                  .slice(0, 10)
                  .map((gameEventId) => {
                    const ge = gameEvents.find((ge) => ge.id === gameEventId);
                    if (!ge) {
                      return undefined;
                    }
                    return {
                      label: ge.id + ' (' + ge.title + ')',
                      isSelected: ge.id === getEditorState().gameEventId,
                      onClick: () => handleGameEventClick(ge.id),
                      onRemove: () => removeRecentGameEvent(ge.id),
                    };
                  })
                  .filter((item) => item !== undefined)}
              />
            }
            searchTerm={searchTerm}
            onSearchChange={setSearchTerm}
            searchPlaceholder="Search game events..."
            afterSearch={
              <div className="event-type-filters">
                <input
                  id="show-modal-events"
                  type="checkbox"
                  checked={eventsToShow.includes('MODAL')}
                  onChange={(e) =>
                    setEventsToShow(
                      e.target.checked
                        ? ['MODAL', ...eventsToShow]
                        : eventsToShow.filter(
                            (eventType) => eventType !== 'MODAL',
                          ),
                    )
                  }
                />
                <label
                  className="event-type-filter-label"
                  htmlFor="show-modal-events"
                >
                  MODAL
                </label>
                <input
                  id="show-talk-events"
                  type="checkbox"
                  checked={eventsToShow.includes('TALK')}
                  onChange={(e) =>
                    setEventsToShow(
                      e.target.checked
                        ? ['TALK', ...eventsToShow]
                        : eventsToShow.filter(
                            (eventType) => eventType !== 'TALK',
                          ),
                    )
                  }
                />
                <label
                  className="event-type-filter-label"
                  htmlFor="show-talk-events"
                >
                  TALK
                </label>
                <input
                  id="show-travel-events"
                  type="checkbox"
                  checked={eventsToShow.includes('TRAVEL')}
                  onChange={(e) =>
                    setEventsToShow(
                      e.target.checked
                        ? ['TRAVEL', ...eventsToShow]
                        : eventsToShow.filter(
                            (eventType) => eventType !== 'TRAVEL',
                          ),
                    )
                  }
                />
                <label
                  className="event-type-filter-label"
                  htmlFor="show-travel-events"
                >
                  TRAVEL
                </label>
              </div>
            }
          >
            <CardListAdvanced
              items={filteredGameEvents}
              selectedIndex={selectedIndex > -1 ? selectedIndex : null}
              renderListItem={(gameEvent) => (
                <div className="special-event-list-card">
                  <div className="special-event-list-card-main">
                    <span
                      className="item-type special-event-type-badge"
                      style={{
                        background:
                          gameEvent.eventType === 'MODAL'
                            ? '#4e4980'
                            : gameEvent.eventType === 'TALK'
                              ? '#4a7c59'
                              : '#a1260d',
                      }}
                    >
                      {gameEvent.eventType}
                    </span>
                    <div className="item-id">{gameEvent.id}</div>
                  </div>
                  <div
                    className="item-card-actions"
                    onClick={(e) => e.stopPropagation()}
                  >
                    <ListCardActions
                      onClone={() => handleClone(gameEvent.id)}
                      onDelete={() => handleDeleteClick(gameEvent.id)}
                    />
                  </div>
                </div>
              )}
              onItemClick={(index) =>
                handleGameEventClick(filteredGameEvents[index].id)
              }
            />
          </EditorSidebar>

          <div className="editor-main special-events-editor-main">
            {currentGameEvent ? (
              <>
                <div className="special-events-subheader">
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
                    onClick={() => setShowJsonOutputModal(true)}
                  >
                    <span
                      role="img"
                      aria-label="JSON Output"
                      style={{ marginRight: '6px' }}
                    >
                      📄
                    </span>
                    JSON Output
                  </Button>
                  <Button
                    variant="small"
                    onClick={() => {
                      centerPanzoomOnNode(
                        document.getElementById(
                          'special-event-editor-canvas-canvas',
                        ) as HTMLCanvasElement,
                        findNodeQuery,
                        true,
                      );
                    }}
                  >
                    <span
                      role="img"
                      aria-label="Find Node"
                      style={{ marginRight: '6px', filter: 'sepia(1)' }}
                    >
                      🔍
                    </span>
                    Find Node
                    <input
                      type="text"
                      value={findNodeQuery}
                      onClick={(e) => e.stopPropagation()}
                      onChange={(e) => setFindNodeQuery(e.target.value)}
                      style={{
                        width: '75px',
                        marginLeft: '4px',
                      }}
                    />
                  </Button>
                  <Button
                    variant="small"
                    onClick={() => {
                      const currentEditorState = getEditorState();
                      syncGameEventFromEditorState(
                        currentGameEvent,
                        currentEditorState,
                      );
                      getEditorState().runnerErrors = [];
                      const gameEvent = gameEvents.find(
                        (gameEvent) =>
                          gameEvent.id === currentEditorState.gameEventId,
                      );
                      if (gameEvent) {
                        const runner = new EventRunner(
                          {},
                          gameEvent,
                          gameEvents,
                        );
                        (window as any).runner = runner;
                        setEventRunner(runner);
                        console.log('run root');
                        runner.advance(runner.currentNodeId, {
                          onceKeysToCommit: [],
                          execStr: '',
                        });
                        setShowEventRunnerModal(true);
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
                <div className="special-events-editor-content">
                  <SpecialEventEditor
                    gameEvent={currentGameEvent}
                    onUpdateGameEvent={(updatedGameEvent) => {
                      const newGameEvents = [...gameEvents];
                      const index = newGameEvents.findIndex(
                        (gameEvent) => gameEvent.id === updatedGameEvent.id,
                      );
                      if (index > -1) {
                        newGameEvents[index] = updatedGameEvent;
                      }
                      setGameEvents(newGameEvents);
                    }}
                  />
                </div>
                {showEventRunnerModal && eventRunner && (
                  <EventRunnerModal
                    isOpen={showEventRunnerModal}
                    eventRunner={eventRunner}
                    gameEvent={currentGameEvent}
                    gameEvents={gameEvents}
                    onCancel={() => {
                      setShowEventRunnerModal(false);
                      setEventRunner(undefined);
                    }}
                  />
                )}
              </>
            ) : (
              <EditorEmptyState message="Select a game event from the list to edit, or create a new one." />
            )}
          </div>
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
          onConfirm={(previousGameEvent, updatedGameEvent) => {
            const newGameEvents = [...gameEvents];
            const index = newGameEvents.findIndex(
              (gameEvent) => gameEvent.id === previousGameEvent.id,
            );
            if (index > -1) {
              newGameEvents[index] = updatedGameEvent;
            }
            setGameEvents(
              newGameEvents.sort((a, b) => a.id.localeCompare(b.id)),
            );
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
              (gameEvent) => gameEvent.id === updatedGameEvent.id,
            );
            if (index > -1) {
              newGameEvents[index] = updatedGameEvent;
            }
            setGameEvents(newGameEvents);
          }}
          onCancel={() => setShowVariableEditorModal(false)}
        />
      )}

      {/* JSON Output Modal */}
      {currentGameEvent && (
        <JsonOutputModal
          isOpen={showJsonOutputModal}
          gameEvent={currentGameEvent}
          onCancel={() => setShowJsonOutputModal(false)}
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
