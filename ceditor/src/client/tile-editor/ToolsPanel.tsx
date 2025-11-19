import { EditorState, setCurrentPaintAction } from './editorState';
import { PaintActionType } from './paintTools';

interface ToolsPanelProps {
  editorState: EditorState;
}

export function ToolsPanel({ editorState }: ToolsPanelProps) {
  const handleDrawClick = () => {
    if (editorState.currentPaintAction !== PaintActionType.DRAW) {
      setCurrentPaintAction(PaintActionType.DRAW);
    }
  };

  const currentPaintAction = editorState.currentPaintAction;

  return (
    <div
      style={{
        flex: 1,
        padding: '15px',
        overflow: 'auto',
        display: 'flex',
        flexDirection: 'column',
        gap: '10px',
      }}
    >
      <div
        style={{
          color: '#858585',
          fontSize: '11px',
          textTransform: 'uppercase',
          fontWeight: 'bold',
          marginBottom: '5px',
        }}
      >
        Tools
      </div>
      <button
        onClick={handleDrawClick}
        style={{
          padding: '10px',
          border: '1px solid #3e3e42',
          backgroundColor:
            currentPaintAction === PaintActionType.DRAW ? '#4ec9b0' : '#3e3e42',
          color:
            currentPaintAction === PaintActionType.DRAW ? '#1e1e1e' : '#ffffff',
          cursor: 'pointer',
          fontSize: '18px',
          borderRadius: '4px',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          transition: 'background-color 0.2s, color 0.2s',
        }}
        onMouseEnter={(e) => {
          if (currentPaintAction !== PaintActionType.DRAW) {
            e.currentTarget.style.backgroundColor = '#4a4a4a';
          }
        }}
        onMouseLeave={(e) => {
          if (currentPaintAction !== PaintActionType.DRAW) {
            e.currentTarget.style.backgroundColor = '#3e3e42';
          }
        }}
        title="Draw tool"
      >
        🖌️
      </button>
    </div>
  );
}
