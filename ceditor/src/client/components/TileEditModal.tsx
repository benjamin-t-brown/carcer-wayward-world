import { TextInput } from '../elements/TextInput';
import { Button } from '../elements/Button';
import { Sprite } from '../elements/Sprite';
import {
  TERRAIN_BORDER_META_OPTIONS,
  TileMetadata,
  TileStepSound,
  TileTerrainBorderMeta,
  TileTerrainBorderTag,
} from '../types/assets';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { OptionSelect } from '../elements/OptionSelect';
import { useState } from 'react';

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
        {TERRAIN_BORDER_META_OPTIONS.map((o) => (
          <option key={o.value} value={o.value}>
            {o.label}
          </option>
        ))}
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
  const centerScale = (3 * scale) / 2;
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

interface TileEditModalProps {
  // tile: TileMetadata | null;
  // tileIndex: number | null;
  tiles: {
    tile: TileMetadata;
    tileIndex: number;
  }[];
  tilesetName: string;
  onClose: () => void;
  onUpdate: (
    tiles: { tile: TileMetadata; tileIndex: number }[],
    field: keyof TileMetadata,
    value: any
  ) => void;
}

export function TileEditModal({
  // tile,
  // tileIndex,
  tiles,
  tilesetName,
  onClose,
  onUpdate,
}: TileEditModalProps) {
  const { spriteMap } = useSDL2WAssets();
  const [fillAllBorderMetaValue, setFillAllBorderMetaValue] =
    useState<TileTerrainBorderTag>(TileTerrainBorderTag.NONE);

  console.log('tiles', tiles);

  if (tiles.length === 0) {
    return null;
  }

  const firstTile = tiles[0];

  const handleUpdate = (field: keyof TileMetadata, value: any) => {
    onUpdate(tiles, field, value);
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
    meta: TileTerrainBorderMeta,
    key: keyof TileTerrainBorderMeta,
    value: TileTerrainBorderTag
  ) => {
    handleUpdate('tileTerrainBorderMeta', {
      ...meta,
      [key]: value,
    });
  };

  // Find the sprite for this tile: tilesetName + '_' + tile.id
  const spriteName = `${tilesetName}_${firstTile.tile.id}`;
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
          Edit Tile Template ({tiles.length} tiles).
        </h2>
        <p
          style={{
            color: '#858585',
            fontSize: '12px',
            marginBottom: '10px',
            display: tiles.length <= 1 ? 'block' : 'none',
          }}
        >
          Press Alt+Left/Right to navigate between tiles.
        </p>

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
              Sprite Name
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

        {!firstTile.tile.tileTerrainBorderMeta && (
          <div
            style={{
              display: 'flex',
              justifyContent: 'center',
              marginBottom: '15px',
            }}
          >
            <Button variant="secondary" onClick={createTerrainBorderMeta}>
              Add Terrain Border Meta
            </Button>
          </div>
        )}

        {/* Tile Preview */}
        {sprite && (
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
              {firstTile.tile.tileTerrainBorderMeta && (
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
                    value={firstTile.tile.tileTerrainBorderMeta.nw}
                    onChange={(value) => {
                      if (firstTile.tile.tileTerrainBorderMeta) {
                        updateTerrainBorderMeta(
                          firstTile.tile.tileTerrainBorderMeta,
                          'nw',
                          value
                        );
                      }
                    }}
                  />
                  <TileBorderMetaSelect
                    value={firstTile.tile.tileTerrainBorderMeta.sw}
                    onChange={(value) => {
                      if (firstTile.tile.tileTerrainBorderMeta) {
                        updateTerrainBorderMeta(
                          firstTile.tile.tileTerrainBorderMeta,
                          'sw',
                          value
                        );
                      }
                    }}
                  />
                </div>
              )}
              <div>
                <div
                  style={{
                    marginBottom: '10px',
                    color: '#d4d4d4',
                    fontSize: '14px',
                  }}
                >
                  Tile Preview
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
                      const tbMeta = firstTile.tile.tileTerrainBorderMeta;
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
                      console.log('onRender', ctx);
                    }}
                    renderDeps={[firstTile.tile.tileTerrainBorderMeta]}
                  />
                </div>
              </div>
              {firstTile.tile.tileTerrainBorderMeta && (
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
                    value={firstTile.tile.tileTerrainBorderMeta.ne}
                    onChange={(value) => {
                      if (firstTile.tile.tileTerrainBorderMeta) {
                        updateTerrainBorderMeta(
                          firstTile.tile.tileTerrainBorderMeta,
                          'ne',
                          value
                        );
                      }
                    }}
                  />
                  <TileBorderMetaSelect
                    value={firstTile.tile.tileTerrainBorderMeta.se}
                    onChange={(value) => {
                      if (firstTile.tile.tileTerrainBorderMeta) {
                        updateTerrainBorderMeta(
                          firstTile.tile.tileTerrainBorderMeta,
                          'se',
                          value
                        );
                      }
                    }}
                  />
                </div>
              )}
            </div>
            {firstTile.tile.tileTerrainBorderMeta && (
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
            )}
          </div>
        )}

        <TextInput
          id="tile-description"
          name="description"
          label="Description"
          value={firstTile.tile.description || ''}
          onChange={(value) => handleUpdate('description', value)}
          placeholder="Tile description"
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
              firstTile.tile.stepSound?.toString() ||
              TileStepSound.TILE_STEP_SOUND_FLOOR.toString()
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
            checked={firstTile.tile.isWalkable !== false}
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
            checked={firstTile.tile.isSeeThrough !== false}
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
            checked={firstTile.tile.isDoor === true}
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
            checked={firstTile.tile.isContainer === true}
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
