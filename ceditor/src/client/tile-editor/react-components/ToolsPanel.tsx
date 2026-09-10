import { EditorState } from '../editorState';
import { CarcerMapTemplate } from '../../types/assets';
import { SelectedTileInfo } from './SelectedTileInfo';
import { OpenMapAndSelectTileArgs } from '../TileEditor';
import type { MapEditorController } from '../MapEditorController';

interface ToolsPanelProps {
  controller: MapEditorController;
  editorState: EditorState;
  map: CarcerMapTemplate;
  onMapUpdate: (map: CarcerMapTemplate) => void;
  onOpenMapAndSelectTile?: (args: OpenMapAndSelectTileArgs) => void;
}

export function ToolsPanel({
  controller,
  editorState,
  map,
  onMapUpdate,
  onOpenMapAndSelectTile,
}: ToolsPanelProps) {
  return (
    <div className="tile-editor-sidebar-section tile-editor-sidebar-selected-tile">
      <div className="tile-editor-sidebar-scroll tile-editor-sidebar-tile-info">
        <SelectedTileInfo
          controller={controller}
          editorState={editorState}
          map={map}
          onMapUpdate={onMapUpdate}
          onOpenMapAndSelectTile={onOpenMapAndSelectTile}
        />
      </div>
    </div>
  );
}
