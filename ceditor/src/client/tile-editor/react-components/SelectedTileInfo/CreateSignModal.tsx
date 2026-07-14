import { useEffect, useState } from 'react';
import { TextInput } from '../../../elements/TextInput';
import { TextArea } from '../../../elements/TextArea';
import { Button } from '../../../elements/Button';
import { MODAL_ROOT_CLASS, useEscapeToClose } from '../../../hooks/useEscapeToClose';
import { suggestSignEventIdPrefix } from './createSignGameEvent';

export interface CreateSignModalResult {
  triggerId: string;
  contents: string;
  title: string;
}

interface CreateSignModalProps {
  isOpen: boolean;
  mapName: string;
  existingEventIds: Set<string>;
  onConfirm: (result: CreateSignModalResult) => void;
  onCancel: () => void;
}

export function CreateSignModal({
  isOpen,
  mapName,
  existingEventIds,
  onConfirm,
  onCancel,
}: CreateSignModalProps) {
  const [triggerId, setTriggerId] = useState('');
  const [contents, setContents] = useState('');
  const [error, setError] = useState('');

  const modalRef = useEscapeToClose(onCancel, isOpen);

  useEffect(() => {
    if (!isOpen) {
      return;
    }
    setTriggerId(suggestSignEventIdPrefix(mapName));
    setContents('');
    setError('');
  }, [isOpen, mapName]);

  if (!isOpen) {
    return null;
  }

  const handleConfirm = () => {
    const id = triggerId.trim();
    const signContents = contents.trim();
    if (!id) {
      setError('Trigger name is required.');
      return;
    }
    if (!signContents) {
      setError('Sign contents are required.');
      return;
    }
    if (existingEventIds.has(id)) {
      setError(`A game event with ID "${id}" already exists.`);
      return;
    }

    const title =
      signContents.split('\n')[0].trim().slice(0, 60) || signContents;

    onConfirm({
      triggerId: id,
      contents: signContents,
      title,
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
          Create Sign
        </h2>

        <TextInput
          id="create-sign-trigger-id"
          name="triggerId"
          label="Trigger Name (ID)"
          value={triggerId}
          onChange={(value) => {
            setTriggerId(value);
            setError('');
          }}
          placeholder="e.g., alinea_sign_warehouse1"
          required
        />

        <TextArea
          id="create-sign-contents"
          name="contents"
          label="Sign Contents"
          value={contents}
          onChange={(value) => {
            setContents(value);
            setError('');
          }}
          placeholder='Text shown on the sign (without quotes)'
          rows={4}
          required
        />

        {contents.trim() && (
          <div
            style={{
              marginTop: '8px',
              marginBottom: '16px',
              padding: '8px 10px',
              backgroundColor: '#1e1e1e',
              border: '1px solid #3e3e42',
              borderRadius: '4px',
              color: '#cccccc',
              fontSize: '12px',
              whiteSpace: 'pre-wrap',
            }}
          >
            Preview: The sign says: &quot;{contents.trim()}&quot;
          </div>
        )}

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
            disabled={!triggerId.trim() || !contents.trim()}
          >
            Create
          </Button>
          <Button variant="secondary" onClick={onCancel}>
            Cancel
          </Button>
        </div>
      </div>
    </div>
  );
}
