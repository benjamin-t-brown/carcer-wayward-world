import { useState, useEffect } from 'react';
import { GameEvent, SwitchCase } from '../../types/assets';
import { Button } from '../../elements/Button';
import { VariableWidget } from '../react-components/VariableWidget';
import { EditorNodeSwitch } from '../cmpts/SwitchNodeComponent';
import { notifyStateUpdated } from '../seEditorState';
import { ExecWidget } from '../react-components/ExecWidget';

interface EditSwitchNodeModalProps {
  isOpen: boolean;
  node: EditorNodeSwitch | undefined;
  gameEvent: GameEvent;
  onCancel: () => void;
  ctx: CanvasRenderingContext2D;
}

export function EditSwitchNodeModal({
  isOpen,
  node,
  gameEvent,
  onCancel,
  ctx,
}: EditSwitchNodeModalProps) {
  const [cases, setCases] = useState<SwitchCase[]>([]);
  const [defaultNext, setDefaultNext] = useState('');

  useEffect(() => {
    if (node) {
      const seNode = node.toSENode();
      setCases([...seNode.cases]);
      setDefaultNext(seNode.defaultNext || '');
    }
  }, [node]);

  if (!isOpen || !node) {
    return null;
  }

  const handleEditNodeConfirm = () => {
    // node.cases = cases;
    // node.defaultNext = defaultNext;
    node.defaultNext = defaultNext;
    node.buildFromCases(cases, ctx);
    notifyStateUpdated();
    onCancel();
  };

  const handleAddCase = () => {
    const newCase: SwitchCase = {
      conditionStr: '',
      next: '',
    };
    setCases([...cases, newCase]);
  };

  const handleRemoveCase = (index: number) => {
    const newCases = cases.filter((_, i) => i !== index);
    setCases(newCases);
  };

  const handleUpdateCase = (
    index: number,
    field: keyof SwitchCase,
    value: string
  ) => {
    const newCases = [...cases];
    newCases[index] = {
      ...newCases[index],
      [field]: value,
    };
    setCases(newCases);
  };

  const handleMoveCaseDir = (index: number, direction: 'up' | 'down') => {
    const newCases = [...cases];
    if (direction === 'up') {
      if (index === 0) {
        return;
      }
      const choice = newCases[index];
      newCases[index] = newCases[index - 1];
      newCases[index - 1] = choice;
    } else {
      if (index === newCases.length - 1) {
        return;
      }
      const choice = newCases[index];
      newCases[index] = newCases[index + 1];
      newCases[index + 1] = choice;
    }
    setCases(newCases);
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
          Edit Switch Node
        </h2>

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
            display: 'flex',
            gap: '10px',
            width: '100%',
            marginBottom: '20px',
          }}
        >
          <div style={{ width: '50%' }}>
            <VariableWidget gameEvent={gameEvent} />
          </div>
          <div style={{ width: '50%' }}>
            <ExecWidget gameEvent={gameEvent} />
          </div>
        </div>

        <div style={{ marginBottom: '20px' }}>
          <div
            style={{
              display: 'flex',
              justifyContent: 'space-between',
              alignItems: 'center',
              marginBottom: '4px',
            }}
          >
            <label
              style={{
                color: '#d4d4d4',
                fontSize: '14px',
                fontWeight: 'bold',
              }}
            >
              Cases
            </label>
            <Button onClick={handleAddCase}>+ Add Case</Button>
          </div>

          {cases.length === 0 && (
            <div
              style={{
                color: '#666',
                fontStyle: 'italic',
                padding: '20px',
                textAlign: 'center',
                border: '1px dashed #3e3e42',
                borderRadius: '4px',
              }}
            >
              No cases. Click "Add Case" to add one.
            </div>
          )}

          <div id="switch-node-cases">
            {cases.map((caseItem, index) => (
              <div
                key={index}
                style={{
                  marginBottom: '4px',
                  padding: '4px',
                }}
              >
                <div
                  style={{
                    display: 'flex',
                    justifyContent: 'space-between',
                    alignItems: 'center',
                  }}
                >
                  <div
                    style={{
                      width: '48px',
                      display: 'flex',
                      alignItems: 'center',
                      height: '100%',
                      marginBottom: '8px',
                      gap: '0px',
                    }}
                  >
                    <button
                      style={{
                        width: '50%',
                      }}
                      onClick={() => handleMoveCaseDir(index, 'up')}
                    >
                      <span>up</span>
                    </button>
                    <button
                      style={{
                        width: '50%',
                      }}
                      onClick={() => handleMoveCaseDir(index, 'down')}
                    >
                      <span>dn</span>
                    </button>
                  </div>

                  <div
                    style={{
                      marginBottom: '8px',
                      width: 'calc(100% - 100px - 48px)',
                    }}
                  >
                    {/* <label
                    style={{
                      display: 'block',
                      color: '#d4d4d4',
                      marginBottom: '4px',
                      fontSize: '12px',
                    }}
                  >
                    Condition String
                  </label> */}
                    <input
                      type="text"
                      value={caseItem.conditionStr}
                      onChange={(e) =>
                        handleUpdateCase(index, 'conditionStr', e.target.value)
                      }
                      style={{
                        width: '100%',
                        padding: '6px',
                        backgroundColor: '#1e1e1e',
                        border: '1px solid #3e3e42',
                        borderRadius: '4px',
                        color: '#d4d4d4',
                        fontFamily: 'monospace',
                        fontSize: '12px',
                      }}
                      placeholder="Enter condition..."
                    />
                  </div>
                  <Button
                    variant="danger"
                    onClick={() => handleRemoveCase(index)}
                  >
                    Delete
                  </Button>
                </div>
              </div>
            ))}
          </div>
        </div>

        <div
          style={{
            display: 'flex',
            justifyContent: 'flex-end',
            gap: '10px',
            marginTop: '30px',
          }}
        >
          <Button onClick={handleEditNodeConfirm}>Save</Button>
          <Button variant="secondary" onClick={onCancel}>
            Cancel
          </Button>
        </div>
      </div>
    </div>
  );
}
