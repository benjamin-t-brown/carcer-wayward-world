import { useState, useEffect } from 'react';
import { GameEvent } from '../../types/assets';
import { Button } from '../../elements/Button';
import { VariableWidget } from '../react-components/VariableWidget';
import { EditorNodeEnd } from '../cmpts/EndNodeComponent';
import { notifyStateUpdated } from '../seEditorState';

interface EditEndNodeModalProps {
  isOpen: boolean;
  node: EditorNodeEnd | undefined;
  gameEvent: GameEvent;
  onCancel: () => void;
}

export function EditEndNodeModal({
  isOpen,
  node,
  gameEvent,
  onCancel,
}: EditEndNodeModalProps) {
  const [next, setNext] = useState('');

  useEffect(() => {
    if (node) {
      const seNode = node.toSENode();
      setNext(seNode.next || '');
    }
  }, [node]);

  if (!isOpen || !node) {
    return null;
  }

  const handleEditNodeConfirm = () => {
    // EndNode doesn't have editable properties beyond next
    // The next field is stored in the SENode but EndNode doesn't use connectors
    // So we'll just close the modal
    notifyStateUpdated();
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
        zIndex: 1001,
      }}
    >
      <div
        style={{
          backgroundColor: '#252526',
          border: '1px solid #3e3e42',
          borderRadius: '8px',
          padding: '30px',
          maxWidth: '90vw',
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
          Edit End Node
        </h2>
        <div>
          <VariableWidget gameEvent={gameEvent} />
        </div>

        <div
          style={{
            color: '#999',
            marginBottom: '20px',
            fontSize: '14px',
          }}
        >
          GameEvent: <span style={{ color: '#00d4d4' }}>{gameEvent.id}</span> |
          Node: <span style={{ color: '#d4d400' }}>{node.id}</span>
        </div>

        <div
          style={{
            padding: '20px',
            backgroundColor: '#1e1e1e',
            border: '1px solid #3e3e42',
            borderRadius: '4px',
            marginBottom: '20px',
          }}
        >
          <p style={{ color: '#d4d4d4', margin: 0 }}>
            This is an End Node. It represents the end of a game event flow.
          </p>
        </div>

        <div
          style={{
            display: 'flex',
            gap: '10px',
            justifyContent: 'flex-end',
          }}
        >
          <Button variant="primary" onClick={handleEditNodeConfirm}>
            Close
          </Button>
          <Button variant="secondary" onClick={onCancel}>
            Cancel
          </Button>
        </div>
      </div>
    </div>
  );
}
