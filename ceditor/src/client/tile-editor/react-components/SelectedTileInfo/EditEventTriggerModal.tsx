import { useEffect, useState } from 'react';
import {
  GameEvent,
  TILE_OVERLAY_VISIBILITY_OPTIONS,
  TileEventTrigger,
  TileOverlayVisibility,
} from '../../../types/assets';
import { Button } from '../../../elements/Button';
import { SoundSearchField } from '../../../elements/SoundSearchField';
import { TextInput } from '../../../elements/TextInput';
import { MODAL_ROOT_CLASS, useEscapeToClose } from '../../../hooks/useEscapeToClose';
import { OverrideCheckbox } from './OverrideCheckbox';
import {
  ConditionEditButton,
  EditTileEventConditionModal,
} from './EditTileEventConditionModal';

export interface EditEventTriggerResult {
  title: string;
  requiresNonCombat: boolean;
  requiresLook: boolean;
  activationSound: string;
  condition: string;
  overlayVisibility: TileOverlayVisibility;
}

interface EditEventTriggerModalProps {
  isOpen: boolean;
  trigger: TileEventTrigger;
  eventTitle: string;
  gameEvent: GameEvent | null;
  onSave: (result: EditEventTriggerResult) => void;
  onCancel: () => void;
  isSaving?: boolean;
}

export function EditEventTriggerModal({
  isOpen,
  trigger,
  eventTitle,
  gameEvent,
  onSave,
  onCancel,
  isSaving = false,
}: EditEventTriggerModalProps) {
  const [title, setTitle] = useState(eventTitle);
  const [requiresNonCombat, setRequiresNonCombat] = useState(true);
  const [requiresLook, setRequiresLook] = useState(false);
  const [activationSound, setActivationSound] = useState('event');
  const [condition, setCondition] = useState('');
  const [overlayVisibility, setOverlayVisibility] =
    useState<TileOverlayVisibility>('HIDDEN');
  const [isConditionModalOpen, setIsConditionModalOpen] = useState(false);

  const modalRef = useEscapeToClose(onCancel, isOpen && !isConditionModalOpen);

  useEffect(() => {
    if (!isOpen) {
      return;
    }
    setTitle(eventTitle);
    setRequiresNonCombat(trigger.requiresNonCombat ?? true);
    setRequiresLook(trigger.requiresLook ?? false);
    setActivationSound(trigger.activationSound ?? 'event');
    setCondition(trigger.condition ?? '');
    setOverlayVisibility(trigger.overlayVisibility ?? 'HIDDEN');
    setIsConditionModalOpen(false);
  }, [isOpen, trigger, eventTitle]);

  if (!isOpen) {
    return null;
  }

  const handleSave = () => {
    onSave({
      title: title.trim() || trigger.eventId,
      requiresNonCombat,
      requiresLook,
      activationSound,
      condition: condition.trim(),
      overlayVisibility,
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
            marginBottom: '8px',
            marginTop: 0,
          }}
        >
          Edit Event Trigger
        </h2>
        <div
          style={{
            color: '#858585',
            fontSize: '11px',
            marginBottom: '20px',
            overflowWrap: 'anywhere',
          }}
        >
          {trigger.eventId}
        </div>

        <TextInput
          id="edit-event-trigger-title"
          name="eventTitle"
          label="Title"
          value={title}
          onChange={setTitle}
          disabled={isSaving || !gameEvent}
          placeholder="In-game modal header"
        />
        {!gameEvent && (
          <div
            style={{
              color: '#ff6b6b',
              fontSize: '11px',
              marginBottom: '12px',
            }}
          >
            Event not found — title cannot be saved.
          </div>
        )}

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
          <SoundSearchField
            id="edit-event-trigger-activation-sound"
            name="activationSound"
            label="Activation Sound"
            value={activationSound}
            emptyLabel="None (silent)"
            onChange={setActivationSound}
          />
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
          <div>
            <label
              htmlFor="edit-event-trigger-overlay-visibility"
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
              id="edit-event-trigger-overlay-visibility"
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
        </div>

        <div
          style={{
            display: 'flex',
            gap: '10px',
            justifyContent: 'flex-end',
          }}
        >
          <Button variant="primary" onClick={handleSave} disabled={isSaving}>
            {isSaving ? 'Saving…' : 'Save'}
          </Button>
          <Button variant="secondary" onClick={onCancel} disabled={isSaving}>
            Cancel
          </Button>
        </div>
      </div>
      <EditTileEventConditionModal
        isOpen={isConditionModalOpen}
        condition={condition}
        gameEvent={gameEvent}
        onConfirm={(nextCondition) => {
          setCondition(nextCondition);
          setIsConditionModalOpen(false);
        }}
        onCancel={() => setIsConditionModalOpen(false)}
      />
    </div>
  );
}
