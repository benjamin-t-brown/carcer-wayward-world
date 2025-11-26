import { useEffect, useRef, useState } from 'react';
import { GameEvent } from '../types/assets';
import { useRenderLoop } from '../hooks/useRenderLoop';
import { getEditorState, updateEditorState } from './seEditorState';
import { EditorStateSE } from './seEditorState';
import { CANVAS_CONTAINER_ID, MapCanvasSE } from './MapCanvasSE';
import { initPanzoom, unInitPanzoom } from './seEditorEvents';
import { loop } from './seLoop';
import { useReRender } from '../hooks/useReRender';
import { ContextMenu } from './ContextMenu';
import { EditExecNodeModal } from './modals/EditExecNodeModal';
import { GameEventChildExec } from '../types/assets';

interface SpecialEventEditorProps {
  gameEvent: GameEvent;
  onUpdateGameEvent: (gameEvent: GameEvent) => void;
}

let prevTs = performance.now();

export function SpecialEventEditor({
  gameEvent,
  onUpdateGameEvent,
}: SpecialEventEditorProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const editorState = useRef<EditorStateSE | undefined>(undefined);
  const reRender = useReRender();
  const [contextMenu, setContextMenu] = useState<{
    x: number;
    y: number;
  } | null>(null);
  const [editingNode, setEditingNode] = useState<GameEventChildExec | null>(
    null
  );

  // hack im lazy
  (window as any).reRenderSpecialEventEditor = reRender;

  useEffect(() => {
    editorState.current = getEditorState();
  }, [editorState]);

  // Update editor state when gameEvent changes
  useEffect(() => {
    if (gameEvent?.id) {
      updateEditorState({ gameEvent: gameEvent });
      editorState.current = getEditorState();
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
        setContextMenu({ x: ev.clientX, y: ev.clientY });
      }
    };

    canvas.addEventListener('contextmenu', handleContextMenu);
    initPanzoom({
      getCanvas: () => canvasRef.current as HTMLCanvasElement,
      getEditorState: () => editorState.current as EditorStateSE,
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
            // flex: 1,
            // display: 'flex',
            // flexDirection: 'column',
            // overflow: 'hidden',
            position: 'relative',
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
          editorStateRef={
            editorState as React.RefObject<EditorStateSE>
          }
          gameEvent={gameEvent}
          onClose={() => setContextMenu(null)}
        />
      )}

      <EditExecNodeModal
        isOpen={editingNode !== null}
        node={editingNode}
        gameEvent={gameEvent}
        updateGameEvent={onUpdateGameEvent}
        onCancel={() => setEditingNode(null)}
      />
    </>
  );
}
