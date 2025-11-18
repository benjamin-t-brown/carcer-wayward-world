import { useState } from 'react';
import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { Button } from '../elements/Button';
import { CarcerMapTemplate, createDefaultMap } from './MapTemplateForm';

interface CreateMapModalProps {
  isOpen: boolean;
  onConfirm: (map: CarcerMapTemplate) => void;
  onCancel: () => void;
}

export function CreateMapModal({
  isOpen,
  onConfirm,
  onCancel,
}: CreateMapModalProps) {
  const [formData, setFormData] = useState<CarcerMapTemplate>(createDefaultMap());

  if (!isOpen) {
    return null;
  }

  const handleConfirm = () => {
    // Basic validation
    if (!formData.name || !formData.label || formData.width <= 0 || formData.height <= 0) {
      return;
    }
    onConfirm(formData);
    // Reset form
    setFormData(createDefaultMap());
  };

  const handleCancel = () => {
    setFormData(createDefaultMap());
    onCancel();
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
          Create New Map
        </h2>
        <div style={{ marginBottom: '20px' }}>
          <TextInput
            id="create-map-name"
            name="name"
            label="Name"
            value={formData.name}
            onChange={(value) => setFormData({ ...formData, name: value })}
            placeholder="e.g., town_map"
            required
          />
          <TextInput
            id="create-map-label"
            name="label"
            label="Label"
            value={formData.label}
            onChange={(value) => setFormData({ ...formData, label: value })}
            placeholder="e.g., Town Map"
            required
          />
          <NumberInput
            id="create-map-width"
            name="width"
            label="Width"
            value={formData.width}
            onChange={(value) => setFormData({ ...formData, width: value })}
            min={1}
            required
          />
          <NumberInput
            id="create-map-height"
            name="height"
            label="Height"
            value={formData.height}
            onChange={(value) => setFormData({ ...formData, height: value })}
            min={1}
            required
          />
        </div>
        <div
          style={{
            display: 'flex',
            gap: '10px',
            justifyContent: 'flex-end',
          }}
        >
          <Button variant="secondary" onClick={handleCancel}>
            Cancel
          </Button>
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
            Create
          </Button>
        </div>
      </div>
    </div>
  );
}

