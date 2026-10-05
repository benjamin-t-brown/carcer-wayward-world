import { useEffect, useState } from 'react';
import { GameEvent } from '../../../types/assets';
import { Button } from '../../../elements/Button';
import { AssetSearchWidget } from '../../../special-event-editor/react-components/AssetSearchWidget';
import { OnceKeyGenerator } from '../../../special-event-editor/react-components/OnceKeyGenerator';
import { MODAL_ROOT_CLASS, useEscapeToClose } from '../../../hooks/useEscapeToClose';

export function ConditionEditButton({
  condition,
  onClick,
}: {
  condition: string;
  onClick: () => void;
}) {
  const [hover, setHover] = useState(false);

  return (
    <div>
      <button
        type="button"
        onClick={onClick}
        onMouseEnter={() => setHover(true)}
        onMouseLeave={() => setHover(false)}
        style={{
          width: '100%',
          padding: '4px 8px',
          border: '1px solid #555555',
          backgroundColor: hover ? '#4a4a4a' : '#3e3e42',
          color: '#ffffff',
          cursor: 'pointer',
          fontSize: '11px',
          borderRadius: '4px',
        }}
      >
        {condition ? 'Edit condition' : 'Set condition'}
      </button>
      {condition ? (
        <div
          style={{
            marginTop: '4px',
            color: '#858585',
            fontSize: '10px',
            overflowWrap: 'anywhere',
            display: '-webkit-box',
            WebkitLineClamp: 3,
            WebkitBoxOrient: 'vertical',
            overflow: 'hidden',
          }}
        >
          {condition}
        </div>
      ) : null}
    </div>
  );
}

interface EditTileEventConditionModalProps {
  isOpen: boolean;
  condition: string;
  gameEvent: GameEvent | null;
  onConfirm: (condition: string) => void;
  onCancel: () => void;
}

export function EditTileEventConditionModal({
  isOpen,
  condition,
  gameEvent,
  onConfirm,
  onCancel,
}: EditTileEventConditionModalProps) {
  const [draft, setDraft] = useState(condition);
  const modalRef = useEscapeToClose(onCancel, isOpen);

  useEffect(() => {
    if (!isOpen) {
      return;
    }
    setDraft(condition);
  }, [isOpen, condition]);

  if (!isOpen) {
    return null;
  }

  return (
    <div
      ref={modalRef}
      className={`${MODAL_ROOT_CLASS} se-node-modal-overlay`}
      onClick={(e) => e.stopPropagation()}
    >
      <div className="se-node-modal-panel" onClick={(e) => e.stopPropagation()}>
        <div className="se-node-modal-chrome">
          <h2>Edit Condition</h2>
          <div className="se-node-modal-ids">
            {gameEvent ? (
              <>
                GameEvent: <span style={{ color: '#00d4d4' }}>{gameEvent.id}</span>
              </>
            ) : (
              'Leave empty to always run this event.'
            )}
          </div>
          <AssetSearchWidget gameEvent={gameEvent} />
          <OnceKeyGenerator />
        </div>

        <div className="se-node-modal-body">
          <div className="se-node-modal-field">
            <label htmlFor="tile-event-condition-editor">Condition</label>
            <textarea
              id="tile-event-condition-editor"
              className="se-node-modal-textarea"
              value={draft}
              onChange={(e) => setDraft(e.target.value)}
              spellCheck={false}
              placeholder="IS(flag) — leave empty to always run"
            />
          </div>
        </div>

        <div className="se-node-modal-footer">
          <Button variant="primary" onClick={() => onConfirm(draft.trim())}>
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
