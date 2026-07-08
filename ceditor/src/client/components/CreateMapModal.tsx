import { useEffect, useState } from 'react';
import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { OptionSelect } from '../elements/OptionSelect';
import { Button } from '../elements/Button';
import { CarcerMapTemplate, MAP_TYPES, MapType } from '../types/assets';
import { createDefaultMap } from './MapTemplateForm';
import { MODAL_ROOT_CLASS, useEscapeToClose } from '../hooks/useEscapeToClose';

export interface CreateMapConstraints {
  width?: number;
  height?: number;
  mapType?: MapType;
  lockDimensions?: boolean;
  lockType?: boolean;
  defaultName?: string;
  defaultLabel?: string;
}

interface CreateMapModalProps {
  isOpen: boolean;
  onConfirm: (map: CarcerMapTemplate) => void;
  onCancel: () => void;
  constraints?: CreateMapConstraints;
  existingMapNames?: string[];
  isSubmitting?: boolean;
}

function buildInitialFormData(constraints?: CreateMapConstraints): CarcerMapTemplate {
  const base = createDefaultMap();
  if (!constraints) {
    return base;
  }
  return {
    ...base,
    width: constraints.width ?? base.width,
    height: constraints.height ?? base.height,
    type: constraints.mapType ?? base.type,
    name: constraints.defaultName ?? base.name,
    label: constraints.defaultLabel ?? base.label,
  };
}

export function CreateMapModal({
  isOpen,
  onConfirm,
  onCancel,
  constraints,
  existingMapNames = [],
  isSubmitting = false,
}: CreateMapModalProps) {
  const [formData, setFormData] =
    useState<CarcerMapTemplate>(buildInitialFormData(constraints));
  const [nameError, setNameError] = useState<string | null>(null);

  useEffect(() => {
    if (isOpen) {
      setFormData(buildInitialFormData(constraints));
      setNameError(null);
    }
  }, [isOpen, constraints]);

  const modalRef = useEscapeToClose(() => {
    setFormData(buildInitialFormData(constraints));
    setNameError(null);
    onCancel();
  }, isOpen);

  if (!isOpen) {
    return null;
  }

  const handleConfirm = () => {
    const trimmedName = formData.name.trim();
    if (!trimmedName || !formData.label.trim()) {
      return;
    }
    if (existingMapNames.includes(trimmedName)) {
      setNameError('A map with this name already exists.');
      return;
    }
    onConfirm({ ...formData, name: trimmedName, label: formData.label.trim() });
    setFormData(buildInitialFormData(constraints));
    setNameError(null);
  };

  const handleCancel = () => {
    setFormData(buildInitialFormData(constraints));
    setNameError(null);
    onCancel();
  };

  const lockDimensions = constraints?.lockDimensions ?? false;
  const lockType = constraints?.lockType ?? false;

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
        zIndex: 1100,
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
        {constraints?.width !== undefined && constraints?.height !== undefined && (
          <div
            style={{
              marginBottom: '16px',
              fontSize: '12px',
              color: '#858585',
            }}
          >
            Map must be {constraints.width}×{constraints.height} tiles
            {constraints.mapType ? ` (${constraints.mapType})` : ''}.
          </div>
        )}
        <div style={{ marginBottom: '20px' }}>
          <TextInput
            id="create-map-name"
            name="name"
            label="Name"
            value={formData.name}
            onChange={(value) => {
              setNameError(null);
              setFormData({ ...formData, name: value });
            }}
            placeholder="e.g., town_map"
            required
          />
          {nameError && (
            <div style={{ color: '#f48771', fontSize: '12px', marginTop: '4px' }}>
              {nameError}
            </div>
          )}
          <TextInput
            id="create-map-label"
            name="label"
            label="Label"
            value={formData.label}
            onChange={(value) => setFormData({ ...formData, label: value })}
            placeholder="e.g., Town Map"
            required
          />
          <OptionSelect
            id="create-map-type"
            name="type"
            label="Type"
            value={formData.type}
            onChange={(value) =>
              setFormData({ ...formData, type: value as MapType })
            }
            options={MAP_TYPES.map((type) => ({
              value: type,
              label: type === 'OUTDOOR' ? 'OUTDOOR' : type,
            }))}
            required
            disabled={lockType}
          />
          <NumberInput
            id="create-map-width"
            name="width"
            label="Width"
            value={formData.width}
            onChange={(value) => setFormData({ ...formData, width: value })}
            min={1}
            required
            disabled={lockDimensions}
          />
          <NumberInput
            id="create-map-height"
            name="height"
            label="Height"
            value={formData.height}
            onChange={(value) => setFormData({ ...formData, height: value })}
            min={1}
            required
            disabled={lockDimensions}
          />
        </div>
        <div
          style={{
            display: 'flex',
            gap: '10px',
            justifyContent: 'flex-end',
          }}
        >
          <Button
            variant="primary"
            onClick={handleConfirm}
            disabled={
              isSubmitting ||
              !formData.name.trim() ||
              !formData.label.trim() ||
              formData.width <= 0 ||
              formData.height <= 0
            }
          >
            {isSubmitting ? 'Creating…' : 'Create'}
          </Button>
          <Button variant="secondary" onClick={handleCancel} disabled={isSubmitting}>
            Cancel
          </Button>
        </div>
      </div>
    </div>
  );
}
