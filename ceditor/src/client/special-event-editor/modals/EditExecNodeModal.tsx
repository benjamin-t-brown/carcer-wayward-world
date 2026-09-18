import { useState, useEffect } from 'react';
import { GameEvent } from '../../types/assets';
import { Button } from '../../elements/Button';
import { EditorNodeExec } from '../cmpts/ExecNodeComponent';
import { notifyStateUpdated } from '../seEditorState';
import { AudioWidget } from '../react-components/AudioWidget';
import { AssetSearchWidget } from '../react-components/AssetSearchWidget';
import { OnceKeyGenerator } from '../react-components/OnceKeyGenerator';
import {
  MODAL_ROOT_CLASS,
  useEscapeToClose,
} from '../../hooks/useEscapeToClose';

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

  const modalRef = useEscapeToClose(onCancel, isOpen && !!node);

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
    <div ref={modalRef} className={`${MODAL_ROOT_CLASS} se-node-modal-overlay`}>
      <div className="se-node-modal-panel" onClick={(e) => e.stopPropagation()}>
        <div className="se-node-modal-chrome">
          <h2>Edit Exec Node</h2>
          <div className="se-node-modal-ids">
            GameEvent: <span style={{ color: '#00d4d4' }}>{gameEvent.id}</span>{' '}
            | Node: <span style={{ color: '#d4d400' }}>{node.id}</span>
          </div>
          <AssetSearchWidget gameEvent={gameEvent} />
          <div className="se-node-modal-check-row">
            <input
              id="autoAdvance"
              type="checkbox"
              checked={autoAdvance}
              onChange={(e) => setAutoAdvance(e.target.checked)}
            />
            <label htmlFor="autoAdvance">Auto Advance</label>
          </div>
          <OnceKeyGenerator />
        </div>

        <div className="se-node-modal-body">
          <div className="se-node-modal-text-audio">
            <div className="se-node-modal-field">
              <label>Text (p)</label>
              <textarea
                className="se-node-modal-textarea"
                value={p}
                onChange={(e) => setP(e.target.value)}
                placeholder="Enter text to display..."
              />
            </div>
            <div className="se-node-modal-audio">
              <AudioWidget node={node} />
            </div>
          </div>
          <div className="se-node-modal-field">
            <label>Execute Code (execStr)</label>
            <textarea
              className="se-node-modal-textarea"
              style={{ width: 'calc(100% - 4px)' }}
              value={execStr}
              onChange={(e) => setExecStr(e.target.value)}
              spellCheck={false}
              placeholder="Enter code to execute..."
            />
          </div>
        </div>

        <div className="se-node-modal-footer">
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
