import { useState, useEffect, useRef } from 'react';
import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { Button } from '../elements/Button';
import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
} from '../types/assets';
import { createDefaultCarcerMapTile } from './MapTemplateForm';

interface EditMapModalProps {
  isOpen: boolean;
  map: CarcerMapTemplate | null;
  onConfirm: (map: CarcerMapTemplate) => void;
  onCancel: () => void;
  onDelete?: () => void;
}

export function EditMapModal({
  isOpen,
  map,
  onConfirm,
  onCancel,
  onDelete,
}: EditMapModalProps) {
  const [formData, setFormData] = useState<CarcerMapTemplate | null>(null);
  const [previousDimensions, setPreviousDimensions] = useState([0, 0]);

  // Update form data when map changes
  useEffect(() => {
    if (map) {
      setFormData({ ...map });
      setPreviousDimensions([map.width, map.height]);
    }
  }, [map]);

  if (!isOpen || !map || !formData) {
    return null;
  }

  const handleConfirm = () => {
    // Basic validation
    if (
      !formData.name ||
      !formData.label ||
      formData.width <= 0 ||
      formData.height <= 0
    ) {
      return;
    }

    // Handle width/height changes and update tiles accordingly
    const handleDimensionChange = (
      newWidth: number,
      newHeight: number,
      currentData: CarcerMapTemplate
    ): CarcerMapTemplate => {
      // Create a set of existing tile positions for quick lookup
      const existingTiles = new Map<string, CarcerMapTileTemplate>();
      const [prevWidth, prevHeight] = previousDimensions ?? [currentData.width, currentData.height];
      for (let i = 0; i < prevHeight; i++) {
        for (let j = 0; j < prevWidth; j++) {
          const key = `${j},${i}`;
          existingTiles.set(key, currentData.tiles[i * prevWidth + j]);
        }
      }

      const nextTiles: CarcerMapTileTemplate[] = [];

      for (let i = 0; i < newHeight; i++) {
        for (let j = 0; j < newWidth; j++) {
          const key = `${j},${i}`;
          const existingTile = existingTiles.get(key);
          if (!existingTile) {
            const newTile = createDefaultCarcerMapTile();
            nextTiles.push(newTile);
          } else {
            nextTiles.push(existingTile);
          }
        }
      }

      return {
        ...currentData,
        width: newWidth,
        height: newHeight,
        tiles: nextTiles,
      };
    };

    onConfirm(handleDimensionChange(formData.width, formData.height, formData));
  };

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
          Edit Map
        </h2>
        <div style={{ marginBottom: '20px' }}>
          <TextInput
            id="edit-map-name"
            name="name"
            label="Name"
            value={formData.name}
            onChange={(value) => setFormData({ ...formData, name: value })}
            placeholder="e.g., town_map"
            required
          />
          <TextInput
            id="edit-map-label"
            name="label"
            label="Label"
            value={formData.label}
            onChange={(value) => setFormData({ ...formData, label: value })}
            placeholder="e.g., Town Map"
            required
          />
          <NumberInput
            id="edit-map-width"
            name="width"
            label="Width"
            value={formData.width}
            onChange={(value) => {
              if (formData) {
                setFormData({ ...formData, width: value });
              }
            }}
            min={1}
            required
          />
          <NumberInput
            id="edit-map-height"
            name="height"
            label="Height"
            value={formData.height}
            onChange={(value) => {
              if (formData) {
                setFormData({ ...formData, height: value });
              }
            }}
            min={1}
            required
          />
        </div>
        <div
          style={{
            display: 'flex',
            gap: '10px',
            justifyContent: 'space-between',
            alignItems: 'center',
          }}
        >
          <div>
            {onDelete && (
              <Button
                variant="primary"
                className="btn-danger"
                onClick={() => {
                  onDelete();
                  onCancel();
                }}
              >
                Delete
              </Button>
            )}
          </div>
          <div style={{ display: 'flex', gap: '10px' }}>
            <Button
              variant="primary"
              onClick={handleConfirm}
              disabled={
                !formData.name ||
                !formData.label ||
                formData.width <= 0 ||
                formData.height <= 0
              }
            >
              Save
            </Button>
            <Button variant="secondary" onClick={onCancel}>
              Cancel
            </Button>
          </div>
        </div>
      </div>
    </div>
  );
}
