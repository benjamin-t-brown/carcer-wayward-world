import { TextInput } from '../elements/TextInput';
import { Button } from '../elements/Button';
import { Sprite } from '../elements/Sprite';
import { TileMetadata } from './TilesetTemplateForm';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';

interface TileEditModalProps {
  tile: TileMetadata | null;
  tileIndex: number | null;
  tilesetName: string;
  onClose: () => void;
  onUpdate: (index: number, field: keyof TileMetadata, value: any) => void;
}

export function TileEditModal({
  tile,
  tileIndex,
  tilesetName,
  onClose,
  onUpdate,
}: TileEditModalProps) {
  const { sprites } = useSDL2WAssets();

  if (!tile || tileIndex === null) {
    return null;
  }

  const handleUpdate = (field: keyof TileMetadata, value: any) => {
    onUpdate(tileIndex, field, value);
  };

  // Find the sprite for this tile: tilesetName + '_' + tile.id
  const spriteName = `${tilesetName}_${tile.id}`;
  const sprite = sprites.find((s) => s.name === spriteName);

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
      onClick={onClose}
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
          Edit Tile #{tileIndex}
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
            <div style={{ color: '#858585', fontSize: '12px', marginBottom: '4px' }}>
              Sprite Name
            </div>
            <div style={{ color: '#d4d4d4', fontSize: '14px', fontFamily: 'monospace' }}>
              {spriteName}
            </div>
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
              textAlign: 'center',
            }}
          >
            <div style={{ marginBottom: '10px', color: '#d4d4d4', fontSize: '14px' }}>
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
              <Sprite sprite={sprite} scale={4} />
            </div>
          </div>
        )}

        <TextInput
          id="tile-description"
          name="description"
          label="Description"
          value={tile.description || ''}
          onChange={(value) => handleUpdate('description', value)}
          placeholder="Tile description"
          onKeyDown={(e) => {
            if (e.key === 'Enter') {
              onClose();
            }
          }}
        />
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
            checked={tile.isWalkable !== false}
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
            checked={tile.isSeeThrough !== false}
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
            checked={tile.isDoor === true}
            onChange={(e) => handleUpdate('isDoor', e.target.checked)}
            style={{ cursor: 'pointer', transform: 'scale(1.5)' }}
          />
          <label htmlFor="isDoor">Is Door</label>
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
