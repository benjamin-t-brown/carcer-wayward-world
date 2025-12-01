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
import { VariableWidget } from '../VariableWidget';
import { EditorNodeChoice } from '../cmpts/ChoiceNodeComponent';
import { Sprite } from '../../elements/Sprite';
import { useSprites } from '../../hooks/useSprites';
import { useSDL2WAssets } from '../../contexts/SDL2WAssetsContext';
import { centerPanzoomOnNode } from '../seEditorState';

interface EventRunnerModalProps {
  isOpen: boolean;
  gameEvent: GameEvent;
  onCancel: () => void;
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

const EventRunnerExecChild = ({
  child,
  gameEvent,
  advance,
}: {
  child: GameEventChildExec;
  gameEvent: GameEvent;
  advance: (nextNodeId: string) => void;
}) => {
  const { spriteMap } = useSDL2WAssets();
  const sprite = spriteMap[gameEvent.icon ?? ''];

  const nodeText = child.p;

  const handleNext = () => {
    advance(child.next);
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
            fontSize: '14px',
            color: 'white',
            height: 'calc(100% - 80px)',
            overflow: 'auto',
          }}
          dangerouslySetInnerHTML={{
            __html: nodeText.replace(/\n/g, '<br />'),
          }}
        ></div>
        <div
          style={{
            display: 'flex',
            justifyContent: 'flex-end',
            marginTop: '10px',
            height: '80px',
            alignItems: 'center',
          }}
        >
          <Button variant="primary" onClick={handleNext}>
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
  advance,
}: {
  child: GameEventChildChoice;
  gameEvent: GameEvent;
  advance: (nextNodeId: string) => void;
}) => {
  const { spriteMap } = useSDL2WAssets();
  const sprite = spriteMap[gameEvent.icon ?? ''];

  const nodeText = child.text;

  const handleNext = (nextNodeId: string) => {
    advance(nextNodeId);
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
            fontSize: '14px',
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
            fontSize: '14px',
            color: 'white',
            height: 'calc(60%)',
            overflow: 'auto',
          }}
        >
          {child.choices.map((choice, i) => (
            <button
              key={i}
              style={{
                background: 'black',
                padding: '10px 4px',
                fontFamily: 'arial',
                fontSize: '14px',
                color: 'white',
                border: 'none',
                cursor: 'pointer',
                width: '100%',
                textAlign: 'left',
                marginBottom: '4px',
              }}
              onClick={() => handleNext(choice.next)}
            >
              {choice.text}
            </button>
          ))}
        </div>
      </div>
    </>
  );
};

const EventRunnerEndChild = ({
  child,
  gameEvent,
  advance,
}: {
  child: GameEventChildEnd;
  gameEvent: GameEvent;
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
            fontSize: '14px',
            color: 'white',
            height: 'calc(100% - 80px)',
            overflow: 'auto',
          }}
        >
          End
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
  onCancel,
}: EventRunnerModalProps) {
  const [gameEventToRun, setGameEventToRun] = useState<GameEvent | undefined>(
    undefined
  );
  const [currentNodeId, setCurrentNodeId] = useState<string | undefined>(
    undefined
  );

  const { spriteMap } = useSDL2WAssets();
  const sprite = spriteMap[gameEventToRun?.icon ?? ''];
  const currentNode = gameEventToRun?.children.find(
    (node) => node.id === currentNodeId
  );

  const advance = (nextNodeId: string) => {
    if (nextNodeId === '') {
      onCancel();
      return;
    }
    setCurrentNodeId(nextNodeId);
    const canvas = document.getElementById(
      'special-event-editor-canvas-canvas'
    ) as HTMLCanvasElement;
    if (canvas) {
      centerPanzoomOnNode(canvas, nextNodeId);
    }
  };

  useEffect(() => {
    if (isOpen) {
      setGameEventToRun(gameEvent);
      const hasRootNode = gameEvent.children.some((node) => node.id === 'root');
      const nextNodeId = hasRootNode ? 'root' : gameEvent.children[0].id;
      advance(nextNodeId);
      // setCurrentNodeId(hasRootNode ? 'root' : gameEvent.children[0].id);
    } else {
      setCurrentNodeId(undefined);
      setGameEventToRun(undefined);
    }
  }, [isOpen, gameEvent]);

  if (!isOpen || !gameEventToRun || !currentNode) {
    return undefined;
  }

  return (
    <div
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
          maxWidth: '60vw',
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
          onClick={onCancel}
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
              gameEvent={gameEventToRun}
              advance={advance}
            />
          )}
          {currentNode.eventChildType === GameEventChildType.CHOICE && (
            <EventRunnerChoiceChild
              child={currentNode as GameEventChildChoice}
              gameEvent={gameEventToRun}
              advance={advance}
            />
          )}
          {currentNode.eventChildType === GameEventChildType.END && (
            <EventRunnerEndChild
              child={currentNode as GameEventChildEnd}
              gameEvent={gameEventToRun}
              advance={advance}
            />
          )}
        </div>
      </div>
    </div>
  );
}
