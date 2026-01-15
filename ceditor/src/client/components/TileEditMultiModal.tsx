import { TextInput } from '../elements/TextInput';
import { Button } from '../elements/Button';
import { Sprite } from '../elements/Sprite';
import {
  TileMetadata,
  TileStepSound,
  TileTerrainBorderMeta,
  TileTerrainBorderTag,
} from '../types/assets';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { OptionSelect } from '../elements/OptionSelect';
import { useState } from 'react';

interface TileEditMultiModalProps {
  tiles: TileMetadata[];
  tileIndices: number[];
  tilesetName: string;
  onClose: () => void;
  onUpdate: (index: number, field: keyof TileMetadata, value: any) => void;
}

const TileBorderMetaSelect = (props: {
  value: TileTerrainBorderTag;
  onChange: (value: TileTerrainBorderTag) => void;
}) => {
  return (
    <div>
      <select
        style={{
          width: '100px',
        }}
        value={props.value}
        onChange={(e) => props.onChange(e.target.value as TileTerrainBorderTag)}
      >
        <option value={TileTerrainBorderTag.NONE}>None</option>
        <option value={TileTerrainBorderTag.GRASS}>Grass</option>
        <option value={TileTerrainBorderTag.DIRT}>Dirt</option>
        <option value={TileTerrainBorderTag.WATER}>Water</option>
      </select>
    </div>
  );
};

const renderBorderMetaTag = (
  pos: 'ne' | 'nw' | 'se' | 'sw',
  tag: TileTerrainBorderTag,
  scale: number,
  ctx: CanvasRenderingContext2D
) => {
  let x = 0;
  let y = 0;
  const centerScale = 3 * scale / 2;
  switch (pos) {
    case 'ne':
      x = ctx.canvas.width - 5 * scale;
      y = 0;
      break;
    case 'nw':
      x = 0;
      y = 0;
      break;
    case 'se':
      x = ctx.canvas.width - 5 * scale;
      y = ctx.canvas.height - 5 * scale;
      break;
    case 'sw':
      x = 0;
      y = ctx.canvas.height - 5 * scale;
      break;
  }
  ctx.fillStyle = '#00000066';
  ctx.fillRect(x, y, 5 * scale, 5 * scale);
  ctx.font = '10px Arial';
  ctx.fillStyle = '#FFFFFF';
  ctx.textAlign = 'left';
  ctx.textBaseline = 'middle';
  ctx.fillText(tag[0].toUpperCase(), x + centerScale, y + (5 * scale) / 2);
};

// Helper to check if all tiles have the same value for a field
const allTilesHaveSameValue = <K extends keyof TileMetadata>(
  tiles: TileMetadata[],
  field: K
): boolean => {
  if (tiles.length === 0) return true;
  const firstValue = tiles[0][field];
  return tiles.every((tile) => tile[field] === firstValue);
};

// Helper to get a representative value (first tile's value, or null if mixed)
const getRepresentativeValue = <K extends keyof TileMetadata>(
  tiles: TileMetadata[],
  field: K
): TileMetadata[K] | null => {
  if (tiles.length === 0) return null;
  if (allTilesHaveSameValue(tiles, field)) {
    return tiles[0][field];
  }
  return null; // Mixed values
};

export function TileEditMultiModal({
  tiles,
  tileIndices,
  tilesetName,
  onClose,
  onUpdate,
}: TileEditMultiModalProps) {
  const { spriteMap } = useSDL2WAssets();
  const [fillAllBorderMetaValue, setFillAllBorderMetaValue] =
    useState<TileTerrainBorderTag>(TileTerrainBorderTag.NONE);

  if (!tiles || tiles.length === 0 || tileIndices.length === 0) {
    return null;
  }

  const handleUpdate = (field: keyof TileMetadata, value: any) => {
    // Update all selected tiles
    tileIndices.forEach((index) => {
      onUpdate(index, field, value);
    });
  };

  const createTerrainBorderMeta = () => {
    handleUpdate('tileTerrainBorderMeta', {
      ne: TileTerrainBorderTag.NONE,
      nw: TileTerrainBorderTag.NONE,
      se: TileTerrainBorderTag.NONE,
      sw: TileTerrainBorderTag.NONE,
    });
  };

  const updateTerrainBorderMeta = (
    key: keyof TileTerrainBorderMeta,
    value: TileTerrainBorderTag
  ) => {
    // Update all tiles with the same border meta structure
    tileIndices.forEach((index, i) => {
      const tile = tiles[i];
      if (tile) {
        onUpdate(index, 'tileTerrainBorderMeta', {
          ...(tile.tileTerrainBorderMeta || {
            ne: TileTerrainBorderTag.NONE,
            nw: TileTerrainBorderTag.NONE,
            se: TileTerrainBorderTag.NONE,
            sw: TileTerrainBorderTag.NONE,
          }),
          [key]: value,
        });
      }
    });
  };

  // Use first tile for preview (or could show multiple)
  const firstTile = tiles[0];
  const spriteName = `${tilesetName}_${firstTile.id}`;
  const sprite = spriteMap[spriteName];

  const tileStepSoundOptions = [
    {
      label: 'Floor',
      value: TileStepSound.TILE_STEP_SOUND_FLOOR as number,
    },
    {
      label: 'Grass',
      value: TileStepSound.TILE_STEP_SOUND_GRASS as number,
    },
    {
      label: 'Dirt',
      value: TileStepSound.TILE_STEP_SOUND_DIRT as number,
    },
    {
      label: 'Gravel',
      value: TileStepSound.TILE_STEP_SOUND_GRAVEL as number,
    },
  ];

  // Check if all tiles have terrain border meta
  const allHaveTerrainBorderMeta = tiles.every(
    (tile) => tile.tileTerrainBorderMeta
  );
  const representativeBorderMeta = getRepresentativeValue(
    tiles,
    'tileTerrainBorderMeta'
  ) as TileTerrainBorderMeta | null;

  return (
    <div
      style={{
        position: 'fixed',
        top: 0,
        left: 0,
        right: 0,
        bottom: 0,
        backgroundColor: 'rgba(0, 0, 0, 0.7)',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        zIndex: 1000,
      }}
    >
      <div
        style={{
          backgroundColor: '#252526',
          border: '1px solid #3e3e42',
          borderRadius: '8px',
          padding: '30px',
          maxWidth: '500px',
          width: '90%',
          maxHeight: '80vh',
          overflow: 'auto',
        }}
        onClick={(e) => e.stopPropagation()}
      >
        <h2
          style={{
            color: '#4ec9b0',
            marginBottom: '20px',
            marginTop: 0,
          }}
        >
          Edit {tileIndices.length} Tile{tileIndices.length !== 1 ? 's' : ''} (Indices: {tileIndices.join(', ')})
        </h2>

        {/* Sprite Name */}
        {sprite && (
          <div
            style={{
              marginBottom: '15px',
              padding: '10px',
              backgroundColor: '#1e1e1e',
              borderRadius: '4px',
              textAlign: 'center',
            }}
          >
            <div
              style={{
                color: '#858585',
                fontSize: '12px',
                marginBottom: '4px',
              }}
            >
              Preview Sprite (First Tile)
            </div>
            <div
              style={{
                color: '#d4d4d4',
                fontSize: '14px',
                fontFamily: 'monospace',
              }}
            >
              {spriteName}
            </div>
          </div>
        )}

        {!allHaveTerrainBorderMeta && (
          <div
            style={{
              display: 'flex',
              justifyContent: 'center',
              marginBottom: '15px',
            }}
          >
            <Button variant="secondary" onClick={createTerrainBorderMeta}>
              Add Terrain Border Meta to All
            </Button>
          </div>
        )}

        {/* Tile Preview */}
        {sprite && representativeBorderMeta && (
          <div
            style={{
              marginBottom: '20px',
              padding: '15px',
              backgroundColor: '#1e1e1e',
              borderRadius: '4px',
            }}
          >
            <div
              style={{
                textAlign: 'center',
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'center',
                gap: '4px',
              }}
            >
              <div
                style={{
                  height: '130px',
                  display: 'flex',
                  flexDirection: 'column',
                  justifyContent: 'space-between',
                  transform: 'translateY(10px)',
                }}
              >
                <TileBorderMetaSelect
                  value={representativeBorderMeta.nw}
                  onChange={(value) => {
                    updateTerrainBorderMeta('nw', value);
                  }}
                />
                <TileBorderMetaSelect
                  value={representativeBorderMeta.sw}
                  onChange={(value) => {
                    updateTerrainBorderMeta('sw', value);
                  }}
                />
              </div>
              <div>
                <div
                  style={{
                    marginBottom: '10px',
                    color: '#d4d4d4',
                    fontSize: '14px',
                  }}
                >
                  Tile Preview (First Tile)
                </div>
                <div
                  style={{
                    display: 'inline-block',
                    padding: '10px',
                    backgroundColor: '#2d2d30',
                    borderRadius: '4px',
                    border: '2px solid #3e3e42',
                  }}
                >
                  <Sprite
                    sprite={sprite}
                    scale={4}
                    onRender={(ctx) => {
                      const tbMeta = representativeBorderMeta;
                      if (tbMeta) {
                        if (tbMeta.ne !== TileTerrainBorderTag.NONE) {
                          renderBorderMetaTag('ne', tbMeta.ne, 4, ctx);
                        }
                        if (tbMeta.nw !== TileTerrainBorderTag.NONE) {
                          renderBorderMetaTag('nw', tbMeta.nw, 4, ctx);
                        }
                        if (tbMeta.se !== TileTerrainBorderTag.NONE) {
                          renderBorderMetaTag('se', tbMeta.se, 4, ctx);
                        }
                        if (tbMeta.sw !== TileTerrainBorderTag.NONE) {
                          renderBorderMetaTag('sw', tbMeta.sw, 4, ctx);
                        }
                      }
                    }}
                    renderDeps={[representativeBorderMeta]}
                  />
                </div>
              </div>
              <div
                style={{
                  height: '130px',
                  display: 'flex',
                  flexDirection: 'column',
                  justifyContent: 'space-between',
                  transform: 'translateY(10px)',
                }}
              >
                <TileBorderMetaSelect
                  value={representativeBorderMeta.ne}
                  onChange={(value) => {
                    updateTerrainBorderMeta('ne', value);
                  }}
                />
                <TileBorderMetaSelect
                  value={representativeBorderMeta.se}
                  onChange={(value) => {
                    updateTerrainBorderMeta('se', value);
                  }}
                />
              </div>
            </div>
            <div
              style={{
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'center',
                gap: '4px',
                width: '100%',
                marginTop: '10px',
              }}
            >
              <span>Set All Borders to: </span>
              <TileBorderMetaSelect
                value={fillAllBorderMetaValue}
                onChange={(value) => {
                  setFillAllBorderMetaValue(value);
                  handleUpdate('tileTerrainBorderMeta', {
                    ne: value,
                    nw: value,
                    se: value,
                    sw: value,
                  });
                }}
              />
            </div>
          </div>
        )}

        <TextInput
          id="tile-description"
          name="description"
          label="Description"
          value={
            allTilesHaveSameValue(tiles, 'description')
              ? (firstTile.description || '')
              : '(Mixed values)'
          }
          onChange={(value) => handleUpdate('description', value)}
          placeholder="Tile description (applies to all selected tiles)"
          onKeyDown={(e) => {
            if (e.key === 'Enter') {
              onClose();
            }
          }}
        />
        <div className="form-group">
          <label htmlFor="stepSound">Step Sound</label>
          <OptionSelect
            id="stepSound"
            name="stepSound"
            options={tileStepSoundOptions}
            value={
              allTilesHaveSameValue(tiles, 'stepSound')
                ? (firstTile.stepSound?.toString() ||
                    TileStepSound.TILE_STEP_SOUND_FLOOR.toString())
                : ''
            }
            onChange={(value) => handleUpdate('stepSound', value)}
          />
        </div>
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: '10px',
            margin: '4px 0',
          }}
        >
          <input
            id="isWalkable"
            name="isWalkable"
            type="checkbox"
            checked={
              allTilesHaveSameValue(tiles, 'isWalkable')
                ? firstTile.isWalkable !== false
                : false
            }
            onChange={(e) => handleUpdate('isWalkable', e.target.checked)}
            style={{ cursor: 'pointer', transform: 'scale(1.5)' }}
          />
          <label htmlFor="isWalkable">Is Walkable</label>
        </div>

        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: '10px',
            margin: '4px 0',
          }}
        >
          <input
            id="isSeeThrough"
            name="isSeeThrough"
            type="checkbox"
            checked={
              allTilesHaveSameValue(tiles, 'isSeeThrough')
                ? firstTile.isSeeThrough !== false
                : false
            }
            onChange={(e) => handleUpdate('isSeeThrough', e.target.checked)}
            style={{ cursor: 'pointer', transform: 'scale(1.5)' }}
          />
          <label htmlFor="isSeeThrough">Is See Through</label>
        </div>
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: '10px',
            margin: '4px 0',
          }}
        >
          <input
            id="isDoor"
            name="isDoor"
            type="checkbox"
            checked={
              allTilesHaveSameValue(tiles, 'isDoor')
                ? firstTile.isDoor === true
                : false
            }
            onChange={(e) => handleUpdate('isDoor', e.target.checked)}
            style={{ cursor: 'pointer', transform: 'scale(1.5)' }}
          />
          <label htmlFor="isDoor">Is Door</label>
        </div>
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: '10px',
            margin: '4px 0',
          }}
        >
          <input
            id="isContainer"
            name="isContainer"
            type="checkbox"
            checked={
              allTilesHaveSameValue(tiles, 'isContainer')
                ? firstTile.isContainer === true
                : false
            }
            onChange={(e) => handleUpdate('isContainer', e.target.checked)}
            style={{ cursor: 'pointer', transform: 'scale(1.5)' }}
          />
          <label htmlFor="isContainer">Is Container</label>
        </div>
        <div
          style={{
            display: 'flex',
            gap: '10px',
            justifyContent: 'flex-end',
            marginTop: '30px',
          }}
        >
          <Button variant="primary" onClick={onClose}>
            Close
          </Button>
        </div>
      </div>
    </div>
  );
}
