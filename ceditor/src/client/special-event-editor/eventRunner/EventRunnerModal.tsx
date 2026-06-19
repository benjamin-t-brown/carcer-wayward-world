import { useEffect, useRef } from 'react';
import {
  GameEvent,
  GameEventChildExec,
  GameEventChildType,
} from '../../types/assets';
import { Sprite } from '../../elements/Sprite';
import { useSDL2WAssets } from '../../contexts/SDL2WAssetsContext';
import {
  centerPanzoomOnNode,
  getEditorState,
  notifyStateUpdated,
} from '../seEditorState';
import { EventRunner, EventRunnerLogEntry } from './EventRunner';
import { useReRender } from '../../hooks/useReRender';
import { CANVAS_CONTAINER_ID } from '../react-components/MapCanvasSE';

interface EventRunnerModalProps {
  isOpen: boolean;
  gameEvent: GameEvent;
  gameEvents: GameEvent[];
  onCancel: () => void;
  eventRunner?: EventRunner;
}

const EVENT_FONT_SIZE = '18px';
const PANEL_MIN_HEIGHT = 500;
const PANEL_VERTICAL_PADDING = 24; // matches .special-events-runner-panel padding (12px * 2)
const HEADER_HEIGHT = 80;
const BODY_HEIGHT =
  PANEL_MIN_HEIGHT - PANEL_VERTICAL_PADDING - HEADER_HEIGHT;
const CHOICES_WIDTH = 500;
const BODY_GAP = 8;
const PANEL_CONTENT_HEIGHT = HEADER_HEIGHT + BODY_HEIGHT;

function getPriorChoiceKeys(entries: EventRunnerLogEntry[]) {
  return new Set(
    entries
      .filter((entry) => entry.type === 'choice' && entry.choiceKey)
      .map((entry) => entry.choiceKey as string)
  );
}

function getPlainChoiceLabel(text: string) {
  return text.replace(/<[^>]*>/g, '').trim();
}

function findLastSegmentIndex(entries: EventRunnerLogEntry[]) {
  for (let i = entries.length - 1; i >= 0; i--) {
    if (entries[i].type === 'choice' || entries[i].type === 'continue') {
      return i;
    }
  }
  return -1;
}

function getLogEntryStyle(
  entry: EventRunnerLogEntry,
  index: number,
  lastSegmentIndex: number
) {
  const base = {
    whiteSpace: 'pre-wrap' as const,
    marginTop: index === 0 ? 0 : '1em',
    cursor: entry.nodeId ? ('pointer' as const) : undefined,
  };
  const isHistoricalText =
    entry.type === 'text' &&
    lastSegmentIndex >= 0 &&
    index <= lastSegmentIndex;
  if (entry.type === 'choice' || isHistoricalText) {
    return {
      ...base,
      color: '#888',
      fontFamily: 'arial',
      fontSize: EVENT_FONT_SIZE,
    };
  }
  if (entry.type === 'storage') {
    return {
      ...base,
      color: '#aaa',
      fontFamily: 'monospace',
      fontSize: '14px',
    };
  }
  return {
    ...base,
    color: 'white',
    fontFamily: 'arial',
    fontSize: EVENT_FONT_SIZE,
  };
}

const EventHeader = ({ gameEvent }: { gameEvent: GameEvent }) => {
  const { spriteMap } = useSDL2WAssets();
  const sprite = spriteMap[gameEvent.icon ?? ''];
  return (
    <div
      style={{
        background: '#777',
        width: '100%',
        height: '80px',
        display: 'flex',
        alignItems: 'center',
        flexShrink: 0,
      }}
    >
      <div style={{ width: '8px', height: '2px' }}></div>
      <Sprite sprite={sprite} maxWidth={100} />
      <div style={{ width: '10px', height: '2px' }}></div>
      <span>{gameEvent.title}</span>
    </div>
  );
};

const ChoiceButton = ({
  hasErrors,
  handleNext,
  text,
  dimmed = false,
}: {
  hasErrors: boolean;
  handleNext: () => void;
  text: string;
  dimmed?: boolean;
}) => {
  return (
    <button
      style={{
        background: 'black',
        padding: '10px 10px',
        fontFamily: 'arial',
        fontSize: EVENT_FONT_SIZE,
        color: dimmed ? '#888' : 'white',
        border: 'none',
        cursor: hasErrors ? 'not-allowed' : 'pointer',
        width: '100%',
        textAlign: 'left',
        marginBottom: '4px',
        borderRadius: '8px',
        opacity: hasErrors ? 0.5 : 1,
      }}
      onClick={() => handleNext()}
      disabled={hasErrors}
    >
      {getPlainChoiceLabel(text)}
    </button>
  );
};

export function EventRunnerModal({
  isOpen,
  gameEvent,
  onCancel,
  eventRunner,
}: EventRunnerModalProps) {
  const reRender = useReRender();
  const logRef = useRef<HTMLDivElement>(null);
  const pauseState = eventRunner?.getPauseState();

  const panToNode = (nodeId: string) => {
    if (!nodeId) {
      return;
    }
    const canvas = document.getElementById(
      CANVAS_CONTAINER_ID + '-canvas'
    ) as HTMLCanvasElement;
    if (canvas) {
      centerPanzoomOnNode(canvas, nodeId);
    }
  };

  const panToCurrentNode = () => {
    if (!eventRunner) {
      return;
    }
    panToNode(eventRunner.currentNodeId);
  };

  const advance = (
    nextNodeId: string,
    {
      onceKeysToCommit,
      execStr,
    }: { onceKeysToCommit: string[]; execStr: string } = {
      onceKeysToCommit: [],
      execStr: '',
    }
  ) => {
    if (nextNodeId === '') {
      onCancel();
      return;
    }
    if (eventRunner) {
      try {
        eventRunner.advance(nextNodeId, { onceKeysToCommit, execStr });
        panToCurrentNode();
      } catch (e) {
        console.error('error advancing event runner:', e);
        getEditorState().runnerErrors = eventRunner.errors;
        notifyStateUpdated();
      }
      reRender();
    }
  };

  const handleContinue = () => {
    const node = eventRunner?.getCurrentNode();
    if (node?.eventChildType === GameEventChildType.EXEC) {
      eventRunner?.recordContinue();
      advance((node as GameEventChildExec).next, {
        onceKeysToCommit: [],
        execStr: '',
      });
    }
  };

  const handleChoice = (index: number) => {
    const choice = eventRunner?.displayTextChoices[index];
    if (choice && eventRunner) {
      eventRunner.recordChoiceSelection(index);
      advance(choice.next, {
        onceKeysToCommit: choice.onceKeysToCommit,
        execStr: choice.execStr,
      });
    }
  };

  const handleClose = () => {
    if (eventRunner) {
      getEditorState().runnerErrors = eventRunner.errors;
      notifyStateUpdated();
    }
    onCancel();
  };

  useEffect(() => {
    const log = logRef.current;
    if (log) {
      log.scrollTo({ top: log.scrollHeight, behavior: 'smooth' });
    }
  }, [eventRunner?.logEntries.length, pauseState]);

  useEffect(() => {
    if (isOpen && eventRunner) {
      panToCurrentNode();
    }
  }, [isOpen, eventRunner]);

  useEffect(() => {
    if (!isOpen || !eventRunner || pauseState === 'done') {
      return;
    }

    const handleKeyDown = (event: KeyboardEvent) => {
      if (event.key !== 'Escape') {
        return;
      }
      event.preventDefault();
      handleClose();
    };

    window.addEventListener('keydown', handleKeyDown, true);
    return () => window.removeEventListener('keydown', handleKeyDown, true);
  }, [isOpen, eventRunner, pauseState]);

  if (!isOpen || !eventRunner || pauseState === 'done') {
    return undefined;
  }

  const hasErrors = eventRunner.errors.length > 0;
  const showChoicesArea = pauseState === 'continue' || pauseState === 'choice';
  const lastSegmentIndex = findLastSegmentIndex(eventRunner.logEntries);
  const priorChoiceKeys = getPriorChoiceKeys(eventRunner.logEntries);

  return (
    <div className="special-events-runner-panel">
      <button
        style={{
          position: 'absolute',
          top: 4,
          right: 8,
          backgroundColor: 'transparent',
          border: 'none',
          color: '#d4d4d4',
          fontSize: '24px',
          fontWeight: 'bold',
          cursor: 'pointer',
          zIndex: 1,
          background: '#990707',
          borderRadius: '8px',
          padding: '4px 8px',
        }}
        onClick={handleClose}
        title="Close runner (Esc)"
      >
        <span>×</span>
      </button>

      <div
        style={{
          display: 'flex',
          flexDirection: 'column',
          width: '100%',
          height: `${PANEL_CONTENT_HEIGHT}px`,
        }}
      >
        <EventHeader gameEvent={gameEvent} />

        <div
          style={{
            display: 'flex',
            flexDirection: 'row',
            gap: `${BODY_GAP}px`,
            height: `${BODY_HEIGHT}px`,
            minHeight: 0,
          }}
        >
          <div
            ref={logRef}
            style={{
              flex: 1,
              minWidth: 0,
              background: '#1e1e1e',
              padding: '10px',
              overflow: 'auto',
              scrollBehavior: 'smooth',
            }}
          >
            {eventRunner.logEntries.map((entry, i) =>
              entry.type === 'continue' ? null : (
                <div
                  key={i}
                  style={getLogEntryStyle(entry, i, lastSegmentIndex)}
                  title={entry.nodeId ? `Go to node ${entry.nodeId}` : undefined}
                  onClick={() => {
                    if (entry.nodeId) {
                      panToNode(entry.nodeId);
                    }
                  }}
                >
                  {entry.type === 'choice' ? ` - ${entry.text}` : entry.text}
                </div>
              )
            )}
          </div>

          {showChoicesArea && (
            <div
              style={{
                width: `${CHOICES_WIDTH}px`,
                flexShrink: 0,
                overflow: 'auto',
              }}
            >
              <div style={{ marginBottom: '8px' }}></div>
              {pauseState === 'continue' && (
                <ChoiceButton
                  hasErrors={hasErrors}
                  handleNext={handleContinue}
                  text="Continue."
                />
              )}

              {pauseState === 'choice' &&
                eventRunner.displayTextChoices.map((choice, i) => {
                  const label =
                    (choice.prefix ? choice.prefix + ' ' : '') + choice.text;
                  return (
                    <ChoiceButton
                      key={`active-choice-${i}`}
                      hasErrors={hasErrors}
                      handleNext={() => handleChoice(i)}
                      text={label}
                      dimmed={priorChoiceKeys.has(choice.choiceKey)}
                    />
                  );
                })}
            </div>
          )}
        </div>

        {hasErrors && (
          <div style={{ flexShrink: 0, marginTop: '8px' }}>
            {eventRunner.errors.map((error, i) => (
              <div key={error.nodeId + i.toString()}>
                <span
                  style={{
                    color: '#FFF',
                    cursor: 'pointer',
                    textDecoration: 'underline',
                  }}
                  onClick={() => {
                    if (error.nodeId) {
                      getEditorState().runnerErrors = eventRunner.errors;
                      notifyStateUpdated();
                      panToNode(error.nodeId);
                    }
                  }}
                >
                  {error.nodeId}:
                </span>{' '}
                <span style={{ color: '#F77' }}>{error.message}</span>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
}
