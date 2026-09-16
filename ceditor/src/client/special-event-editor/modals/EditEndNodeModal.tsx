import { useState, useEffect } from 'react';
import { GameEvent } from '../../types/assets';
import { Button } from '../../elements/Button';
import { EditorNodeEnd } from '../cmpts/EndNodeComponent';
import { notifyStateUpdated } from '../seEditorState';
import { AssetSearchWidget } from '../react-components/AssetSearchWidget';
import { MODAL_ROOT_CLASS, useEscapeToClose } from '../../hooks/useEscapeToClose';

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
  const [_next, setNext] = useState('');

  useEffect(() => {
    if (node) {
      const seNode = node.toSENode();
      setNext(seNode.next || '');
    }
  }, [node]);

  const modalRef = useEscapeToClose(onCancel, isOpen && !!node);

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
      ref={modalRef}
      className={`${MODAL_ROOT_CLASS} se-node-modal-overlay`}
    >
      <div className="se-node-modal-panel" onClick={(e) => e.stopPropagation()}>
        <div className="se-node-modal-chrome">
          <h2>Edit End Node</h2>
          <AssetSearchWidget gameEvent={gameEvent} />
          <div className="se-node-modal-ids">
            GameEvent: <span style={{ color: '#00d4d4' }}>{gameEvent.id}</span> |
            Node: <span style={{ color: '#d4d400' }}>{node.id}</span>
          </div>
        </div>

        <div className="se-node-modal-body">
          <div
            style={{
              padding: '20px',
              backgroundColor: '#1e1e1e',
              border: '1px solid #3e3e42',
              borderRadius: '4px',
            }}
          >
            <p style={{ color: '#d4d4d4', margin: 0 }}>
              This is an End Node. It represents the end of a game event flow.
            </p>
          </div>
        </div>

        <div className="se-node-modal-footer">
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
