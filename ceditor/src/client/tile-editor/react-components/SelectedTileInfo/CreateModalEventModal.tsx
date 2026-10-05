import { useEffect, useState } from 'react';
import { TextInput } from '../../../elements/TextInput';
import { TextArea } from '../../../elements/TextArea';
import { Button } from '../../../elements/Button';
import { MODAL_ROOT_CLASS, useEscapeToClose } from '../../../hooks/useEscapeToClose';
import {
  TILE_OVERLAY_VISIBILITY_OPTIONS,
  TileOverlayVisibility,
} from '../../../types/assets';
import { suggestMapEventId } from './suggestMapEventId';
import { DEFAULT_MODAL_NODE_TEXT } from './createModalGameEvent';
import { OverrideCheckbox } from './OverrideCheckbox';
import {
  ConditionEditButton,
  EditTileEventConditionModal,
} from './EditTileEventConditionModal';

export interface CreateModalEventResult {
  triggerId: string;
  text: string;
  requiresNonCombat: boolean;
  requiresLook: boolean;
  overlayVisibility: TileOverlayVisibility;
  condition: string;
}

interface CreateModalEventModalProps {
  isOpen: boolean;
  mapName: string;
  existingEventIds: Set<string>;
  defaultRequiresLook: boolean;
  onConfirm: (result: CreateModalEventResult) => void;
  onCancel: () => void;
}

export function CreateModalEventModal({
  isOpen,
  mapName,
  existingEventIds,
  defaultRequiresLook,
  onConfirm,
  onCancel,
}: CreateModalEventModalProps) {
  const [triggerId, setTriggerId] = useState('');
  const [nodeText, setNodeText] = useState(DEFAULT_MODAL_NODE_TEXT);
  const [requiresNonCombat, setRequiresNonCombat] = useState(true);
  const [requiresLook, setRequiresLook] = useState(false);
  const [overlayVisibility, setOverlayVisibility] =
    useState<TileOverlayVisibility>('HIDDEN');
  const [condition, setCondition] = useState('');
  const [isConditionModalOpen, setIsConditionModalOpen] = useState(false);
  const [error, setError] = useState('');

  const modalRef = useEscapeToClose(onCancel, isOpen);

  useEffect(() => {
    if (!isOpen) {
      return;
    }
    setTriggerId(suggestMapEventId(mapName, existingEventIds));
    setNodeText(DEFAULT_MODAL_NODE_TEXT);
    setRequiresNonCombat(true);
    setRequiresLook(defaultRequiresLook);
    setOverlayVisibility('HIDDEN');
    setCondition('');
    setIsConditionModalOpen(false);
    setError('');
  }, [isOpen, mapName, existingEventIds, defaultRequiresLook]);

  if (!isOpen) {
    return null;
  }

  const handleConfirm = () => {
    const id = triggerId.trim();
    if (!id) {
      setError('Trigger name is required.');
      return;
    }
    if (existingEventIds.has(id)) {
      setError(`A game event with ID "${id}" already exists.`);
      return;
    }
    onConfirm({
      triggerId: id,
      text: nodeText,
      requiresNonCombat,
      requiresLook,
      overlayVisibility,
      condition,
    });
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
          maxWidth: '520px',
          width: '90%',
          maxHeight: '90vh',
          overflowY: 'auto',
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
          Create Modal
        </h2>

        <TextInput
          id="create-modal-trigger-id"
          name="triggerId"
          label="Trigger Name (ID)"
          value={triggerId}
          onChange={(value) => {
            setTriggerId(value);
            setError('');
          }}
          placeholder="e.g., alinea_outsideAlinea1_k3f9a2"
          required
        />

        <TextArea
          id="create-modal-node-text"
          name="nodeText"
          label="Node Text"
          value={nodeText}
          onChange={setNodeText}
          placeholder="Text shown in the event node"
          rows={4}
        />

        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            gap: '8px',
            marginBottom: '16px',
          }}
        >
          <OverrideCheckbox
            value={requiresNonCombat}
            label="Requires Non Combat"
            onChange={(newValue) => setRequiresNonCombat(newValue === true)}
          />
          <OverrideCheckbox
            value={requiresLook}
            label="Requires Look"
            onChange={(newValue) => setRequiresLook(newValue === true)}
          />
          <div>
            <label
              htmlFor="create-modal-overlay-visibility"
              style={{
                color: '#858585',
                fontSize: '10px',
                textTransform: 'uppercase',
                fontWeight: 'bold',
                display: 'block',
                marginBottom: '4px',
              }}
            >
              Tile Overlay Visibility
            </label>
            <select
              id="create-modal-overlay-visibility"
              value={overlayVisibility}
              onChange={(e) =>
                setOverlayVisibility(e.target.value as TileOverlayVisibility)
              }
              style={{
                width: '100%',
                padding: '4px 6px',
                border: '1px solid #3e3e42',
                backgroundColor: '#1e1e1e',
                color: '#ffffff',
                fontSize: '12px',
                borderRadius: '4px',
              }}
            >
              {TILE_OVERLAY_VISIBILITY_OPTIONS.map((option) => (
                <option key={option} value={option}>
                  {option}
                </option>
              ))}
            </select>
          </div>
          <div>
            <div
              style={{
                color: '#858585',
                fontSize: '10px',
                textTransform: 'uppercase',
                fontWeight: 'bold',
                marginBottom: '4px',
              }}
            >
              Condition
            </div>
            <ConditionEditButton
              condition={condition}
              onClick={() => setIsConditionModalOpen(true)}
            />
          </div>
        </div>

        {error && (
          <div
            style={{
              color: '#ff6b6b',
              fontSize: '12px',
              marginBottom: '12px',
            }}
          >
            {error}
          </div>
        )}

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
            disabled={!triggerId.trim()}
          >
            Create
          </Button>
          <Button variant="secondary" onClick={onCancel}>
            Cancel
          </Button>
        </div>
      </div>
      <EditTileEventConditionModal
        isOpen={isConditionModalOpen}
        condition={condition}
        gameEvent={null}
        onConfirm={(nextCondition) => {
          setCondition(nextCondition);
          setIsConditionModalOpen(false);
        }}
        onCancel={() => setIsConditionModalOpen(false)}
      />
    </div>
  );
}
