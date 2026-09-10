import { EditorState, setCurrentPaintAction } from '../editorState';
import { PaintActionType } from '../paintTools';
import { MapTool, TOOL_LIST } from '../tools';
import type { MapEditorController } from '../MapEditorController';

function ToolButton({
  tool,
  isActive,
  onClick,
}: {
  tool: MapTool;
  isActive: boolean;
  onClick: () => void;
}) {
  return (
    <button
      type="button"
      className={`tile-editor-tool-button${isActive ? ' active' : ''}`}
      onClick={onClick}
      title={tool.title}
    >
      {tool.iconClassName ? (
        <span className={tool.iconClassName}>{tool.icon}</span>
      ) : (
        tool.icon
      )}
    </button>
  );
}

const ROWS: Array<MapTool['row']> = ['primary', 'secondary'];

export function MapToolsOverlay({
  controller,
  editorState,
}: {
  controller: MapEditorController;
  editorState: EditorState;
}) {
  const currentPaintAction = editorState.currentPaintAction;

  return (
    <div
      className="tile-editor-map-tools-overlay"
      onMouseDown={(e) => e.stopPropagation()}
      onClick={(e) => e.stopPropagation()}
    >
      <div className="tile-editor-tool-grid">
        {ROWS.map((row) => (
          <div
            key={row}
            className={`tile-editor-tool-row${
              row === 'secondary' ? ' tile-editor-tool-row-secondary' : ''
            }`}
          >
            {TOOL_LIST.filter((tool) => tool.row === row).map((tool) => (
              <ToolButton
                key={tool.id}
                tool={tool}
                isActive={currentPaintAction === tool.id}
                onClick={() => {
                  if (currentPaintAction !== tool.id) {
                    setCurrentPaintAction(
                      controller,
                      tool.id as PaintActionType,
                    );
                  }
                }}
              />
            ))}
          </div>
        ))}
      </div>
    </div>
  );
}
