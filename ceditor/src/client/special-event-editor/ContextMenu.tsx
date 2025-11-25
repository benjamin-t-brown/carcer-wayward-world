import { GameEvent, GameEventChildType, SENode } from '../types/assets';
import { randomId } from '../utils/mathUtils';
import {
  createChoiceNode,
  createEndNode,
  createExecNode,
  createKeywordNode,
  createSwitchNode,
} from './nodeCreation';
import { screenToWorldCoords } from './seEditorEvents';
import { SpecialEventEditorState, updateEditorState } from './seEditorState';

interface ContextMenuProps {
  x: number;
  y: number;
  canvasRef: React.RefObject<HTMLCanvasElement>;
  editorStateRef: React.RefObject<SpecialEventEditorState>;
  gameEvent: GameEvent;
  onClose: () => void;
}

const NODE_TYPE_LABELS: Record<GameEventChildType, string> = {
  [GameEventChildType.EXEC]: '▶️ Create Exec',
  [GameEventChildType.CHOICE]: '🤔 Create Choice',
  [GameEventChildType.KEYWORD]: '📓 Create Keyword',
  [GameEventChildType.SWITCH]: '🧠 Create Switch',
  [GameEventChildType.END]: '🛑 Create End',
};

export function ContextMenu({
  x,
  y,
  canvasRef,
  editorStateRef,
  gameEvent,
  onClose,
}: ContextMenuProps) {
  const nodeTypes = [
    GameEventChildType.EXEC,
    GameEventChildType.CHOICE,
    GameEventChildType.SWITCH,
    GameEventChildType.KEYWORD,
    GameEventChildType.END,
  ]

  const handleSelectNodeType = (nodeType: GameEventChildType) => {
    if (!canvasRef.current || !editorStateRef.current) {
      return;
    }

    const canvas = canvasRef.current;
    const [worldX, worldY] = screenToWorldCoords(
      x,
      y,
      canvas,
      editorStateRef.current.zoneWidth,
      editorStateRef.current.zoneHeight
    );

    // Generate unique ID
    const nodeId = randomId();
    let newNode: SENode;
    // Create the node based on type
    switch (nodeType) {
      case GameEventChildType.EXEC:
        const n = createExecNode(nodeId);
        n.p = 'This is example text for an exec node.';
        newNode = n;
        break;
      case GameEventChildType.CHOICE:
        newNode = createChoiceNode(nodeId);
        break;
      case GameEventChildType.SWITCH:
        newNode = createSwitchNode(nodeId);
        break;
      case GameEventChildType.END:
        newNode = createEndNode(nodeId);
        break;
      case GameEventChildType.KEYWORD:
        newNode = createKeywordNode(nodeId);
        break;
      default:
        return;
    }

    // Set position
    newNode.x = worldX;
    newNode.y = worldY;

    // Add to gameEvent
    // const updatedGameEvent: GameEvent = {
    //   ...gameEvent,
    //   children: [...(gameEvent.children || []), newNode],
    // };
    gameEvent.children.push(newNode);
    updateEditorState({});
  };

  return (
    <>
      {/* Backdrop */}
      <div
        style={{
          position: 'fixed',
          top: 0,
          left: 0,
          right: 0,
          bottom: 0,
          zIndex: 999,
        }}
        onClick={onClose}
      />
      {/* Menu */}
      <div
        style={{
          position: 'fixed',
          left: `${x}px`,
          top: `${y}px`,
          backgroundColor: '#252526',
          border: '1px solid #3e3e42',
          borderRadius: '4px',
          padding: '4px',
          zIndex: 1000,
          minWidth: '120px',
          boxShadow: '0 2px 8px rgba(0, 0, 0, 0.3)',
        }}
        onClick={(e) => e.stopPropagation()}
      >
        {nodeTypes.map((nodeType) => (
          <button
            key={nodeType}
            onClick={() => {
              handleSelectNodeType(nodeType);
              onClose();
            }}
            style={{
              display: 'block',
              width: '100%',
              padding: '8px 12px',
              textAlign: 'left',
              backgroundColor: 'transparent',
              border: 'none',
              color: '#d4d4d4',
              cursor: 'pointer',
              fontSize: '14px',
              borderRadius: '2px',
            }}
            onMouseEnter={(e) => {
              e.currentTarget.style.backgroundColor = '#2a2d2e';
            }}
            onMouseLeave={(e) => {
              e.currentTarget.style.backgroundColor = 'transparent';
            }}
          >
            <span style={{ filter: 'grayscale(75%) brightness(1)' }}>
              {NODE_TYPE_LABELS[nodeType]}
            </span>
          </button>
        ))}
      </div>
    </>
  );
}
