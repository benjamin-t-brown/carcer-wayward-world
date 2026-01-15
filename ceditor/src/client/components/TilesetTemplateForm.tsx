import { useState, useEffect } from 'react';
import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { PicturePicker } from '../elements/PicturePicker';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { TileEditModal } from './TileEditModal';
import { TileEditMultiModal } from './TileEditMultiModal';
import { TileStepSound, TileMetadata, TilesetTemplate } from '../types/assets';
import { Button } from '../elements/Button';

// Re-export for backward compatibility
export { TileStepSound };
export type { TileMetadata, TilesetTemplate };

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

function createDefaultTile(): TileMetadata {
  return {
    id: 0,
    description: '',
    stepSound: TileStepSound.TILE_STEP_SOUND_FLOOR,
    isWalkable: true,
    isSeeThrough: true,
    isDoor: false,
    isContainer: false,
  };
}

export function TilesetTemplateForm(props: TilesetTemplateFormProps) {
  const tileset = props.tileset;
  const { pictures } = useSDL2WAssets();
  const [selectedTileIndex, setSelectedTileIndex] = useState<number | null>(
    null
  );
  const [selectedTileIndices, setSelectedTileIndices] = useState<number[]>([]);
  const [isSelecting, setIsSelecting] = useState(false);
  const [selectionStart, setSelectionStart] = useState<{
    x: number;
    y: number;
  } | null>(null);
  const [selectionEnd, setSelectionEnd] = useState<{
    x: number;
    y: number;
  } | null>(null);
  const [dragStartPos, setDragStartPos] = useState<{
    x: number;
    y: number;
  } | null>(null);
  const [isDragging, setIsDragging] = useState(false);

  const formData = tileset as TilesetTemplate;
  const setFormData = (data: TilesetTemplate) => {
    props.updateTileset(data);
  };

  console.log('render tileset edit form', tileset);

  const updateField = <K extends keyof TilesetTemplate>(
    field: K,
    value: TilesetTemplate[K]
  ) => {
    setFormData({ ...formData, [field]: value });
  };

  const updateTiles = (
    tiles: { tile: TileMetadata; tileIndex: number }[],
    field: keyof TileMetadata,
    value: any
  ) => {
    const newTiles = [...formData.tiles];
    for (const tile of tiles) {
      newTiles[tile.tileIndex] = {
        ...newTiles[tile.tileIndex],
        [field]: value,
      };
    }
    console.log('SET FORM DATA', formData);
    setFormData({ ...formData, tiles: newTiles });
  };

  const handleTileClick = (index: number, e: React.MouseEvent) => {
    // Don't handle click if we were dragging or if we're currently selecting
    // if (isDragging || isSelecting) {
    //   return;
    // }

    // If Ctrl/Cmd is held, toggle selection
    if (e.ctrlKey || e.metaKey) {
      setSelectedTileIndices((prev) => {
        if (prev.includes(index)) {
          return prev.filter((i) => i !== index);
        } else {
          return [...prev, index];
        }
      });
      setSelectedTileIndex(null);
    } else {
      // Single click - clear multi-selection and select single tile
      if (selectedTileIndices.length <= 1) {
        setSelectedTileIndex(index);
      }
      setSelectedTileIndices([]);
    }
  };

  const handleTileMouseDown = (_index: number, e: React.MouseEvent) => {
    // Only start drag if not Ctrl/Cmd clicking
    if (!e.ctrlKey && !e.metaKey && selectedTileIndices.length <= 1) {
      // Find the container div that wraps the image
      const container = (e.currentTarget as HTMLElement).closest(
        'div[style*="position: relative"]'
      ) as HTMLElement;
      if (container) {
        const rect = container.getBoundingClientRect();
        const x = e.clientX - rect.left;
        const y = e.clientY - rect.top;
        setDragStartPos({ x, y });
        setIsDragging(false);
        setSelectedTileIndex(null);
      }
    }
  };

  const handleContainerMouseDown = (e: React.MouseEvent) => {
    // Only start selection if clicking on the image container, not on a tile div
    if (
      e.target === e.currentTarget ||
      (e.target as HTMLElement).tagName === 'IMG'
    ) {
      const rect = (e.currentTarget as HTMLElement).getBoundingClientRect();
      const x = e.clientX - rect.left;
      const y = e.clientY - rect.top;

      setIsSelecting(true);
      setSelectionStart({ x, y });
      setSelectionEnd({ x, y });
      setSelectedTileIndices([]);
      setSelectedTileIndex(null);
      setIsDragging(false);
      setDragStartPos({ x, y });
    }
  };

  const handleContainerMouseMove = (e: React.MouseEvent) => {
    const rect = (e.currentTarget as HTMLElement).getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;

    // Check if we should start dragging from a tile
    if (dragStartPos && !isSelecting) {
      const dragDistance = Math.sqrt(
        Math.pow(x - dragStartPos.x, 2) + Math.pow(y - dragStartPos.y, 2)
      );
      if (dragDistance > 5) {
        // Started dragging - clear selection and start new selection
        setIsDragging(true);
        setIsSelecting(true);
        setSelectionStart(dragStartPos);
        setSelectionEnd({ x, y });
        setSelectedTileIndices([]);
        setSelectedTileIndex(null);
      }
    }

    if (isSelecting && selectionStart) {
      setSelectionEnd({ x, y });

      // Calculate selected tiles
      const minX = Math.min(selectionStart.x, x);
      const maxX = Math.max(selectionStart.x, x);
      const minY = Math.min(selectionStart.y, y);
      const maxY = Math.max(selectionStart.y, y);

      const selectedIndices: number[] = [];
      for (let i = 0; i < formData.tiles.length; i++) {
        const { x: tileX, y: tileY } = getTilePosition(i);
        const tileRight = tileX + formData.tileWidth;
        const tileBottom = tileY + formData.tileHeight;

        // Check if tile overlaps with selection rectangle
        if (
          tileX < maxX &&
          tileRight > minX &&
          tileY < maxY &&
          tileBottom > minY
        ) {
          selectedIndices.push(i);
        }
      }
      setSelectedTileIndices(selectedIndices);
    }
  };

  const handleContainerMouseUp = (e: React.MouseEvent) => {
    if (isSelecting && selectionStart) {
      const rect = (e.currentTarget as HTMLElement).getBoundingClientRect();
      const x = e.clientX - rect.left;
      const y = e.clientY - rect.top;

      // Calculate selected tiles
      const minX = Math.min(selectionStart.x, x);
      const maxX = Math.max(selectionStart.x, x);
      const minY = Math.min(selectionStart.y, y);
      const maxY = Math.max(selectionStart.y, y);

      const selectedIndices: number[] = [];
      for (let i = 0; i < formData.tiles.length; i++) {
        const { x: tileX, y: tileY } = getTilePosition(i);
        const tileRight = tileX + formData.tileWidth;
        const tileBottom = tileY + formData.tileHeight;

        // Check if tile overlaps with selection rectangle
        if (
          tileX < maxX &&
          tileRight > minX &&
          tileY < maxY &&
          tileBottom > minY
        ) {
          selectedIndices.push(i);
        }
      }
      setSelectedTileIndices(selectedIndices);
    }

    setIsSelecting(false);
    setSelectionStart(null);
    setSelectionEnd(null);
    setDragStartPos(null);
    setIsDragging(false);
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
      ...createDefaultTile(),
      id: index,
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

  // Keyboard navigation: Alt+Left/Right to navigate between tiles when modal is open
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      // Check if modal is open and no multi-selection
      if (
        selectedTileIndex !== null &&
        selectedTileIndices.length === 0 &&
        e.altKey
      ) {
        if (e.key === 'ArrowLeft') {
          e.preventDefault();
          const newIndex = selectedTileIndex - 1;
          if (newIndex >= 0 && newIndex < formData.tiles.length) {
            setSelectedTileIndex(newIndex);
          }
        } else if (e.key === 'ArrowRight') {
          e.preventDefault();
          const newIndex = selectedTileIndex + 1;
          if (newIndex >= 0 && newIndex < formData.tiles.length) {
            setSelectedTileIndex(newIndex);
          }
        }
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => {
      window.removeEventListener('keydown', handleKeyDown);
    };
  }, [selectedTileIndex, selectedTileIndices.length, formData?.tiles?.length]);

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

  const selectedTiles: { tile: TileMetadata; tileIndex: number }[] = [];
  if (selectedTileIndex !== null) {
    if (selectedTileIndices.length > 0) {
      for (const index of selectedTileIndices) {
        if (formData.tiles[index]) {
          selectedTiles.push({ tile: formData.tiles[index], tileIndex: index });
        }
      }
    } else if (selectedTile) {
      selectedTiles.push({ tile: selectedTile, tileIndex: selectedTileIndex });
    }
  }

  return (
    <div className="item-form">
      <h2>Edit Tileset</h2>
      <form>
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

        {formData.spriteBase &&
          formData.tileWidth > 0 &&
          formData.tileHeight > 0 &&
          picturePath && (
            <div className="form-group">
              <div style={{ marginBottom: '10px' }}>
                <Button
                  variant="secondary"
                  disabled={selectedTileIndices.length === 0}
                  onClick={() => {
                    setSelectedTileIndices(selectedTileIndices);
                    setSelectedTileIndex(selectedTileIndices[0]);
                  }}
                >
                  Edit Selected Tiles
                </Button>
              </div>

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
                onMouseDown={handleContainerMouseDown}
                onMouseMove={handleContainerMouseMove}
                onMouseUp={handleContainerMouseUp}
                onMouseLeave={handleContainerMouseUp}
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
                  {/* Selection rectangle */}
                  {isSelecting && selectionStart && selectionEnd && (
                    <div
                      style={{
                        position: 'absolute',
                        left: `${Math.min(selectionStart.x, selectionEnd.x)}px`,
                        top: `${Math.min(selectionStart.y, selectionEnd.y)}px`,
                        width: `${Math.abs(
                          selectionEnd.x - selectionStart.x
                        )}px`,
                        height: `${Math.abs(
                          selectionEnd.y - selectionStart.y
                        )}px`,
                        border: '2px dashed #4ec9b0',
                        backgroundColor: 'rgba(78, 201, 176, 0.1)',
                        pointerEvents: 'none',
                        zIndex: 10,
                      }}
                    />
                  )}
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
                      const isMultiSelected =
                        selectedTileIndices.includes(index);
                      const hasDescription =
                        tile.description && tile.description.trim() !== '';
                      return (
                        <div
                          key={index}
                          onMouseDown={(e) => {
                            handleTileMouseDown(index, e);
                          }}
                          onClick={(e) => {
                            e.stopPropagation();
                            handleTileClick(index, e);
                          }}
                          style={{
                            position: 'absolute',
                            left: `${x}px`,
                            top: `${y}px`,
                            width: `${formData.tileWidth}px`,
                            height: `${formData.tileHeight}px`,
                            border: isSelected
                              ? '2px solid #4ec9b0'
                              : isMultiSelected
                              ? '2px solid #4ec9b0'
                              : hasDescription
                              ? '1px solid rgba(78, 201, 176, 0.3)'
                              : '1px solid rgba(255, 0, 0, 0.5)',
                            backgroundColor:
                              isSelected || isMultiSelected
                                ? 'rgba(78, 201, 176, 0.2)'
                                : 'transparent',
                            cursor: 'pointer',
                            pointerEvents: 'auto',
                            boxSizing: 'border-box',
                          }}
                          onMouseEnter={(e) => {
                            if (!isSelected && !isMultiSelected) {
                              e.currentTarget.style.backgroundColor =
                                'rgba(78, 201, 176, 0.41)';
                            }
                          }}
                          onMouseLeave={(e) => {
                            if (!isSelected && !isMultiSelected) {
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
                  display: 'flex',
                  alignItems: 'center',
                  gap: '10px',
                  flexWrap: 'wrap',
                }}
              >
                <div
                  style={{
                    fontSize: '12px',
                    color: '#858585',
                  }}
                >
                  Click on a tile to edit its properties. Drag to select
                  multiple tiles. Ctrl+Click to toggle selection.
                  {selectedTileIndices.length > 0 && (
                    <span style={{ marginLeft: '10px', color: '#4ec9b0' }}>
                      {selectedTileIndices.length} tile
                      {selectedTileIndices.length !== 1 ? 's' : ''} selected
                    </span>
                  )}
                </div>
              </div>
            </div>
          )}

        {selectedTiles.length > 0 && (
          <TileEditModal
            tiles={selectedTiles}
            tilesetName={formData.name}
            onClose={() => {
              setSelectedTileIndex(null);
            }}
            onUpdate={updateTiles}
          />
        )}
      </form>
    </div>
  );
}
