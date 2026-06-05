import { useMemo } from 'react';
import { useAssets } from '../contexts/AssetsContext';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { Sprite } from '../elements/Sprite';
import {
  EditorState,
  updateEditorState,
} from './editorState';
import { PaintActionType } from './paintTools';
import {
  TERRAIN_BORDER_TAG_LABELS,
  TileTerrainBorderTag,
} from '../types/assets';
import { getSpriteNameFromTileMetadata } from '../utils/draw';
import {
  buildTerrainLookup,
  findInteriorTileId,
  getPaintableTerrainTags,
  getTerrainTileset,
  TERRAIN_TILESET_NAME,
} from './terrainTool';

interface TerrainToolPanelProps {
  editorState: EditorState;
}

export function TerrainToolPanel({ editorState }: TerrainToolPanelProps) {
  const { tilesets } = useAssets();
  const { spriteMap } = useSDL2WAssets();

  const terrainTileset = getTerrainTileset(tilesets);
  const lookup = useMemo(
    () => (terrainTileset ? buildTerrainLookup(terrainTileset) : null),
    [terrainTileset]
  );

  const paintableTags = useMemo(
    () => (lookup ? getPaintableTerrainTags(lookup) : []),
    [lookup]
  );

  if (editorState.currentPaintAction !== PaintActionType.TERRAIN) {
    return null;
  }

  if (!terrainTileset || !lookup) {
    return (
      <div className="tile-editor-sidebar-section">
        <div style={{ color: '#f44747', fontSize: '13px', padding: '8px' }}>
          Tileset &quot;{TERRAIN_TILESET_NAME}&quot; not found.
        </div>
      </div>
    );
  }

  return (
    <div className="tile-editor-sidebar-section tile-editor-terrain-panel">
      <div
        style={{
          fontSize: '12px',
          fontWeight: 600,
          color: '#cccccc',
          marginBottom: '8px',
          textTransform: 'uppercase',
          letterSpacing: '0.5px',
        }}
      >
        Terrain
      </div>
      <div
        style={{
          display: 'flex',
          flexDirection: 'column',
          gap: '6px',
        }}
      >
        {paintableTags.map((tag) => {
          const tileId = findInteriorTileId(lookup, tag);
          const tile =
            tileId !== undefined
              ? terrainTileset.tiles.find((t) => t.id === tileId)
              : undefined;
          const spriteName =
            tile &&
            getSpriteNameFromTileMetadata(terrainTileset.spriteBase, tile);
          const sprite = spriteName ? spriteMap[spriteName] : undefined;
          const isActive = editorState.selectedTerrainTag === tag;

          return (
            <button
              key={tag}
              type="button"
              title={TERRAIN_BORDER_TAG_LABELS[tag]}
              onClick={() => updateEditorState({ selectedTerrainTag: tag })}
              style={{
                display: 'flex',
                alignItems: 'center',
                gap: '8px',
                padding: '6px 8px',
                border: isActive
                  ? '2px solid #4ec9b0'
                  : '1px solid #3e3e42',
                borderRadius: '4px',
                backgroundColor: isActive ? '#2a3a36' : '#1e1e1e',
                cursor: 'pointer',
                color: '#cccccc',
                fontSize: '13px',
              }}
            >
              <div
                style={{
                  width: '36px',
                  height: '40px',
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                  backgroundColor: '#2d2d30',
                  borderRadius: '2px',
                  overflow: 'hidden',
                }}
              >
                {sprite ? (
                  <Sprite sprite={sprite} scale={1.2} />
                ) : (
                  <span style={{ fontSize: '10px', color: '#858585' }}>?</span>
                )}
              </div>
              {TERRAIN_BORDER_TAG_LABELS[tag]}
            </button>
          );
        })}
      </div>
      {paintableTags.length === 0 && (
        <div style={{ color: '#858585', fontSize: '12px' }}>
          No terrain types configured in {TERRAIN_TILESET_NAME}.
        </div>
      )}
    </div>
  );
}
