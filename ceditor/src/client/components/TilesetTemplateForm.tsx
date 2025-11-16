import { useState } from 'react';
import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { PicturePicker } from '../elements/PicturePicker';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { TileEditModal } from './TileEditModal';

export interface TileMetadata {
  id: number;
  description?: string;
  isWalkable?: boolean;
  isSeeThrough?: boolean;
  isDoor?: boolean;
}

export interface TilesetTemplate {
  name: string;
  spriteBase: string;
  imageWidth: number;
  imageHeight: number;
  tileWidth: number;
  tileHeight: number;
  tiles: TileMetadata[];
}

interface TilesetTemplateFormProps {
  tileset?: TilesetTemplate;
  updateTileset: (tileset: TilesetTemplate) => void;
}

export function createDefaultTileset(): TilesetTemplate {
  return {
    name: '',
    spriteBase: '',
    imageWidth: 0,
    imageHeight: 0,
    tileWidth: 28,
    tileHeight: 32,
    tiles: [],
  };
}

export function TilesetTemplateForm(props: TilesetTemplateFormProps) {
  const tileset = props.tileset;
  const { pictures } = useSDL2WAssets();
  const [selectedTileIndex, setSelectedTileIndex] = useState<number | null>(
    null
  );

  const formData = tileset as TilesetTemplate;
  const setFormData = (data: TilesetTemplate) => {
    props.updateTileset(data);
  };

  const updateField = <K extends keyof TilesetTemplate>(
    field: K,
    value: TilesetTemplate[K]
  ) => {
    setFormData({ ...formData, [field]: value });
  };

  const updateTile = (index: number, field: keyof TileMetadata, value: any) => {
    const newTiles = [...formData.tiles];
    newTiles[index] = { ...newTiles[index], [field]: value };
    setFormData({ ...formData, tiles: newTiles });
  };

  const handleTileClick = (index: number) => {
    setSelectedTileIndex(index);
  };

  const handleSelectPicture = (
    value: string,
    imageWidth: number,
    imageHeight: number
  ) => {
    const numTiles =
      Math.floor(imageWidth / formData.tileWidth) *
      Math.floor(imageHeight / formData.tileHeight);
    const newTiles = Array.from({ length: numTiles }, (_, index) => ({
      id: index,
      description: '',
      isWalkable: true,
      isSeeThrough: true,
      isDoor: false,
    }));
    setFormData({
      ...formData,
      name: value,
      imageWidth: imageWidth,
      imageHeight: imageHeight,
      spriteBase: value,
      tiles: newTiles,
    });
    // setImageDimensions({ width: imageWidth, height: imageHeight });
  };

  const getTilePosition = (index: number): { x: number; y: number } => {
    if (formData.tileWidth === 0 || formData.tileHeight === 0) {
      return { x: 0, y: 0 };
    }
    const tilesWide = Math.floor(
      (formData.imageWidth || 0) / formData.tileWidth
    );
    const x = (index % tilesWide) * formData.tileWidth;
    const y = Math.floor(index / tilesWide) * formData.tileHeight;
    return { x, y };
  };

  if (!tileset) {
    return <div>Select a tileset to edit</div>;
  }

  const tilesWide =
    formData.imageWidth && formData.tileWidth > 0
      ? Math.floor(formData.imageWidth / formData.tileWidth)
      : 0;
  const tilesHigh =
    formData.imageHeight && formData.tileHeight > 0
      ? Math.floor(formData.imageHeight / formData.tileHeight)
      : 0;
  const selectedTile =
    selectedTileIndex !== null ? formData.tiles[selectedTileIndex] : null;
  const picturePath = formData.spriteBase
    ? pictures[formData.spriteBase]
    : null;

  return (
    <div className="item-form">
      <h2>Edit Tileset</h2>
      <form>
        <TextInput
          id="tileset-name"
          name="name"
          label="Name"
          value={formData.name}
          onChange={(value) => updateField('name', value)}
          placeholder="select an image to populate the name"
          required
          disabled
        />

        <div className="form-group">
          <label htmlFor="tileset-sprite-base">Sprite Base (Picture) *</label>
          <div style={{ marginTop: '8px' }}>
            <PicturePicker
              value={formData.spriteBase}
              onChange={handleSelectPicture}
            />
          </div>
          {formData.spriteBase && (
            <div
              style={{ marginTop: '8px', fontSize: '12px', color: '#858585' }}
            >
              Selected: {formData.spriteBase}
              {formData.imageWidth && formData.imageHeight && (
                <span>
                  {' '}
                  ({formData.imageWidth} × {formData.imageHeight}px)
                </span>
              )}
            </div>
          )}
        </div>

        <div className="form-row">
          <NumberInput
            id="tileset-tile-width"
            name="tileWidth"
            label="Tile Width"
            value={formData.tileWidth}
            onChange={(value) => updateField('tileWidth', value || 0)}
            min={1}
            disabled
          />

          <NumberInput
            id="tileset-tile-height"
            name="tileHeight"
            label="Tile Height"
            value={formData.tileHeight}
            onChange={(value) => updateField('tileHeight', value || 0)}
            min={1}
            disabled
          />
        </div>

        {formData.spriteBase &&
          formData.tileWidth > 0 &&
          formData.tileHeight > 0 &&
          picturePath && (
            <div className="form-group">
              <label>
                Tile Grid ({tilesWide} × {tilesHigh} tiles)
              </label>
              <div
                style={{
                  marginTop: '10px',
                  position: 'relative',
                  display: 'inline-block',
                  border: '2px solid #3e3e42',
                  borderRadius: '4px',
                  overflow: 'auto',
                  maxWidth: '100%',
                  maxHeight: '600px',
                }}
              >
                <div style={{ position: 'relative', display: 'inline-block' }}>
                  <img
                    src={`/api/${picturePath}`}
                    alt={formData.spriteBase}
                    style={{
                      display: 'block',
                      imageRendering: 'pixelated',
                      maxWidth: '100%',
                      height: 'auto',
                    }}
                  />
                  {/* Overlay grid */}
                  <div
                    style={{
                      position: 'absolute',
                      top: 0,
                      left: 0,
                      right: 0,
                      bottom: 0,
                      pointerEvents: 'none',
                    }}
                  >
                    {formData.tiles.map((tile, index) => {
                      const { x, y } = getTilePosition(index);
                      const isSelected = selectedTileIndex === index;
                      const hasDescription = tile.description && tile.description.trim() !== '';
                      return (
                        <div
                          key={index}
                          onClick={(e) => {
                            e.stopPropagation();
                            handleTileClick(index);
                          }}
                          style={{
                            position: 'absolute',
                            left: `${x}px`,
                            top: `${y}px`,
                            width: `${formData.tileWidth}px`,
                            height: `${formData.tileHeight}px`,
                            border: isSelected
                              ? '2px solid #4ec9b0'
                              : hasDescription
                              ? '1px solid rgba(78, 201, 176, 0.3)'
                              : '1px solid rgba(255, 0, 0, 0.5)',
                            backgroundColor: isSelected
                              ? 'rgba(78, 201, 176, 0.2)'
                              : 'transparent',
                            cursor: 'pointer',
                            pointerEvents: 'auto',
                            boxSizing: 'border-box',
                          }}
                          onMouseEnter={(e) => {
                            if (!isSelected) {
                              e.currentTarget.style.backgroundColor =
                                'rgba(78, 201, 176, 0.41)';
                            }
                          }}
                          onMouseLeave={(e) => {
                            if (!isSelected) {
                              e.currentTarget.style.backgroundColor =
                                'transparent';
                            }
                          }}
                          title={`Tile ${index}: ${
                            tile.description || 'No description'
                          }`}
                        />
                      );
                    })}
                  </div>
                </div>
              </div>
              <div
                style={{
                  marginTop: '10px',
                  fontSize: '12px',
                  color: '#858585',
                }}
              >
                Click on a tile to edit its properties
              </div>
            </div>
          )}

        <TileEditModal
          tile={selectedTile}
          tileIndex={selectedTileIndex}
          tilesetName={formData.name}
          onClose={() => setSelectedTileIndex(null)}
          onUpdate={updateTile}
        />
      </form>
    </div>
  );
}
