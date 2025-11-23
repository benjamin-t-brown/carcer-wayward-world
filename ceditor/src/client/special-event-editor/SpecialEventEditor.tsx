import { useEffect, useRef } from 'react';
import { GameEvent } from '../types/assets';
import { useRenderLoop } from '../hooks/useRenderLoop';
import { getEditorState, updateEditorState } from './seEditorState';
import { SpecialEventEditorState } from './seEditorState';
import { MapCanvasSE } from './MapCanvasSE';
import { initPanzoom, unInitPanzoom } from './seEditorEvents';
import { loop } from './seLoop';
import { useReRender } from '../hooks/useReRender';

interface SpecialEventEditorProps {
  gameEvent: GameEvent;
}

let prevTs = performance.now();

export function SpecialEventEditor({ gameEvent }: SpecialEventEditorProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const editorState = useRef<SpecialEventEditorState | undefined>(undefined);
  // const { sprites, spriteMap } = useSDL2WAssets();
  // const { gameEvents } = useAssets();
  const reRender = useReRender();

  // console.log('re render tile editor');

  // hack im lazy
  (window as any).reRenderSpecialEventEditor = reRender;

  useEffect(() => {
    editorState.current = getEditorState();
  }, [editorState]);

  // Update editor state when gameEvent changes
  useEffect(() => {
    if (gameEvent?.id) {
      updateEditorState({ selectedEventId: gameEvent.id });
      editorState.current = getEditorState();
    }
  }, [gameEvent?.id]);

  useEffect(() => {
    console.log('initPanzoom SE');
    initPanzoom({
      getCanvas: () => canvasRef.current as HTMLCanvasElement,
      // getEditorState: () => editorState.current as SpecialEventEditorState,
    });
    return () => {
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
          getEditorState: () => editorState.current as SpecialEventEditorState,
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
          flex: 1,
          display: 'flex',
          flexDirection: 'column',
          overflow: 'hidden',
        }}
      >
        <MapCanvasSE
          canvasRef={canvasRef}
          width={editorState.current?.zoneWidth || 1000}
          height={editorState.current?.zoneHeight || 1000}
        />
      </div>
    </div>
  );
}
