import { useState } from 'react';
import { TextInput } from '../elements/TextInput';
import { OptionSelect } from '../elements/OptionSelect';
import { SpritePicker } from '../elements/SpritePicker';
import { Button } from '../elements/Button';
import { GameEvent, createDefaultGameEvent } from './GameEventForm';

interface CreateGameEventModalProps {
  isOpen: boolean;
  onConfirm: (gameEvent: GameEvent) => void;
  onCancel: () => void;
}

const GAME_EVENT_TYPES = ['MODAL', 'TALK', 'TRAVEL'];

export function CreateGameEventModal({
  isOpen,
  onConfirm,
  onCancel,
}: CreateGameEventModalProps) {
  const [formData, setFormData] = useState<GameEvent>(createDefaultGameEvent());

  if (!isOpen) {
    return null;
  }

  const handleConfirm = () => {
    // Basic validation
    if (!formData.id || !formData.title || !formData.eventType || !formData.icon) {
      return;
    }
    onConfirm(formData);
    // Reset form
    setFormData(createDefaultGameEvent());
  };

  const handleCancel = () => {
    setFormData(createDefaultGameEvent());
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
      onClick={handleCancel}
    >
      <div
        style={{
          backgroundColor: '#252526',
          border: '1px solid #3e3e42',
          borderRadius: '8px',
          padding: '30px',
          maxWidth: '600px',
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
          Create New Game Event
        </h2>
        <div style={{ marginBottom: '20px' }}>
          <TextInput
            id="create-game-event-id"
            name="id"
            label="ID"
            value={formData.id}
            onChange={(value) => setFormData({ ...formData, id: value })}
            placeholder="e.g., exampleEvent"
            required
          />

          <TextInput
            id="create-game-event-title"
            name="title"
            label="Title"
            value={formData.title}
            onChange={(value) => setFormData({ ...formData, title: value })}
            placeholder="e.g., Example Event"
            required
          />

          <OptionSelect
            id="create-game-event-type"
            name="eventType"
            label="Event Type"
            value={formData.eventType}
            onChange={(value) =>
              setFormData({ ...formData, eventType: value as 'MODAL' | 'TALK' | 'TRAVEL' })
            }
            options={GAME_EVENT_TYPES.map((type) => ({
              value: type,
              label: type,
            }))}
            required
          />

          <div className="form-group" style={{ marginTop: '15px' }}>
            <label htmlFor="create-game-event-icon">Icon *</label>
            <div style={{ marginTop: '8px' }}>
              <SpritePicker
                value={formData.icon}
                onChange={(value) => setFormData({ ...formData, icon: value })}
                scale={2}
                maxHeight="300px"
              />
            </div>
            {formData.icon && (
              <div
                style={{ marginTop: '8px', fontSize: '12px', color: '#858585' }}
              >
                Selected: {formData.icon}
              </div>
            )}
          </div>
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
              !formData.id ||
              !formData.title ||
              !formData.eventType ||
              !formData.icon
            }
          >
            Create
          </Button>
          <Button variant="secondary" onClick={handleCancel}>
            Cancel
          </Button>
        </div>
      </div>
    </div>
  );
}

