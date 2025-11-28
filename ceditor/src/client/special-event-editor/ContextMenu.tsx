import { GameEvent, GameEventChildType, GameEventChildExec, SENode } from '../types/assets';
import { randomId } from '../utils/mathUtils';
import {
  createChoiceNode,
  createEndNode,
  createExecNode,
  createKeywordNode,
  createSwitchNode,
} from './nodeCreation';
import { screenToWorldCoords } from './seEditorEvents';
import { EditorStateSE, updateEditorState } from './seEditorState';
import { getNodeBounds } from './nodeRendering/nodeHelpers';

interface ContextMenuProps {
  x: number;
  y: number;
  canvasRef: React.RefObject<HTMLCanvasElement>;
  editorStateRef: React.RefObject<EditorStateSE>;
  gameEvent: GameEvent;
  clickedNodeId?: string | null;
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
  clickedNodeId,
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
    let newNodeX: number;
    let newNodeY: number;

    // If clicking on a node, position new node below it
    if (clickedNodeId) {
      const clickedNode = gameEvent.children.find((child) => child.id === clickedNodeId);
      if (clickedNode) {
        const [, nodeHeight] = getNodeBounds(clickedNode);
        newNodeX = clickedNode.x;
        newNodeY = clickedNode.y + (clickedNode.h || nodeHeight) + 20; // 20px spacing
      } else {
        // Fallback to mouse position if node not found
        const [worldX, worldY] = screenToWorldCoords(
          x,
          y,
          canvas,
          editorStateRef.current.zoneWidth,
          editorStateRef.current.zoneHeight
        );
        newNodeX = worldX;
        newNodeY = worldY;
      }
    } else {
      // No node clicked, use mouse position
      const [worldX, worldY] = screenToWorldCoords(
        x,
        y,
        canvas,
        editorStateRef.current.zoneWidth,
        editorStateRef.current.zoneHeight
      );
      newNodeX = worldX;
      newNodeY = worldY;
    }

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
    newNode.x = newNodeX;
    newNode.y = newNodeY;

    // If creating an exec node and clicking on a node, set the clicked node's next property
    if (nodeType === GameEventChildType.EXEC && clickedNodeId) {
      const clickedNode = gameEvent.children.find((child) => child.id === clickedNodeId);
      if (clickedNode && clickedNode.eventChildType === GameEventChildType.EXEC) {
        (clickedNode as GameEventChildExec).next = nodeId;
      }
    }

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
