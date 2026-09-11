import { useEffect, useRef, useState, useSyncExternalStore } from 'react';
import { GameEvent, GameEventChildType } from '../types/assets';
import { useRenderLoop } from '../hooks/useRenderLoop';
import {
  initEditorStateForGameEvent,
  updateEditorState,
} from './seEditorState';
import { MapCanvasSE } from './react-components/MapCanvasSE';
import {
  initPanzoom,
  unInitPanzoom,
  checkRightClickLineEvents,
} from './seEditorEvents';
import { loop } from './seLoop';
import { ContextMenu } from './react-components/ContextMenu';
import { EditExecNodeModal } from './modals/EditExecNodeModal';
import { EditSwitchNodeModal } from './modals/EditSwitchNodeModal';
import { screenToWorldCoords } from './nodeHelpers';
import { EditorNodeExec } from './cmpts/ExecNodeComponent';
import { EditorNodeSwitch } from './cmpts/SwitchNodeComponent';
import { EditorNodeChoice } from './cmpts/ChoiceNodeComponent';
import { EditorNodeEnd } from './cmpts/EndNodeComponent';
import { EditChoiceNodeModal } from './modals/EditChoiceNodeModal';
import { EditEndNodeModal } from './modals/EditEndNodeModal';
import { EditorNodeComment } from './cmpts/CommentNodeComponent';
import { EditCommentModal } from './modals/EditCommentModal';
import { SpecialEventEditorController } from './SpecialEventEditorController';
import { cloneSpecialEventDocument } from './specialEventDocument';

interface SpecialEventEditorProps {
  controller: SpecialEventEditorController;
  gameEvent: GameEvent;
}

export function SpecialEventEditor({
  controller,
  gameEvent,
}: SpecialEventEditorProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const prevTsRef = useRef(performance.now());
  const selectedGameEventRef = useRef<GameEvent | null>(null);
  if (
    !selectedGameEventRef.current ||
    selectedGameEventRef.current.id !== gameEvent.id
  ) {
    selectedGameEventRef.current = cloneSpecialEventDocument(gameEvent);
  }
  const gameEventId = gameEvent.id;
  useSyncExternalStore(
    controller.subscribe,
    controller.getSnapshot,
    controller.getSnapshot,
  );
  const editorState = controller.getState();
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
  const [editingCommentNode, setEditingCommentNode] = useState<
    EditorNodeComment | undefined
  >(undefined);
  const [editingEndNode, setEditingEndNode] = useState<
    EditorNodeEnd | undefined
  >(undefined);

  // Update editor state when gameEvent changes
  useEffect(() => {
    const canvas = canvasRef.current;

    if (gameEventId && canvas) {
      const selectedGameEvent = selectedGameEventRef.current;
      if (!selectedGameEvent) return;
      controller.resetDocumentInteraction();
      initEditorStateForGameEvent(controller, selectedGameEvent, canvas);
      updateEditorState(controller, { gameEventId });
    }
  }, [controller, gameEventId]);

  useEffect(() => {
    console.log('initPanzoom SE');
    const canvas = canvasRef.current;
    if (!canvas) {
      return;
    }

    const showContextMenu = (ev: MouseEvent) => {
      const currentEditorState = controller.getState();
      if (currentEditorState.gameEventId) {
        // Check if right-clicking on a line first
        const lineClicked = checkRightClickLineEvents({
          controller,
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
          currentEditorState.zoneHeight,
          currentEditorState,
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
    };

    initPanzoom(controller, {
      getCanvas: () => canvasRef.current as HTMLCanvasElement,
      getEditorFuncs: () => ({
        onContextMenu: showContextMenu,
        onNodeDoubleClick: (nodeId: string) => {
          // Find the node and open edit modal
          const node = controller
            .getState()
            .editorNodes.find((n) => n.id === nodeId);
          if (node) {
            if (node.type === GameEventChildType.EXEC) {
              setEditingExecNode(node as EditorNodeExec);
            } else if (node.type === GameEventChildType.SWITCH) {
              setEditingSwitchNode(node as EditorNodeSwitch);
            } else if (node.type === GameEventChildType.CHOICE) {
              setEditingChoiceNode(node as EditorNodeChoice);
            } else if (node.type === GameEventChildType.COMMENT) {
              setEditingCommentNode(node as EditorNodeComment);
            } else if (node.type === GameEventChildType.END) {
              setEditingEndNode(node as EditorNodeEnd);
            }
          }
        },
      }),
    });

    return () => {
      console.log('unInitPanzoom SE');
      unInitPanzoom(controller);
    };
  }, [controller]);

  useRenderLoop((ts) => {
    if (canvasRef.current) {
      loop(
        {
          getCanvas: () => canvasRef.current as HTMLCanvasElement,
          getEditorState: controller.getState,
        },
        ts - prevTsRef.current,
      );
    }
    prevTsRef.current = ts;
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
            width={editorState.zoneWidth || 1000}
            height={editorState.zoneHeight || 1000}
          />
        </div>
      </div>

      {contextMenu && (
        <ContextMenu
          x={contextMenu.x}
          y={contextMenu.y}
          canvasRef={canvasRef}
          controller={controller}
          clickedNodeId={contextMenu.clickedNodeId}
          onClose={() => setContextMenu(null)}
        />
      )}

      <EditExecNodeModal
        controller={controller}
        isOpen={editingExecNode !== undefined}
        node={editingExecNode}
        gameEvent={gameEvent}
        onCancel={() => setEditingExecNode(undefined)}
        ctx={canvasRef.current?.getContext('2d') as CanvasRenderingContext2D}
      />
      <EditSwitchNodeModal
        controller={controller}
        isOpen={editingSwitchNode !== undefined}
        node={editingSwitchNode}
        gameEvent={gameEvent}
        onCancel={() => setEditingSwitchNode(undefined)}
        ctx={canvasRef.current?.getContext('2d') as CanvasRenderingContext2D}
      />
      <EditChoiceNodeModal
        controller={controller}
        isOpen={editingChoiceNode !== undefined}
        node={editingChoiceNode}
        gameEvent={gameEvent}
        onCancel={() => setEditingChoiceNode(undefined)}
        ctx={canvasRef.current?.getContext('2d') as CanvasRenderingContext2D}
      />
      <EditCommentModal
        controller={controller}
        isOpen={editingCommentNode !== undefined}
        node={editingCommentNode}
        gameEvent={gameEvent}
        onCancel={() => setEditingCommentNode(undefined)}
        ctx={canvasRef.current?.getContext('2d') as CanvasRenderingContext2D}
      />
      <EditEndNodeModal
        controller={controller}
        isOpen={editingEndNode !== undefined}
        node={editingEndNode}
        gameEvent={gameEvent}
        onCancel={() => setEditingEndNode(undefined)}
      />
    </>
  );
}
