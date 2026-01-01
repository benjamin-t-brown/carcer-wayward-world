import { useState, useEffect } from 'react';
import { GameEvent } from '../../types/assets';
import { Button } from '../../elements/Button';
import { VariableWidget } from '../react-components/VariableWidget';
import { EditorNodeExec } from '../cmpts/ExecNodeComponent';
import { notifyStateUpdated } from '../seEditorState';
import { ExecWidget } from '../react-components/ExecWidget';
import { AudioWidget } from '../react-components/AudioWidget';
import { ItemTemplateWidget } from '../react-components/ItemTemplateWidget';

interface EditExecNodeModalProps {
  isOpen: boolean;
  node: EditorNodeExec | undefined;
  gameEvent: GameEvent;
  onCancel: () => void;
  ctx: CanvasRenderingContext2D;
}

export function EditExecNodeModal({
  isOpen,
  node,
  gameEvent,
  onCancel,
  ctx,
}: EditExecNodeModalProps) {
  const [p, setP] = useState('');
  const [execStr, setExecStr] = useState('');
  const [autoAdvance, setAutoAdvance] = useState(false);
  useEffect(() => {
    if (node) {
      setP(node.p || '');
      setExecStr(node.execStr || '');
      setAutoAdvance(node.autoAdvance || false);
    }
  }, [node]);

  if (!isOpen || !node) {
    return null;
  }

  const handleEditNodeConfirm = () => {
    node.p = p;
    node.execStr = execStr;
    node.autoAdvance = autoAdvance;
    node.build(ctx);
    notifyStateUpdated();
    onCancel();
  };

  return (
    <div
      className="generic-modal"
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
          Edit Exec Node
        </h2>

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
        <div
          style={{
            display: 'flex',
            gap: '10px',
            width: '100%',
            marginBottom: '20px',
          }}
        >
          <div style={{ width: '34%' }}>
            <VariableWidget gameEvent={gameEvent} />
          </div>
          <div style={{ width: '33%' }}>
            <ExecWidget gameEvent={gameEvent} />
          </div>
          <div style={{ width: '33%' }}>
            <ItemTemplateWidget gameEvent={gameEvent} />
          </div>
        </div>
        <div
          style={{
            width: '100%',
            display: 'flex',
            alignItems: 'center',
            gap: '10px',
            marginBottom: '20px',
          }}
        >
          <input
            id="autoAdvance"
            type="checkbox"
            checked={autoAdvance}
            onChange={(e) => setAutoAdvance(e.target.checked)}
          />
          <label
            htmlFor="autoAdvance"
            style={{
              color: '#d4d4d4',
              fontSize: '14px',
            }}
          >
            Auto Advance
          </label>
        </div>

        <div
          style={{
            marginBottom: '20px',
            display: 'flex',
            justifyContent: 'space-between',
          }}
        >
          <div style={{ width: '75%' }}>
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
          <div style={{ width: '20%' }}>
            <AudioWidget node={node} />
          </div>
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
          <Button variant="primary" onClick={() => handleEditNodeConfirm()}>
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
