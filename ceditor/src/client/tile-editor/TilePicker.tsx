import { useState, useEffect, useRef } from 'react';
import { useAssets } from '../contexts/AssetsContext';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { getEditorState, updateEditorState } from './editorState';
import { OptionSelect } from '../elements/OptionSelect';
import { TileEditModal } from '../components/TileEditModal';
import { TileMetadata } from '../components/TilesetTemplateForm';
import { Sprite } from '../elements/Sprite';

export function TilePicker() {
  const { tilesets, setTilesets } = useAssets();
  const { pictures, sprites } = useSDL2WAssets();
  const [selectedTilesetName, setSelectedTilesetName] = useState<string>('');
  const [scale, setScale] = useState<number>(1);
  const [isEditModalOpen, setIsEditModalOpen] = useState(false);
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const imageRef = useRef<HTMLImageElement | null>(null);
  const [editorState, setEditorState] = useState(getEditorState());

  // Initialize with first tileset if available
  useEffect(() => {
    if (tilesets.length > 0 && !selectedTilesetName) {
      const firstTileset = tilesets[0];
      setSelectedTilesetName(firstTileset.name);
      // If no tile is selected, select the first tile of the first tileset
      if (
        editorState.selectedTileIndex === -1 ||
        !editorState.selectedTilesetName
      ) {
        updateEditorState({
          selectedTilesetName: firstTileset.name,
          selectedTileIndex: 0,
        });
        setEditorState(getEditorState());
      }
    }
  }, [tilesets, selectedTilesetName, editorState]);

  // Sync with editorState changes
  useEffect(() => {
    const interval = setInterval(() => {
      const currentState = getEditorState();
      if (
        currentState.selectedTilesetName !== editorState.selectedTilesetName ||
        currentState.selectedTileIndex !== editorState.selectedTileIndex
      ) {
        setEditorState(currentState);
        // Update selected tileset tab if needed
        if (
          currentState.selectedTilesetName &&
          currentState.selectedTilesetName !== selectedTilesetName
        ) {
          setSelectedTilesetName(currentState.selectedTilesetName);
        }
      }
    }, 100);

    return () => clearInterval(interval);
  }, [editorState, selectedTilesetName]);

  const selectedTileset = tilesets.find((t) => t.name === selectedTilesetName);

  // Draw tileset function
  const drawTileset = () => {
    if (!selectedTileset || !canvasRef.current || !imageRef.current) return;

    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const img = imageRef.current;
    const currentState = getEditorState();
    const { tileWidth, tileHeight, imageWidth, imageHeight } = selectedTileset;

    // Set canvas size to fit the scaled image
    const scaledWidth = imageWidth * scale;
    const scaledHeight = imageHeight * scale;
    canvas.width = scaledWidth;
    canvas.height = scaledHeight;

    // Clear and draw scaled image
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.imageSmoothingEnabled = false;
    ctx.drawImage(img, 0, 0, scaledWidth, scaledHeight);

    // Draw grid lines (scaled)
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.1)';
    ctx.lineWidth = 1;

    const scaledTileWidth = tileWidth * scale;
    const scaledTileHeight = tileHeight * scale;

    // Vertical lines
    for (let x = 0; x <= scaledWidth; x += scaledTileWidth) {
      ctx.beginPath();
      ctx.moveTo(x, 0);
      ctx.lineTo(x, scaledHeight);
      ctx.stroke();
    }

    // Horizontal lines
    for (let y = 0; y <= scaledHeight; y += scaledTileHeight) {
      ctx.beginPath();
      ctx.moveTo(0, y);
      ctx.lineTo(scaledWidth, y);
      ctx.stroke();
    }

    // Highlight selected tile
    if (
      currentState.selectedTilesetName === selectedTileset.name &&
      currentState.selectedTileIndex >= 0
    ) {
      const tilesWide = Math.floor(imageWidth / tileWidth);
      const tileIndex = currentState.selectedTileIndex;
      const tileX = (tileIndex % tilesWide) * scaledTileWidth;
      const tileY = Math.floor(tileIndex / tilesWide) * scaledTileHeight;

      // Draw highlight border
      ctx.strokeStyle = '#4ec9b0';
      ctx.lineWidth = 3;
      ctx.strokeRect(tileX, tileY, scaledTileWidth, scaledTileHeight);

      // Draw highlight overlay
      ctx.fillStyle = 'rgba(78, 201, 176, 0.2)';
      ctx.fillRect(tileX, tileY, scaledTileWidth, scaledTileHeight);
    }
  };

  // Load and draw tileset image
  useEffect(() => {
    if (!selectedTileset || !canvasRef.current) return;

    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const picturePath = pictures[selectedTileset.spriteBase];
    if (!picturePath) {
      // Clear canvas if no picture
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      return;
    }

    // Load image
    const img = new Image();
    img.src = `/api/${picturePath}`;
    img.onload = () => {
      imageRef.current = img;
      drawTileset();
    };
    img.onerror = () => {
      imageRef.current = null;
      ctx.clearRect(0, 0, canvas.width, canvas.height);
    };
  }, [selectedTileset, pictures]);

  // Redraw when editorState or scale changes
  useEffect(() => {
    if (imageRef.current) {
      drawTileset();
    }
  }, [
    editorState.selectedTileIndex,
    editorState.selectedTilesetName,
    selectedTileset,
    scale,
  ]);

  const handleCanvasClick = (e: React.MouseEvent<HTMLCanvasElement>) => {
    if (!selectedTileset || !canvasRef.current) return;

    const canvas = canvasRef.current;
    const rect = canvas.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;

    const { tileWidth, tileHeight, imageWidth } = selectedTileset;
    const scaledTileWidth = tileWidth * scale;
    const scaledTileHeight = tileHeight * scale;
    const tilesWide = Math.floor(imageWidth / tileWidth);
    const tileX = Math.floor(x / scaledTileWidth);
    const tileY = Math.floor(y / scaledTileHeight);

    if (tileX >= 0 && tileY >= 0 && tileX < tilesWide) {
      const tileIndex = tileY * tilesWide + tileX;
      const maxTiles = selectedTileset.tiles.length;
      if (tileIndex >= 0 && tileIndex < maxTiles) {
        updateEditorState({
          selectedTilesetName: selectedTileset.name,
          selectedTileIndex: tileIndex,
        });
        setEditorState(getEditorState());
      }
    }
  };

  const handleTilesetSelect = (tilesetName: string) => {
    setSelectedTilesetName(tilesetName);
    const tileset = tilesets.find((t) => t.name === tilesetName);
    if (tileset && tileset.tiles.length > 0) {
      updateEditorState({
        selectedTilesetName: tilesetName,
        selectedTileIndex: 0,
      });
      setEditorState(getEditorState());
    }
  };

  const handleUpdateTile = (
    index: number,
    field: keyof TileMetadata,
    value: any
  ) => {
    if (!selectedTileset) return;

    const tilesetIndex = tilesets.findIndex(
      (t) => t.name === selectedTileset.name
    );
    if (tilesetIndex >= 0) {
      const updatedTilesets = [...tilesets];
      const updatedTiles = [...selectedTileset.tiles];
      updatedTiles[index] = { ...updatedTiles[index], [field]: value };
      updatedTilesets[tilesetIndex] = {
        ...selectedTileset,
        tiles: updatedTiles,
      };
      setTilesets(updatedTilesets);
    }
  };

  const selectedTile =
    selectedTileset && editorState.selectedTileIndex >= 0
      ? selectedTileset.tiles[editorState.selectedTileIndex]
      : null;

  // Find the sprite for the selected tile
  const selectedTileSprite =
    selectedTile && selectedTileset
      ? sprites.find(
          (s) => s.name === `${selectedTileset.name}_${selectedTile.id}`
        )
      : null;

  const selectedTileId =
    selectedTile && selectedTileset
      ? `${selectedTileset.name}_${selectedTile.id}`
      : null;

  if (tilesets.length === 0) {
    return (
      <div
        style={{
          height: '200px',
          minHeight: '150px',
          maxHeight: '400px',
          borderTop: '1px solid #3e3e42',
          backgroundColor: '#1e1e1e',
          padding: '15px',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
        }}
      >
        <div style={{ color: '#858585', fontSize: '14px' }}>
          No tilesets available
        </div>
      </div>
    );
  }

  return (
    <div
      style={{
        height: '250px',
        minHeight: '150px',
        maxHeight: '600px',
        borderTop: '1px solid #3e3e42',
        backgroundColor: '#1e1e1e',
        display: 'flex',
        flexDirection: 'column',
        overflow: 'hidden',
        resize: 'vertical',
      }}
    >
      {/* Tileset Tabs */}
      <div
        style={{
          display: 'flex',
          borderBottom: '1px solid #3e3e42',
          backgroundColor: '#252526',
          overflowX: 'auto',
          flexShrink: 0,
        }}
      >
        {tilesets.map((tileset) => (
          <div
            key={tileset.name}
            style={{
              display: 'flex',
              alignItems: 'center',
              borderBottom:
                selectedTilesetName === tileset.name
                  ? '2px solid #4ec9b0'
                  : '2px solid transparent',
              backgroundColor:
                selectedTilesetName === tileset.name
                  ? '#1e1e1e'
                  : 'transparent',
            }}
            onMouseEnter={(e) => {
              if (selectedTilesetName !== tileset.name) {
                e.currentTarget.style.backgroundColor = '#2d2d30';
              }
            }}
            onMouseLeave={(e) => {
              if (selectedTilesetName !== tileset.name) {
                e.currentTarget.style.backgroundColor = 'transparent';
              }
            }}
          >
            <button
              onClick={() => handleTilesetSelect(tileset.name)}
              style={{
                padding: '8px 16px',
                border: 'none',
                backgroundColor: 'transparent',
                color:
                  selectedTilesetName === tileset.name ? '#ffffff' : '#858585',
                cursor: 'pointer',
                whiteSpace: 'nowrap',
                fontSize: '13px',
              }}
            >
              {tileset.name}
            </button>
            <button
              onClick={(e) => {
                e.stopPropagation();
                const url = `${window.location.origin}${
                  window.location.pathname
                }#/editor/tilesetTemplates?tileset=${encodeURIComponent(
                  tileset.name
                )}`;
                window.open(url, '_blank');
              }}
              style={{
                padding: '4px 8px',
                border: 'none',
                backgroundColor: 'transparent',
                color:
                  selectedTilesetName === tileset.name ? '#858585' : '#666666',
                cursor: 'pointer',
                fontSize: '14px',
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'center',
                transition: 'color 0.2s',
              }}
              onMouseEnter={(e) => {
                e.currentTarget.style.color = '#4ec9b0';
              }}
              onMouseLeave={(e) => {
                e.currentTarget.style.color =
                  selectedTilesetName === tileset.name ? '#858585' : '#666666';
              }}
              title="Edit tileset in new tab"
            >
              ✎
            </button>
          </div>
        ))}
      </div>

      {/* Main Content: Control Panel + Canvas */}
      <div
        style={{
          flex: 1,
          display: 'flex',
          overflow: 'hidden',
        }}
      >
        {/* Left Column: Control Panel */}
        <div
          style={{
            width: '132px',
            borderRight: '1px solid #3e3e42',
            backgroundColor: '#252526',
            display: 'flex',
            flexDirection: 'column',
            padding: '10px',
            gap: '0px',
            flexShrink: 0,
          }}
        >
          <div
            style={{
              display: 'flex',
              flexDirection: 'column',
              gap: '5px',
            }}
          >
            <label
              style={{
                color: '#858585',
                fontSize: '11px',
                textTransform: 'uppercase',
                fontWeight: 'bold',
              }}
            >
              Scale
            </label>
            <OptionSelect
              value={scale.toString()}
              onChange={(value) => setScale(parseFloat(value))}
              style={{ marginBottom: '0px' }}
              options={[
                { value: '1', label: '1x' },
                { value: '1.5', label: '1.5x' },
                { value: '2', label: '2x' },
                { value: '2.5', label: '2.5x' },
                { value: '3', label: '3x' },
                { value: '3.5', label: '3.5x' },
                { value: '4', label: '4x' },
                { value: '4.5', label: '4.5x' },
                { value: '5', label: '5x' },
              ]}
            />
          </div>
          {selectedTileset && selectedTile && (
            <div
              style={{
                display: 'flex',
                flexDirection: 'column',
                gap: '10px',
              }}
            >
              {/* Tile Information */}
              <div
                style={{
                  display: 'flex',
                  flexDirection: 'column',
                  gap: '8px',
                  padding: '8px',
                  backgroundColor: '#1e1e1e',
                  borderRadius: '4px',
                  border: '1px solid #3e3e42',
                }}
              >
                {selectedTileId && (
                  <div
                    style={{
                      color: '#d4d4d4',
                      fontSize: '11px',
                      fontFamily: 'monospace',
                      wordBreak: 'break-all',
                    }}
                  >
                    {selectedTileId}
                  </div>
                )}
                {selectedTileSprite && (
                  <div
                    style={{
                      display: 'flex',
                      justifyContent: 'center',
                      padding: '8px',
                      backgroundColor: '#2d2d30',
                      borderRadius: '4px',
                      border: '1px solid #3e3e42',
                    }}
                  >
                    <Sprite sprite={selectedTileSprite} scale={1} />
                  </div>
                )}
              </div>
              <button
                onClick={() => setIsEditModalOpen(true)}
                style={{
                  padding: '8px',
                  border: '1px solid #3e3e42',
                  backgroundColor: '#3e3e42',
                  color: '#ffffff',
                  cursor: 'pointer',
                  fontSize: '12px',
                  borderRadius: '2px',
                  transition: 'background-color 0.2s',
                }}
                onMouseEnter={(e) => {
                  e.currentTarget.style.backgroundColor = '#4a4a4a';
                }}
                onMouseLeave={(e) => {
                  e.currentTarget.style.backgroundColor = '#3e3e42';
                }}
                title="Edit selected tile"
              >
                Edit Tile
              </button>
            </div>
          )}
        </div>

        {/* Right Column: Canvas Container */}
        <div
          style={{
            flex: 1,
            overflow: 'auto',
            padding: '10px',
            display: 'flex',
            justifyContent: 'flex-start',
            alignItems: 'flex-start',
          }}
        >
          {selectedTileset ? (
            <canvas
              ref={canvasRef}
              onClick={handleCanvasClick}
              style={{
                cursor: 'pointer',
                imageRendering: 'pixelated',
                border: '1px solid #3e3e42',
                borderRadius: '4px',
              }}
            />
          ) : (
            <div
              style={{ color: '#858585', fontSize: '14px', padding: '20px' }}
            >
              Select a tileset
            </div>
          )}
        </div>
      </div>

      {/* Tile Edit Modal */}
      {isEditModalOpen && selectedTileset && selectedTile && (
        <TileEditModal
          tile={selectedTile}
          tileIndex={editorState.selectedTileIndex}
          tilesetName={selectedTileset.name}
          onClose={() => setIsEditModalOpen(false)}
          onUpdate={handleUpdateTile}
        />
      )}
    </div>
  );
}
