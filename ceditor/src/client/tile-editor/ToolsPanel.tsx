import { EditorState, setCurrentPaintAction } from './editorState';
import { PaintActionType } from './paintTools';
import { CarcerMapTemplate } from '../types/assets';
import { SelectedTileInfo } from './SelectedTileInfo';

interface ToolsPanelProps {
  editorState: EditorState;
  map: CarcerMapTemplate;
  onMapUpdate: (map: CarcerMapTemplate) => void;
  onOpenMapAndSelectTile?: (
    mapName: string,
    markerName?: string,
    x?: number,
    y?: number
  ) => void;
}

interface ToolButtonProps {
  onClick: () => void;
  isActive: boolean;
  icon: string;
  title: string;
}

function ToolButton({ onClick, isActive, icon, title }: ToolButtonProps) {
  return (
    <button
      onClick={onClick}
      style={{
        padding: '6px 10px',
        border: '1px solid #3e3e42',
        backgroundColor: isActive ? '#4ec9b0' : '#3e3e42',
        color: isActive ? '#1e1e1e' : '#ffffff',
        cursor: 'pointer',
        fontSize: '18px',
        borderRadius: '4px',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        transition: 'background-color 0.2s, color 0.2s',
        width: '40px',
      }}
      onMouseEnter={(e) => {
        if (!isActive) {
          e.currentTarget.style.backgroundColor = '#4a4a4a';
        }
      }}
      onMouseLeave={(e) => {
        if (!isActive) {
          e.currentTarget.style.backgroundColor = '#3e3e42';
        }
      }}
      title={title}
    >
      {icon}
    </button>
  );
}

export function ToolsPanel({
  editorState,
  map,
  onMapUpdate,
  onOpenMapAndSelectTile,
}: ToolsPanelProps) {
  const handleDrawClick = () => {
    if (editorState.currentPaintAction !== PaintActionType.DRAW) {
      setCurrentPaintAction(PaintActionType.DRAW);
    }
  };

  const handleEraseClick = () => {
    if (editorState.currentPaintAction !== PaintActionType.ERASE) {
      setCurrentPaintAction(PaintActionType.ERASE);
    }
  };

  const handleFillClick = () => {
    if (editorState.currentPaintAction !== PaintActionType.FILL) {
      setCurrentPaintAction(PaintActionType.FILL);
    }
  };

  const handleSelectClick = () => {
    if (editorState.currentPaintAction !== PaintActionType.SELECT) {
      setCurrentPaintAction(PaintActionType.SELECT);
    }
  };

  const handleCloneClick = () => {
    if (editorState.currentPaintAction !== PaintActionType.CLONE) {
      setCurrentPaintAction(PaintActionType.CLONE);
    }
  };

  const handleEraseMetaClick = () => {
    if (editorState.currentPaintAction !== PaintActionType.ERASE_META) {
      setCurrentPaintAction(PaintActionType.ERASE_META);
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
      <div
        style={{
          display: 'flex',
          flexWrap: 'wrap',
          gap: '8px',
        }}
      >
        <ToolButton
          onClick={handleDrawClick}
          isActive={currentPaintAction === PaintActionType.DRAW}
          icon="🖌️"
          title="Draw tool"
        />
        <ToolButton
          onClick={handleEraseClick}
          isActive={currentPaintAction === PaintActionType.ERASE}
          icon="✖️"
          title="Erase tool"
        />
        <ToolButton
          onClick={handleFillClick}
          isActive={currentPaintAction === PaintActionType.FILL}
          icon="🪣"
          title="Fill tool"
        />
        <div
          style={{
            width: '100%',
            height: '1px',
            backgroundColor: '#3e3e42',
            margin: '4px 0',
          }}
        />
        <ToolButton
          onClick={handleSelectClick}
          isActive={currentPaintAction === PaintActionType.SELECT}
          icon="👆"
          title="Select tool"
        />
        <ToolButton
          onClick={handleCloneClick}
          isActive={currentPaintAction === PaintActionType.CLONE}
          icon="📋"
          title="Clone tool"
        />
        <ToolButton
          onClick={handleEraseMetaClick}
          isActive={currentPaintAction === PaintActionType.ERASE_META}
          icon="🗑"
          title="Erase metadata tool"
        />
      </div>
      <SelectedTileInfo
        editorState={editorState}
        map={map}
        onMapUpdate={onMapUpdate}
        onOpenMapAndSelectTile={onOpenMapAndSelectTile}
      />
    </div>
  );
}
