import { useState, useEffect } from 'react';
import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { OptionSelect } from '../elements/OptionSelect';
import { Button } from '../elements/Button';
import {
  CarcerMapTemplate,
  MAP_TYPES,
  MapType,
} from '../types/assets';
import { createDefaultCarcerMapTile } from './MapTemplateForm';
import { MODAL_ROOT_CLASS, useEscapeToClose } from '../hooks/useEscapeToClose';
import { resizeMap } from '../utils/mapIndex';

interface EditMapModalProps {
  isOpen: boolean;
  map: CarcerMapTemplate | null;
  onConfirm: (map: CarcerMapTemplate) => void;
  onCancel: () => void;
  onDelete?: () => void;
  onDuplicate?: (map: CarcerMapTemplate) => void;
}

export function EditMapModal({
  isOpen,
  map,
  onConfirm,
  onCancel,
  onDelete,
  onDuplicate,
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

  const modalRef = useEscapeToClose(onCancel, isOpen && !!map && !!formData);

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

    const [prevWidth, prevHeight] = previousDimensions;
    onConfirm(
      resizeMap(
        formData,
        prevWidth,
        prevHeight,
        formData.width,
        formData.height,
        createDefaultCarcerMapTile
      )
    );
  };

  const handleDuplicate = () => {
    if (!formData || !onDuplicate) {
      return;
    }

    const [prevWidth, prevHeight] = previousDimensions;
    onDuplicate(
      resizeMap(
        formData,
        prevWidth,
        prevHeight,
        formData.width,
        formData.height,
        createDefaultCarcerMapTile
      )
    );
  };

  return (
    <div
      ref={modalRef}
      className={MODAL_ROOT_CLASS}
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
          <OptionSelect
            id="edit-map-type"
            name="type"
            label="Type"
            value={formData.type}
            onChange={(value) =>
              setFormData({ ...formData, type: value as MapType })
            }
            options={MAP_TYPES.map((type) => ({ value: type, label: type }))}
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
          <div style={{ display: 'flex', gap: '10px' }}>
            {onDuplicate && (
              <Button variant="secondary" onClick={handleDuplicate}>
                Duplicate
              </Button>
            )}
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
