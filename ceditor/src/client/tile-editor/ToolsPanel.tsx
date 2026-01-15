import { EditorState, setCurrentPaintAction } from './editorState';
import { PaintActionType } from './paintTools';
import { CarcerMapTemplate } from '../types/assets';
import { SelectedTileInfo } from './SelectedTileInfo';
import { OpenMapAndSelectTileArgs } from './TileEditor';

interface ToolsPanelProps {
  editorState: EditorState;
  map: CarcerMapTemplate;
  onMapUpdate: (map: CarcerMapTemplate) => void;
  onOpenMapAndSelectTile?: (args: OpenMapAndSelectTileArgs) => void;
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
        color: 'white',
        padding: '2px',
        fontWeight: 'bold',
        fontSize: '16px',
        background: isActive ? '#4ec9b0' : '#3e3e42',
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
        padding: '2px',
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
