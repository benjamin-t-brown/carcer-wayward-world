import {
  GameEvent,
  GameEventChildType,
  GameEventChildExec,
  GameEventChildSwitch,
  GameEventChildChoice,
  GameEventChildEnd,
  GameEventChildComment,
} from '../../types/assets';
import { randomId } from '../../utils/mathUtils';
import {
  EditorStateSE,
  enterLinkingMode,
  updateEditorState,
} from '../seEditorState';
import { screenToWorldCoords } from '../nodeHelpers';
import { EditorNodeExec } from '../cmpts/ExecNodeComponent';
import { EditorNode } from '../cmpts/EditorNode';
import { EditorNodeSwitch } from '../cmpts/SwitchNodeComponent';
import { EditorNodeChoice } from '../cmpts/ChoiceNodeComponent';
import { EditorNodeEnd } from '../cmpts/EndNodeComponent';
import { EditorNodeComment } from '../cmpts/CommentNodeComponent';

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
  [GameEventChildType.COMMENT]: '📝 Create Comment',
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
  // Check if clicked node is a switch node
  const clickedNode = clickedNodeId
    ? gameEvent.children.find((child) => child.id === clickedNodeId)
    : null;
  const isSwitchNode =
    clickedNode?.eventChildType === GameEventChildType.SWITCH;
  const isChoiceNode =
    clickedNode?.eventChildType === GameEventChildType.CHOICE;
  const isEndNode = clickedNode?.eventChildType === GameEventChildType.END;

  const nodeTypes = [
    GameEventChildType.EXEC,
    GameEventChildType.CHOICE,
    GameEventChildType.SWITCH,
    GameEventChildType.KEYWORD,
    GameEventChildType.COMMENT,
    GameEventChildType.END,
  ];

  const handleSelectNodeType = (nodeType: GameEventChildType) => {
    if (!canvasRef.current || !editorStateRef.current) {
      return;
    }

    const canvas = canvasRef.current;
    let newNodeX: number;
    let newNodeY: number;

    // If clicking on a node, position new node below it
    if (clickedNodeId) {
      const clickedNode = editorStateRef.current.editorNodes.find(
        (node) => node.id === clickedNodeId
      );
      if (clickedNode) {
        const { height } = clickedNode.getBounds();
        newNodeX = clickedNode.x;
        newNodeY = clickedNode.y + height + 20; // 20px spacing
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
    let newNode: EditorNode | undefined = undefined;
    // Create the node based on type
    switch (nodeType) {
      case GameEventChildType.EXEC: {
        const n = new EditorNodeExec(
          {
            id: nodeId,
            eventChildType: GameEventChildType.EXEC,
            x: 0,
            y: 0,
            h: 0,
          } as GameEventChildExec,
          editorStateRef.current
        );
        n.p = '';
        newNode = n;
        break;
      }
      case GameEventChildType.CHOICE: {
        const n = new EditorNodeChoice(
          {
            id: nodeId,
            eventChildType: GameEventChildType.CHOICE,
            x: 0,
            y: 0,
            h: 0,
            text: '',
            choices: [
              {
                text: 'This is the default choice.',
                next: '',
              },
            ],
          } as GameEventChildChoice,
          editorStateRef.current
        );
        newNode = n;
        break;
      }
      case GameEventChildType.SWITCH:
        newNode = new EditorNodeSwitch(
          {
            id: nodeId,
            eventChildType: GameEventChildType.SWITCH,
            x: 0,
            y: 0,
            h: 0,
            defaultNext: '',
            cases: [],
          } as GameEventChildSwitch,
          editorStateRef.current
        );
        break;
      case GameEventChildType.END:
        newNode = new EditorNodeEnd(
          {
            id: nodeId,
            eventChildType: GameEventChildType.END,
            x: 0,
            y: 0,
            h: 0,
            next: '',
          } as GameEventChildEnd,
          editorStateRef.current
        );
        break;
      case GameEventChildType.COMMENT:
        newNode = new EditorNodeComment(
          {
            id: nodeId,
            eventChildType: GameEventChildType.COMMENT,
            x: 0,
            y: 0,
            h: 0,
            comment: 'This is a comment node.',
          } as GameEventChildComment,
          editorStateRef.current
        );
        break;
      case GameEventChildType.KEYWORD:
        // newNode = createKeywordNode(nodeId);
        break;
      default:
        return;
    }

    if (!newNode) {
      console.error('Failed to create new node');
      return;
    }

    // Set position
    newNode.x = newNodeX;
    newNode.y = newNodeY;

    // If creating an exec node and clicking on a node, set the clicked node's next property
    if (nodeType === GameEventChildType.EXEC && clickedNodeId) {
      newNode.updateExitLink(clickedNodeId);
    }
    newNode.calculateHeight(canvasRef.current.getContext('2d')!);
    editorStateRef.current.editorNodes.push(newNode);

    updateEditorState({});
  };

  const handleLinkNode = () => {
    if (!clickedNodeId || !editorStateRef.current) {
      return;
    }
    enterLinkingMode(editorStateRef.current, clickedNodeId);
    updateEditorState({});
    onClose();
  };

  const handleCopyNodeId = async () => {
    if (!clickedNodeId || !editorStateRef.current) {
      return;
    }
    try {
      await navigator.clipboard.writeText(clickedNodeId);
    } catch (err) {
      // Fallback for older browsers
      const textArea = document.createElement('textarea');
      textArea.value = clickedNodeId;
      textArea.style.position = 'fixed';
      textArea.style.left = '-999999px';
      document.body.appendChild(textArea);
      textArea.select();
      try {
        document.execCommand('copy');
      } catch (fallbackErr) {
        console.error('Failed to copy node ID:', fallbackErr);
      }
      document.body.removeChild(textArea);
    }

    // Show feedback message
    const editorState = editorStateRef.current;
    editorState.showCopyFeedback = true;

    // Clear any existing timeout
    if (editorState.copyFeedbackTimeout !== null) {
      clearTimeout(editorState.copyFeedbackTimeout);
    }

    // Hide feedback after 2 seconds
    editorState.copyFeedbackTimeout = window.setTimeout(() => {
      editorState.showCopyFeedback = false;
      editorState.copyFeedbackTimeout = null;
      updateEditorState({});
    }, 2000);

    updateEditorState({});
    onClose();
  };

  return (
    <>
      {/* Backdrop */}
      <div
        id="context-menu-backdrop"
        style={{
          position: 'fixed',
          top: 0,
          left: 0,
          right: 0,
          bottom: 0,
          zIndex: 999,
        }}
        onClick={onClose}
        onContextMenu={(e) => {
          e.preventDefault();
          return true;
        }}
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
        {clickedNodeId && (
          <>
            <button
              onClick={() => {
                handleCopyNodeId();
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
                borderBottom: isSwitchNode ? 'none' : '1px solid #3e3e42',
                marginBottom: isSwitchNode ? '0' : '4px',
              }}
              onMouseEnter={(e) => {
                e.currentTarget.style.backgroundColor = '#2a2d2e';
              }}
              onMouseLeave={(e) => {
                e.currentTarget.style.backgroundColor = 'transparent';
              }}
            >
              📋 Copy Node ID
            </button>
            {!isSwitchNode && !isChoiceNode && !isEndNode && (
              <button
                onClick={() => {
                  handleLinkNode();
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
                  borderBottom: '1px solid #3e3e42',
                  marginBottom: '4px',
                }}
                onMouseEnter={(e) => {
                  e.currentTarget.style.backgroundColor = '#2a2d2e';
                }}
                onMouseLeave={(e) => {
                  e.currentTarget.style.backgroundColor = 'transparent';
                }}
              >
                🔗 Link Node
              </button>
            )}
          </>
        )}
        {!isSwitchNode &&
          !isChoiceNode &&
          !isEndNode &&
          nodeTypes.map((nodeType) => (
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
