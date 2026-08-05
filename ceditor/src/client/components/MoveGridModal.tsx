import { useEffect, useState } from 'react';
import { NumberInput } from '../elements/NumberInput';
import { Button } from '../elements/Button';
import { MODAL_ROOT_CLASS, useEscapeToClose } from '../hooks/useEscapeToClose';

interface MoveGridModalProps {
  isOpen: boolean;
  onConfirm: (offsetX: number, offsetY: number) => void;
  onCancel: () => void;
}

export function MoveGridModal({
  isOpen,
  onConfirm,
  onCancel,
}: MoveGridModalProps) {
  const [offsetX, setOffsetX] = useState(0);
  const [offsetY, setOffsetY] = useState(0);

  useEffect(() => {
    if (isOpen) {
      setOffsetX(0);
      setOffsetY(0);
    }
  }, [isOpen]);

  const modalRef = useEscapeToClose(onCancel, isOpen);

  if (!isOpen) {
    return null;
  }

  const handleConfirm = () => {
    onConfirm(Math.trunc(offsetX) || 0, Math.trunc(offsetY) || 0);
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
          maxWidth: '420px',
          width: '90%',
        }}
      >
        <h2
          style={{
            color: '#4ec9b0',
            marginBottom: '12px',
            marginTop: 0,
          }}
        >
          Move Grid
        </h2>
        <p
          style={{
            color: '#858585',
            fontSize: '12px',
            marginTop: 0,
            marginBottom: '20px',
            lineHeight: 1.4,
          }}
        >
          Shift all map assignments by the given cell offsets. Negative values
          move left/up. Assignments that leave the grid bounds are cleared.
        </p>
        <div className="form-fields-inline" style={{ marginBottom: '24px' }}>
          <NumberInput
            id="move-grid-offset-x"
            name="offsetX"
            label="X Offset"
            value={offsetX}
            onChange={setOffsetX}
          />
          <NumberInput
            id="move-grid-offset-y"
            name="offsetY"
            label="Y Offset"
            value={offsetY}
            onChange={setOffsetY}
          />
        </div>
        <div
          style={{
            display: 'flex',
            gap: '10px',
            justifyContent: 'flex-end',
          }}
        >
          <Button variant="primary" onClick={handleConfirm}>
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
