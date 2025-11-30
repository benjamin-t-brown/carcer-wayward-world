import { useEffect, useRef, useState } from 'react';
import { GameEvent, GameEventChildType } from '../types/assets';
import { useRenderLoop } from '../hooks/useRenderLoop';
import {
  centerPanzoomOnNode,
  getEditorState,
  initEditorStateForGameEvent,
  updateEditorState,
} from './seEditorState';
import { EditorStateSE } from './seEditorState';
import { CANVAS_CONTAINER_ID, MapCanvasSE } from './MapCanvasSE';
import {
  initPanzoom,
  unInitPanzoom,
  checkRightClickLineEvents,
} from './seEditorEvents';
import { loop } from './seLoop';
import { useReRender } from '../hooks/useReRender';
import { ContextMenu } from './ContextMenu';
import { EditExecNodeModal } from './modals/EditExecNodeModal';
import { EditSwitchNodeModal } from './modals/EditSwitchNodeModal';
import { GameEventChildExec, GameEventChildSwitch } from '../types/assets';
import { screenToWorldCoords } from './nodeHelpers';
import { EditorNodeExec } from './cmpts/ExecNodeComponent';
import { EditorNodeSwitch } from './cmpts/SwitchNodeComponent';
import { EditorNodeChoice } from './cmpts/ChoiceNodeComponent';
import { EditChoiceNodeModal } from './modals/EditChoiceNodeModal';

interface SpecialEventEditorProps {
  gameEvent: GameEvent;
  onUpdateGameEvent: (gameEvent: GameEvent) => void;
}

let prevTs = performance.now();

export function SpecialEventEditor({ gameEvent }: SpecialEventEditorProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const editorState = useRef<EditorStateSE | undefined>(undefined);
  const reRender = useReRender();
  const [contextMenu, setContextMenu] = useState<{
    x: number;
    y: number;
    clickedNodeId?: string | null;
  } | null>(null);
  const [editingExecNode, setEditingExecNode] = useState<
    EditorNodeExec | undefined
  >(undefined);
  const [editingSwitchNode, setEditingSwitchNode] = useState<
    EditorNodeSwitch | undefined
  >(undefined);
  const [editingChoiceNode, setEditingChoiceNode] = useState<
    EditorNodeChoice | undefined
  >(undefined);

  // hack im lazy
  (window as any).reRenderSpecialEventEditor = reRender;

  useEffect(() => {
    editorState.current = getEditorState();
  }, [editorState]);

  // Update editor state when gameEvent changes
  useEffect(() => {
    const gameEventId = gameEvent?.id;
    const canvas = canvasRef.current;

    if (gameEventId && canvas) {
      initEditorStateForGameEvent(gameEvent, canvas);
      editorState.current = getEditorState();
      updateEditorState({ gameEventId: gameEventId, baseGameEvent: gameEvent });
    }
  }, [gameEvent?.id]);

  useEffect(() => {
    console.log('initPanzoom SE');
    const canvas = canvasRef.current;
    if (!canvas) {
      return;
    }

    const handleContextMenu = (ev: MouseEvent) => {
      ev.preventDefault();
      const targetId = (ev.target as HTMLElement)?.id;
      if (ev.target === canvas || targetId.includes(CANVAS_CONTAINER_ID)) {
        const currentEditorState = editorState.current;
        if (currentEditorState && currentEditorState.gameEventId) {
          // Check if right-clicking on a line first
          const lineClicked = checkRightClickLineEvents({
            ev,
            canvas,
            editorState: currentEditorState,
          });

          if (lineClicked) {
            return; // Don't show context menu if line was deleted
          }

          // Check if right-clicking on a node
          const [worldX, worldY] = screenToWorldCoords(
            ev.clientX,
            ev.clientY,
            canvas,
            currentEditorState.zoneWidth,
            currentEditorState.zoneHeight
          );

          let clickedNodeId: string | undefined = undefined;
          for (const node of currentEditorState.editorNodes) {
            if (node.isPointInBounds(worldX, worldY)) {
              clickedNodeId = node.id;
              break;
            }
          }

          setContextMenu({
            x: ev.clientX,
            y: ev.clientY,
            clickedNodeId: clickedNodeId,
          });
        } else {
          setContextMenu({ x: ev.clientX, y: ev.clientY });
        }
      }
    };

    canvas.addEventListener('contextmenu', handleContextMenu);
    initPanzoom({
      getCanvas: () => canvasRef.current as HTMLCanvasElement,
      getEditorState: () => editorState.current as EditorStateSE,
      getEditorFuncs: () => ({
        onNodeDoubleClick: (nodeId: string) => {
          // Find the node and open edit modal
          const node = editorState.current?.editorNodes.find(
            (n) => n.id === nodeId
          );
          if (node) {
            if (node.type === GameEventChildType.EXEC) {
              setEditingExecNode(node as EditorNodeExec);
            } else if (node.type === GameEventChildType.SWITCH) {
              setEditingSwitchNode(node as EditorNodeSwitch);
            } else if (node.type === GameEventChildType.CHOICE) {
              setEditingChoiceNode(node as EditorNodeChoice);
            }
          }
        },
      }),
    });

    return () => {
      canvas.removeEventListener('contextmenu', handleContextMenu);
      console.log('unInitPanzoom SE');
      unInitPanzoom();
    };
  }, []);

  // Handle Ctrl+Z for undo TODO later
  // useEffect(() => {
  //   const handleKeyDown = (e: KeyboardEvent) => {
  //     // Check if Ctrl+Z (or Cmd+Z on Mac) is pressed
  //     if ((e.ctrlKey || e.metaKey) && e.key === 'z' && !e.shiftKey) {
  //       // Check if we're not focused on an input field
  //       const activeElement = document.activeElement;
  //       const isInputFocused =
  //         activeElement &&
  //         (activeElement.tagName === 'INPUT' ||
  //           activeElement.tagName === 'TEXTAREA' ||
  //           activeElement.getAttribute('contenteditable') === 'true');

  //       if (!isInputFocused && mapRef.current && editorState.current) {
  //         e.preventDefault();
  //         const success = undo(mapRef.current, editorState.current);
  //         if (success) {
  //           // Trigger map update to reflect changes
  //           onMapUpdate({ ...mapRef.current });
  //         }
  //       }
  //     }
  //   };

  //   window.addEventListener('keydown', handleKeyDown);
  //   return () => {
  //     window.removeEventListener('keydown', handleKeyDown);
  //   };
  // }, [onMapUpdate]);

  useRenderLoop((ts) => {
    if (canvasRef.current && editorState.current) {
      loop(
        {
          getCanvas: () => canvasRef.current as HTMLCanvasElement,
          getEditorState: () => editorState.current as EditorStateSE,
        },
        ts - prevTs
      );
    }
    prevTs = ts;
  });

  if (!gameEvent) {
    return (
      <div
        style={{
          color: '#858585',
          fontSize: '14px',
          textAlign: 'center',
          marginTop: '50px',
        }}
      >
        Select a game event to get started.
      </div>
    );
  }

  return (
    <>
      <div
        style={{
          display: 'flex',
          height: '100%',
          width: '100%',
          overflow: 'hidden',
        }}
      >
        <div
          style={{
            position: 'relative',
            width: '100%',
            height: '100%',
          }}
        >
          <MapCanvasSE
            canvasRef={canvasRef}
            width={editorState.current?.zoneWidth || 1000}
            height={editorState.current?.zoneHeight || 1000}
          />
          {/* <div
            id="se-nodes"
            style={{
              position: 'absolute',
              left: 0,
              top: 0,
              width: editorState.current?.zoneWidth || 1000,
              height: editorState.current?.zoneHeight || 1000,
              background: 'blue',
            }}
          >
          </div> */}
        </div>
      </div>

      {contextMenu && (
        <ContextMenu
          x={contextMenu.x}
          y={contextMenu.y}
          canvasRef={canvasRef}
          editorStateRef={editorState as React.RefObject<EditorStateSE>}
          gameEvent={gameEvent}
          clickedNodeId={contextMenu.clickedNodeId}
          onClose={() => setContextMenu(null)}
        />
      )}

      <EditExecNodeModal
        isOpen={editingExecNode !== undefined}
        node={editingExecNode}
        gameEvent={gameEvent}
        onCancel={() => setEditingExecNode(undefined)}
        ctx={canvasRef.current?.getContext('2d') as CanvasRenderingContext2D}
      />
      <EditSwitchNodeModal
        isOpen={editingSwitchNode !== undefined}
        node={editingSwitchNode}
        gameEvent={gameEvent}
        onCancel={() => setEditingSwitchNode(undefined)}
      />
      <EditChoiceNodeModal
        isOpen={editingChoiceNode !== undefined}
        node={editingChoiceNode}
        gameEvent={gameEvent}
        onCancel={() => setEditingChoiceNode(undefined)}
      />
    </>
  );
}
