import { useState, useEffect, useRef } from 'react';
import { useAssets } from '../contexts/AssetsContext';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { getEditorState, updateEditorState } from './editorState';
import { OptionSelect } from '../elements/OptionSelect';

export function TilePicker() {
  const { tilesets } = useAssets();
  const { pictures } = useSDL2WAssets();
  const [selectedTilesetName, setSelectedTilesetName] = useState<string>('');
  const [scale, setScale] = useState<number>(1);
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const imageRef = useRef<HTMLImageElement | null>(null);
  const [editorState, setEditorState] = useState(getEditorState());

  // Initialize with first tileset if available
  useEffect(() => {
    if (tilesets.length > 0 && !selectedTilesetName) {
      const firstTileset = tilesets[0];
      setSelectedTilesetName(firstTileset.name);
      // If no tile is selected, select the first tile of the first tileset
      if (editorState.selectedTileIndex === -1 || !editorState.selectedTilesetName) {
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
        if (currentState.selectedTilesetName && currentState.selectedTilesetName !== selectedTilesetName) {
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
  }, [editorState.selectedTileIndex, editorState.selectedTilesetName, selectedTileset, scale]);

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
        height: '200px',
        minHeight: '150px',
        maxHeight: '400px',
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
          <button
            key={tileset.name}
            onClick={() => handleTilesetSelect(tileset.name)}
            style={{
              padding: '8px 16px',
              border: 'none',
              borderBottom:
                selectedTilesetName === tileset.name
                  ? '2px solid #4ec9b0'
                  : '2px solid transparent',
              backgroundColor:
                selectedTilesetName === tileset.name ? '#1e1e1e' : 'transparent',
              color: selectedTilesetName === tileset.name ? '#ffffff' : '#858585',
              cursor: 'pointer',
              whiteSpace: 'nowrap',
              fontSize: '13px',
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
            {tileset.name}
          </button>
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
            width: '96px',
            borderRight: '1px solid #3e3e42',
            backgroundColor: '#252526',
            display: 'flex',
            flexDirection: 'column',
            padding: '10px',
            gap: '15px',
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
        </div>

        {/* Right Column: Canvas Container */}
        <div
          style={{
            flex: 1,
            overflow: 'auto',
            padding: '10px',
            display: 'flex',
            justifyContent: 'center',
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
            <div style={{ color: '#858585', fontSize: '14px', padding: '20px' }}>
              Select a tileset
            </div>
          )}
        </div>
      </div>
    </div>
  );
}

