import { useState, useEffect } from 'react';
import { GameEvent, GameEventChildExec } from '../../types/assets';
import { Button } from '../../elements/Button';
import { VariableWidget } from '../VariableWidget';

interface EditExecNodeModalProps {
  isOpen: boolean;
  node: GameEventChildExec | null;
  gameEvent: GameEvent;
  updateGameEvent: (gameEvent: GameEvent) => void;
  onCancel: () => void;
}

export function EditExecNodeModal({
  isOpen,
  node,
  gameEvent,
  updateGameEvent,
  onCancel,
}: EditExecNodeModalProps) {
  const [p, setP] = useState('');
  const [execStr, setExecStr] = useState('');

  useEffect(() => {
    if (node) {
      setP(node.p || '');
      setExecStr(node.execStr || '');
    }
  }, [node]);

  if (!isOpen || !node) {
    return null;
  }

  const handleEditNodeConfirm = (updatedNode: GameEventChildExec) => {
    const updatedChildren = (gameEvent.children || []).map((child) =>
      child.id === updatedNode.id ? updatedNode : child
    );
    // const updatedGameEvent: GameEvent = {
    //   ...gameEvent,
    //   children: updatedChildren,
    // };
    gameEvent.children = updatedChildren;
    updateGameEvent(gameEvent);
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
          maxWidth: '60vw',
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
          Edit Exec Node
        </h2>
        <div>
          <VariableWidget gameEvent={gameEvent} />
        </div>

        <div
          style={{
            color: '#999',
            marginBottom: '8px',
            fontSize: '14px',
          }}
        >
          GameEvent: <span style={{ color: '#00d4d4' }}>{gameEvent.id}</span> |
          Node: <span style={{ color: '#d4d400' }}>{node.id}</span>
        </div>

        <div style={{ marginBottom: '20px' }}>
          <label
            style={{
              display: 'block',
              color: '#d4d4d4',
              marginBottom: '8px',
              fontSize: '14px',
            }}
          >
            Text (p)
          </label>
          <textarea
            value={p}
            onChange={(e) => setP(e.target.value)}
            style={{
              width: '100%',
              minHeight: '200px',
              padding: '8px',
              backgroundColor: '#1e1e1e',
              border: '1px solid #3e3e42',
              borderRadius: '4px',
              color: '#d4d4d4',
              fontFamily: 'monospace',
              fontSize: '14px',
              resize: 'vertical',
            }}
            placeholder="Enter text to display..."
          />
        </div>

        <div style={{ marginBottom: '20px' }}>
          <label
            style={{
              display: 'block',
              color: '#d4d4d4',
              marginBottom: '8px',
              fontSize: '14px',
            }}
          >
            Execute Code (execStr)
          </label>
          <textarea
            value={execStr}
            onChange={(e) => setExecStr(e.target.value)}
            style={{
              width: '100%',
              minHeight: '200px',
              padding: '8px',
              backgroundColor: '#1e1e1e',
              border: '1px solid #3e3e42',
              borderRadius: '4px',
              color: '#d4d4d4',
              fontFamily: 'monospace',
              fontSize: '14px',
              resize: 'vertical',
            }}
            spellCheck={false}
            placeholder="Enter code to execute..."
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
            onClick={() =>
              handleEditNodeConfirm({
                ...node,
                p,
                execStr,
              })
            }
          >
            Confirm
          </Button>
          <Button variant="secondary" onClick={onCancel}>
            Cancel
          </Button>
        </div>
      </div>
    </div>
  );
}
