import { EditorState, setCurrentPaintAction } from './editorState';
import { PaintActionType } from './paintTools';

interface ToolButtonProps {
  onClick: () => void;
  isActive: boolean;
  icon: string;
  title: string;
}

function ToolButton({
  onClick,
  isActive,
  icon,
  title,
  iconClassName,
}: ToolButtonProps & { iconClassName?: string }) {
  return (
    <button
      type="button"
      className={`tile-editor-tool-button${isActive ? ' active' : ''}`}
      onClick={onClick}
      title={title}
    >
      {iconClassName ? (
        <span className={iconClassName}>{icon}</span>
      ) : (
        icon
      )}
    </button>
  );
}

export function MapToolsOverlay({ editorState }: { editorState: EditorState }) {
  const currentPaintAction = editorState.currentPaintAction;

  return (
    <div
      className="tile-editor-map-tools-overlay"
      onMouseDown={(e) => e.stopPropagation()}
      onClick={(e) => e.stopPropagation()}
    >
      <div className="tile-editor-tool-grid">
        <div className="tile-editor-tool-row">
          <ToolButton
            onClick={() => {
              if (currentPaintAction !== PaintActionType.SELECT) {
                setCurrentPaintAction(PaintActionType.SELECT);
              }
            }}
            isActive={currentPaintAction === PaintActionType.SELECT}
            icon="👆"
            title="Select tool"
          />
          <ToolButton
            onClick={() => {
              if (currentPaintAction !== PaintActionType.DRAW) {
                setCurrentPaintAction(PaintActionType.DRAW);
              }
            }}
            isActive={currentPaintAction === PaintActionType.DRAW}
            icon="🖌️"
            title="Draw tool"
          />
          <ToolButton
            onClick={() => {
              if (currentPaintAction !== PaintActionType.FILL) {
                setCurrentPaintAction(PaintActionType.FILL);
              }
            }}
            isActive={currentPaintAction === PaintActionType.FILL}
            icon="🪣"
            title="Fill tool"
          />
          <ToolButton
            onClick={() => {
              if (currentPaintAction !== PaintActionType.CLONE) {
                setCurrentPaintAction(PaintActionType.CLONE);
              }
            }}
            isActive={currentPaintAction === PaintActionType.CLONE}
            icon="📋"
            title="Clone tool"
          />
        </div>
        <div className="tile-editor-tool-row tile-editor-tool-row-secondary">
          <ToolButton
            onClick={() => {
              if (currentPaintAction !== PaintActionType.ERASE) {
                setCurrentPaintAction(PaintActionType.ERASE);
              }
            }}
            isActive={currentPaintAction === PaintActionType.ERASE}
            icon="✖️"
            title="Erase tool"
          />
          <ToolButton
            onClick={() => {
              if (currentPaintAction !== PaintActionType.ERASE_META) {
                setCurrentPaintAction(PaintActionType.ERASE_META);
              }
            }}
            isActive={currentPaintAction === PaintActionType.ERASE_META}
            icon="🗑"
            title="Erase metadata tool"
          />
          <ToolButton
            onClick={() => {
              if (currentPaintAction !== PaintActionType.DELETE_FILL) {
                setCurrentPaintAction(PaintActionType.DELETE_FILL);
              }
            }}
            isActive={currentPaintAction === PaintActionType.DELETE_FILL}
            icon="🪣"
            iconClassName="tile-editor-tool-icon-bucket-delete"
            title="Delete fill tool"
          />
        </div>
      </div>
    </div>
  );
}
