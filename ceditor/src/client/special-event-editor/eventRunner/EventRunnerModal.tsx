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
import { MODAL_ROOT_CLASS, useEscapeToClose } from '../../hooks/useEscapeToClose';

interface EventRunnerModalProps {
  isOpen: boolean;
  gameEvent: GameEvent;
  gameEvents: GameEvent[];
  onCancel: () => void;
  eventRunner?: EventRunner;
}

const EVENT_FONT_SIZE = '18px';
const HEADER_HEIGHT = 80;
const TEXT_LOG_HEIGHT = 260;
const CHOICES_AREA_HEIGHT = 150;
const CHOICES_GAP = 8;
const MODAL_CONTENT_HEIGHT =
  HEADER_HEIGHT + TEXT_LOG_HEIGHT + CHOICES_GAP + CHOICES_AREA_HEIGHT;

function getLogEntryStyle(entry: EventRunnerLogEntry, index: number) {
  const base = {
    whiteSpace: 'pre-wrap' as const,
    marginTop: index === 0 ? 0 : '1em',
  };
  if (entry.type === 'choice') {
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
      <div style={{ width: '2px', height: '2px' }}></div>
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
}: {
  hasErrors: boolean;
  handleNext: () => void;
  text: string;
}) => {
  return (
    <button
      style={{
        background: 'black',
        padding: '10px 10px',
        fontFamily: 'arial',
        fontSize: EVENT_FONT_SIZE,
        color: 'white',
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
      {text}
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

  const panToCurrentNode = () => {
    if (!eventRunner) {
      return;
    }
    const canvas = document.getElementById(
      'special-event-editor-canvas-canvas'
    ) as HTMLCanvasElement;
    if (canvas) {
      centerPanzoomOnNode(canvas, eventRunner.currentNodeId);
    }
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

  const modalRef = useEscapeToClose(
    handleClose,
    isOpen && !!eventRunner && pauseState !== 'done'
  );

  if (!isOpen || !eventRunner || pauseState === 'done') {
    return undefined;
  }

  const hasErrors = eventRunner.errors.length > 0;
  const isEnded = pauseState === 'end';
  const showChoicesArea = pauseState === 'continue' || pauseState === 'choice';
  const textLogHeight = isEnded
    ? MODAL_CONTENT_HEIGHT - HEADER_HEIGHT
    : TEXT_LOG_HEIGHT;

  return (
    <div
      ref={modalRef}
      className={MODAL_ROOT_CLASS}
      style={{
        position: 'fixed',
        top: 0,
        left: 0,
        right: 0,
        bottom: 0,
        backgroundColor: 'rgba(0, 0, 0, 0.7)',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        zIndex: 1001,
      }}
    >
      <div
        style={{
          backgroundColor: '#252526',
          border: '1px solid #3e3e42',
          borderRadius: '8px',
          padding: '30px',
          maxWidth: '50vw',
          width: '90%',
          maxHeight: '80vh',
          overflow: 'auto',
          position: 'relative',
        }}
        onClick={(e) => e.stopPropagation()}
      >
        <button
          style={{
            position: 'absolute',
            top: 1,
            right: 5,
            backgroundColor: 'transparent',
            border: 'none',
            color: '#d4d4d4',
            fontSize: '24px',
            fontWeight: 'bold',
            cursor: 'pointer',
          }}
          onClick={handleClose}
        >
          <span>×</span>
        </button>

        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            width: '100%',
            height: `${MODAL_CONTENT_HEIGHT}px`,
          }}
        >
          <EventHeader gameEvent={gameEvent} />

          <div
            ref={logRef}
            style={{
              height: `${textLogHeight}px`,
              flexShrink: 0,
              background: '#1e1e1e',
              padding: '10px',
              border: 'none',
              overflow: 'auto',
              scrollBehavior: 'smooth',
            }}
          >
            {eventRunner.logEntries.map((entry, i) => (
              <div key={i} style={getLogEntryStyle(entry, i)}>
                {entry.text}
              </div>
            ))}
          </div>

          {showChoicesArea && (
            <div
              style={{
                height: `${CHOICES_AREA_HEIGHT}px`,
                flexShrink: 0,
                marginTop: `${CHOICES_GAP}px`,
                overflow: 'auto',
              }}
            >
              {pauseState === 'continue' && (
                <ChoiceButton
                  hasErrors={hasErrors}
                  handleNext={handleContinue}
                  text="Continue."
                />
              )}

              {pauseState === 'choice' &&
                eventRunner.displayTextChoices.map((choice, i) => (
                  <ChoiceButton
                    key={i}
                    hasErrors={hasErrors}
                    handleNext={() => handleChoice(i)}
                    text={
                      (choice.prefix ? choice.prefix + ' ' : '') + choice.text
                    }
                  />
                ))}
            </div>
          )}

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
                      const canvas = document.getElementById(
                        CANVAS_CONTAINER_ID + '-canvas'
                      ) as HTMLCanvasElement;
                      if (canvas) {
                        handleClose();
                        getEditorState().runnerErrors = eventRunner.errors;
                        notifyStateUpdated();
                        centerPanzoomOnNode(canvas, error.nodeId);
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
    </div>
  );
}
