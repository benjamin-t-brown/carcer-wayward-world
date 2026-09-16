import { useState, useEffect } from 'react';
import { GameEvent, SwitchCase } from '../../types/assets';
import { Button } from '../../elements/Button';
import { EditorNodeSwitch } from '../cmpts/SwitchNodeComponent';
import { notifyStateUpdated } from '../seEditorState';
import { AssetSearchWidget } from '../react-components/AssetSearchWidget';
import { OnceKeyGenerator } from '../react-components/OnceKeyGenerator';
import { MODAL_ROOT_CLASS, useEscapeToClose } from '../../hooks/useEscapeToClose';

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

  const modalRef = useEscapeToClose(onCancel, isOpen && !!node);

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
      ref={modalRef}
      className={`${MODAL_ROOT_CLASS} se-node-modal-overlay`}
    >
      <div className="se-node-modal-panel" onClick={(e) => e.stopPropagation()}>
        <div className="se-node-modal-chrome">
          <h2>Edit Switch Node</h2>
          <div className="se-node-modal-ids">
            GameEvent: <span style={{ color: '#00d4d4' }}>{gameEvent.id}</span> |
            Node: <span style={{ color: '#d4d400' }}>{node.id}</span>
          </div>
          <AssetSearchWidget gameEvent={gameEvent} />
          <OnceKeyGenerator />
        </div>

        <div className="se-node-modal-body">
          <div className="se-node-modal-section">
            <div className="se-node-modal-section-header">
              <label>Cases</label>
              <Button onClick={handleAddCase}>+ Add Case</Button>
            </div>

            <div id="switch-node-cases" className="se-node-modal-scroll">
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
        </div>

        <div className="se-node-modal-footer">
          <Button onClick={handleEditNodeConfirm}>Save</Button>
          <Button variant="secondary" onClick={onCancel}>
            Cancel
          </Button>
        </div>
      </div>
    </div>
  );
}
