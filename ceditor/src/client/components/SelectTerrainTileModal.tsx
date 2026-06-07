import { useMemo, useState } from 'react';
import { GenericModal } from '../elements/GenericModal';
import {
  PAINTABLE_TERRAIN_BORDER_OPTIONS,
  TilesetTemplate,
  TileTerrainBorderMeta,
  TileTerrainBorderTag,
} from '../types/assets';

export interface TerrainTileSelection {
  primaryTerrain: TileTerrainBorderTag;
  secondaryTerrain: TileTerrainBorderTag;
  startTileId: number;
}

interface SelectTerrainTileModalProps {
  tileset: TilesetTemplate;
  picturePath: string;
  onConfirm: (selection: TerrainTileSelection) => void;
  onCancel: () => void;
}

const TERRAIN_AUTOTILE_COUNT = 13;

const TERRAIN_AUTOTILE_LAYOUT: ReadonlyArray<{
  label: string;
  corners: string;
}> = [
  { label: 'center', corners: 'PPPP' },
  { label: 'edge N', corners: 'SSPP' },
  { label: 'edge S', corners: 'PPSS' },
  { label: 'edge E', corners: 'PSPS' },
  { label: 'edge W', corners: 'SPSP' },
  { label: 'outer NE', corners: 'PSPP' },
  { label: 'outer NW', corners: 'SPPP' },
  { label: 'outer SW', corners: 'PPPS' },
  { label: 'outer SE', corners: 'PPSP' },
  { label: 'inner NW', corners: 'SSPS' },
  { label: 'inner NE', corners: 'SSSP' },
  { label: 'inner SE', corners: 'PSSS' },
  { label: 'inner SW', corners: 'SPSS' },
];

function cornersFromPattern(
  pattern: string,
  primary: TileTerrainBorderTag,
  secondary: TileTerrainBorderTag,
): TileTerrainBorderMeta {
  const mapChar = (c: string): TileTerrainBorderTag => {
    if (c === 'P') {
      return primary;
    }
    if (c === 'S') {
      return secondary;
    }
    throw new Error(`Invalid corner char: ${c}`);
  };
  return {
    nw: mapChar(pattern[0]),
    ne: mapChar(pattern[1]),
    sw: mapChar(pattern[2]),
    se: mapChar(pattern[3]),
  };
}

export function getTerrainLayoutMeta(
  layoutIndex: number,
  primary: TileTerrainBorderTag,
  secondary: TileTerrainBorderTag,
): TileTerrainBorderMeta | null {
  if (layoutIndex < 0 || layoutIndex >= TERRAIN_AUTOTILE_COUNT) {
    return null;
  }
  return cornersFromPattern(
    TERRAIN_AUTOTILE_LAYOUT[layoutIndex].corners,
    primary,
    secondary,
  );
}

function getTilePosition(
  index: number,
  tileset: TilesetTemplate,
): { x: number; y: number } {
  if (tileset.tileWidth === 0 || tileset.tileHeight === 0) {
    return { x: 0, y: 0 };
  }
  const tilesWide = Math.floor(tileset.imageWidth / tileset.tileWidth);
  const x = (index % tilesWide) * tileset.tileWidth;
  const y = Math.floor(index / tilesWide) * tileset.tileHeight;
  return { x, y };
}

export function SelectTerrainTileModal({
  tileset,
  picturePath,
  onConfirm,
  onCancel,
}: SelectTerrainTileModalProps) {
  const defaultPrimary =
    tileset.terrain?.primaryTerrain ?? TileTerrainBorderTag.GRASS;
  const defaultSecondary =
    tileset.terrain?.secondaryTerrain ?? TileTerrainBorderTag.WATER;

  const [primaryTerrain, setPrimaryTerrain] =
    useState<TileTerrainBorderTag>(defaultPrimary);
  const [secondaryTerrain, setSecondaryTerrain] =
    useState<TileTerrainBorderTag>(defaultSecondary);
  const [startTileId, setStartTileId] = useState<number | null>(
    tileset.terrain?.startTileId ?? null,
  );
  const [error, setError] = useState<string | null>(null);

  const stripEndId =
    startTileId !== null ? startTileId + TERRAIN_AUTOTILE_COUNT - 1 : null;

  const tilesInStrip = useMemo(() => {
    if (startTileId === null) {
      return 0;
    }
    return tileset.tiles.filter(
      (t) => t.id >= startTileId && t.id < startTileId + TERRAIN_AUTOTILE_COUNT,
    ).length;
  }, [tileset.tiles, startTileId]);

  const handleConfirm = () => {
    if (startTileId === null) {
      setError('Click a tile on the grid to set the strip start.');
      return;
    }
    if (primaryTerrain === secondaryTerrain) {
      setError('Primary and secondary terrain must differ.');
      return;
    }
    setError(null);
    onConfirm({
      primaryTerrain,
      secondaryTerrain,
      startTileId,
    });
  };

  const selectStyle: React.CSSProperties = {
    display: 'block',
    marginTop: '4px',
    minWidth: '140px',
    padding: '6px 8px',
    backgroundColor: '#3c3c3c',
    color: '#d4d4d4',
    border: '1px solid #3e3e42',
    borderRadius: '4px',
  };

  return (
    <GenericModal
      title="Select Terrain Tile"
      maxWidth="720px"
      fillBody
      onCancel={onCancel}
      onConfirm={handleConfirm}
      body={() => (
        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            gap: '12px',
            minHeight: 0,
            flex: 1,
          }}
        >
          <div
            style={{
              display: 'flex',
              flexWrap: 'wrap',
              gap: '16px',
              alignItems: 'flex-end',
            }}
          >
            <label>
              <span style={{ fontSize: '12px', color: '#858585' }}>
                Primary terrain
              </span>
              <select
                value={primaryTerrain}
                onChange={(e) => {
                  setError(null);
                  setPrimaryTerrain(e.target.value as TileTerrainBorderTag);
                }}
                style={selectStyle}
              >
                {PAINTABLE_TERRAIN_BORDER_OPTIONS.map((o) => (
                  <option key={o.value} value={o.value}>
                    {o.label}
                  </option>
                ))}
              </select>
            </label>
            <label>
              <span style={{ fontSize: '12px', color: '#858585' }}>
                Secondary terrain
              </span>
              <select
                value={secondaryTerrain}
                onChange={(e) => {
                  setError(null);
                  setSecondaryTerrain(e.target.value as TileTerrainBorderTag);
                }}
                style={selectStyle}
              >
                {PAINTABLE_TERRAIN_BORDER_OPTIONS.map((o) => (
                  <option key={o.value} value={o.value}>
                    {o.label}
                  </option>
                ))}
              </select>
            </label>
            {startTileId !== null && (
              <div style={{ fontSize: '12px', color: '#4ec9b0' }}>
                Strip: tile ids {startTileId}–{stripEndId} ({tilesInStrip} tile
                {tilesInStrip !== 1 ? 's' : ''} in sheet)
              </div>
            )}
          </div>

          <p style={{ margin: 0, fontSize: '12px', color: '#858585' }}>
            Pick primary and secondary terrains, then click the first tile of
            your autotile strip. That tile and the next 12 tile ids form the
            strip; Confirm applies corner tags to all of them.
          </p>

          {primaryTerrain === secondaryTerrain && (
            <div style={{ fontSize: '12px', color: '#f44747' }}>
              Primary and secondary terrain must differ.
            </div>
          )}

          {error && (
            <div style={{ fontSize: '12px', color: '#f44747' }}>{error}</div>
          )}

          <div
            style={{
              flex: 1,
              minHeight: 0,
              overflow: 'auto',
              border: '2px solid #3e3e42',
              borderRadius: '4px',
            }}
          >
            <div style={{ position: 'relative', display: 'inline-block' }}>
              <img
                src={`/api/${picturePath}`}
                alt={tileset.spriteBase}
                style={{
                  display: 'block',
                  imageRendering: 'pixelated',
                }}
              />
              <div
                style={{
                  position: 'absolute',
                  top: 0,
                  left: 0,
                  right: 0,
                  bottom: 0,
                }}
              >
                {tileset.tiles.map((tile, index) => {
                  const { x, y } = getTilePosition(index, tileset);
                  const isStart =
                    startTileId !== null && tile.id === startTileId;
                  const inStrip =
                    startTileId !== null &&
                    tile.id >= startTileId &&
                    tile.id < startTileId + TERRAIN_AUTOTILE_COUNT;
                  return (
                    <div
                      key={index}
                      onClick={() => {
                        setError(null);
                        setStartTileId(tile.id);
                      }}
                      title={`Tile id ${tile.id}`}
                      style={{
                        position: 'absolute',
                        left: `${x}px`,
                        top: `${y}px`,
                        width: `${tileset.tileWidth}px`,
                        height: `${tileset.tileHeight}px`,
                        border: isStart
                          ? '2px solid #ce9178'
                          : inStrip
                            ? '1px solid rgba(206, 145, 120, 0.6)'
                            : '1px solid rgba(255, 255, 255, 0.15)',
                        backgroundColor: isStart
                          ? 'rgba(206, 145, 120, 0.3)'
                          : inStrip
                            ? 'rgba(206, 145, 120, 0.12)'
                            : 'transparent',
                        cursor: 'pointer',
                        boxSizing: 'border-box',
                      }}
                    />
                  );
                })}
              </div>
            </div>
          </div>
        </div>
      )}
    />
  );
}

/** Apply standard autotile corner meta to tile ids start..start+12. */
export function applyTerrainStripLayout(
  tileset: TilesetTemplate,
  primary: TileTerrainBorderTag,
  secondary: TileTerrainBorderTag,
  startTileId: number,
): { ok: boolean; error?: string; tileset?: TilesetTemplate } {
  if (primary === secondary) {
    return { ok: false, error: 'Primary and secondary terrain must differ.' };
  }

  const newTiles = tileset.tiles.map((tile) => {
    const offset = tile.id - startTileId;
    if (offset < 0 || offset >= TERRAIN_AUTOTILE_COUNT) {
      return tile;
    }
    const meta = getTerrainLayoutMeta(offset, primary, secondary);
    if (!meta) {
      return tile;
    }
    return { ...tile, tileTerrainBorderMeta: meta };
  });

  return {
    ok: true,
    tileset: { ...tileset, tiles: newTiles },
  };
}
