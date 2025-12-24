import { useState, useEffect } from 'react';
import {
  Choice,
  GameEvent,
  GameEventChildChoice,
  GameEventChildEnd,
  GameEventChildExec,
  GameEventChildType,
} from '../../types/assets';
import { Button } from '../../elements/Button';
import { VariableWidget } from '../react-components/VariableWidget';
import { EditorNodeChoice } from '../cmpts/ChoiceNodeComponent';
import { Sprite } from '../../elements/Sprite';
import { useSprites } from '../../hooks/useSprites';
import { useSDL2WAssets } from '../../contexts/SDL2WAssetsContext';
import {
  centerPanzoomOnNode,
  getEditorState,
  notifyStateUpdated,
} from '../seEditorState';
import { EventRunner } from './EventRunner';
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
      }}
    >
      <div
        style={{
          width: '2px',
          height: '2px',
        }}
      ></div>
      <Sprite sprite={sprite} maxWidth={100} />
      <div
        style={{
          width: '10px',
          height: '2px',
        }}
      ></div>
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

const EventRunnerExecChild = ({
  child,
  gameEvent,
  runner,
  advance,
}: {
  child: GameEventChildExec;
  gameEvent: GameEvent;
  runner: EventRunner;
  advance: (nextNodeId: string) => void;
}) => {
  const nodeText = runner.displayText;

  const handleNext = () => {
    advance(child.next);
  };

  const hasErrors = runner.errors.length > 0;

  return (
    <>
      <EventHeader gameEvent={gameEvent} />
      <div
        style={{
          height: 'calc(100% - 80px)',
        }}
      >
        <div
          style={{
            background: '#1e1e1e',
            padding: '10px 4px',
            fontFamily: 'arial',
            fontSize: EVENT_FONT_SIZE,
            color: 'white',
            height: 'calc(100% - 80px - 100px)',
            overflow: 'auto',
          }}
          dangerouslySetInnerHTML={{
            __html: nodeText.replace(/\n/g, '<br />'),
          }}
        ></div>
        <div
          style={{
            height: '100px',
          }}
        >
          <ChoiceButton
            hasErrors={hasErrors}
            handleNext={handleNext}
            text="Continue."
          ></ChoiceButton>
        </div>
        <div
          style={{
            display: 'flex',
            justifyContent: 'flex-end',
            marginTop: '10px',
            height: '80px',
            alignItems: 'center',
            // opacity: hasErrors ? 0.5 : 1,
          }}
        >
          <Button variant="primary" onClick={handleNext} disabled={hasErrors}>
            Okay
          </Button>
        </div>
      </div>
    </>
  );
};

const EventRunnerChoiceChild = ({
  child,
  gameEvent,
  runner,
  advance,
}: {
  child: GameEventChildChoice;
  gameEvent: GameEvent;
  runner: EventRunner;
  advance: (
    nextNodeId: string,
    {
      onceKeysToCommit,
      execStr,
    }: { onceKeysToCommit: string[]; execStr: string }
  ) => void;
}) => {
  const nodeText = runner.displayText;

  const handleNext = (index: number) => {
    const obj = runner.displayTextChoices[index];
    console.log('HANDLE NEXT', obj);
    if (obj) {
      advance(obj.next, {
        onceKeysToCommit: obj.onceKeysToCommit,
        execStr: obj.execStr,
      });
    }
  };

  return (
    <>
      <EventHeader gameEvent={gameEvent} />
      <div
        style={{
          height: 'calc(100% - 80px)',
        }}
      >
        <div
          style={{
            background: '#1e1e1e',
            padding: '10px 4px',
            fontFamily: 'arial',
            fontSize: EVENT_FONT_SIZE,
            color: 'white',
            height: 'calc(40%)',
            overflow: 'auto',
          }}
          dangerouslySetInnerHTML={{
            __html: nodeText.replace(/\n/g, '<br />'),
          }}
        ></div>
        <div
          style={{
            background: '#1e1e1e',
            padding: '10px 4px',
            fontFamily: 'arial',
            fontSize: EVENT_FONT_SIZE,
            color: 'white',
            height: 'calc(60%)',
            overflow: 'auto',
          }}
        >
          {runner.displayTextChoices.map((choice, i) => (
            <ChoiceButton
              key={i}
              hasErrors={runner.errors.length > 0}
              handleNext={() => handleNext(i)}
              text={(choice.prefix ? choice.prefix + ' ' : '') + choice.text}
            ></ChoiceButton>
          ))}
        </div>
      </div>
    </>
  );
};

const EventRunnerEndChild = ({
  child,
  gameEvent,
  runner,
  advance,
}: {
  child: GameEventChildEnd;
  gameEvent: GameEvent;
  runner: EventRunner;
  advance: (nextNodeId: string) => void;
}) => {
  const { spriteMap } = useSDL2WAssets();
  const sprite = spriteMap[gameEvent.icon ?? ''];

  return (
    <>
      <EventHeader gameEvent={gameEvent} />
      <div
        style={{
          height: 'calc(100% - 80px)',
        }}
      >
        <div
          style={{
            background: '#1e1e1e',
            padding: '10px 4px',
            fontFamily: 'arial',
            fontSize: EVENT_FONT_SIZE,
            color: 'white',
            height: 'calc(100% - 80px)',
            overflow: 'auto',
          }}
        >
          End, storage result:
          <br />
          <textarea
            spellCheck={false}
            readOnly
            style={{
              background: '#444',
              padding: '10px 4px',
              fontFamily: 'monospace',
              fontSize: '14px',
              color: 'white',
              overflow: 'auto',
              whiteSpace: 'pre',
              width: '100%',
              height: '300px',
            }}
            value={JSON.stringify(runner.storage, null, 2)}
          ></textarea>
        </div>

        <div
          style={{
            display: 'flex',
            justifyContent: 'flex-end',
            marginTop: '10px',
            height: '80px',
            alignItems: 'center',
          }}
        >
          <Button variant="primary" onClick={() => advance('')}>
            Close
          </Button>
        </div>
      </div>
    </>
  );
};

export function EventRunnerModal({
  isOpen,
  gameEvent,
  gameEvents,
  onCancel,
  eventRunner,
}: EventRunnerModalProps) {
  // const [eventRunner, setEventRunner] = useState<EventRunner | undefined>(
  //   undefined
  // );
  const reRender = useReRender();
  // const [gameEventToRun, setGameEventToRun] = useState<GameEvent | undefined>(
  //   undefined
  // );
  // const [currentNodeId, setCurrentNodeId] = useState<string | undefined>(
  //   undefined
  // );

  const { spriteMap } = useSDL2WAssets();
  const sprite = spriteMap[gameEvent?.icon ?? ''];
  const currentNode = eventRunner?.getCurrentNode();

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
      const canvas = document.getElementById(
        'special-event-editor-canvas-canvas'
      ) as HTMLCanvasElement;
      if (canvas) {
        centerPanzoomOnNode(canvas, nextNodeId);
      }
      try {
        eventRunner.advance(nextNodeId, { onceKeysToCommit, execStr });
      } catch (e) {
        console.error('error advancing event runner:', e);
        getEditorState().runnerErrors = eventRunner.errors;
        notifyStateUpdated();
      }
      reRender();
    }
  };

  if (!isOpen || !eventRunner || !currentNode) {
    return undefined;
  }

  return (
    <div
      className="generic-modal"
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
          onClick={() => {
            getEditorState().runnerErrors = eventRunner.errors;
            notifyStateUpdated();
            onCancel();
          }}
        >
          <span>×</span>
        </button>

        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            width: '100%',
            height: '500px',
          }}
        >
          {currentNode.eventChildType === GameEventChildType.EXEC && (
            <EventRunnerExecChild
              child={currentNode as GameEventChildExec}
              gameEvent={gameEvent}
              runner={eventRunner}
              advance={advance}
            />
          )}
          {currentNode.eventChildType === GameEventChildType.CHOICE && (
            <EventRunnerChoiceChild
              child={currentNode as GameEventChildChoice}
              gameEvent={gameEvent}
              runner={eventRunner}
              advance={advance}
            />
          )}
          {currentNode.eventChildType === GameEventChildType.END && (
            <EventRunnerEndChild
              child={currentNode as GameEventChildEnd}
              gameEvent={gameEvent}
              runner={eventRunner}
              advance={advance}
            />
          )}
          {eventRunner.errors.length > 0 && (
            <div>
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
                        onCancel();
                        // getEditorState().runnerErrors.push(error);
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
