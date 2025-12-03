import { useEffect, useState } from 'react';
import { GameEvent } from '../../types/assets';
import { GenericModal } from '../../elements/GenericModal';

export interface PickImportModalProps {
  isOpen: boolean;
  existingImports: string[];
  gameEvents: GameEvent[];
  gameEvent: GameEvent;
  onConfirm: (importFrom: string) => void;
  onCancel: () => void;
}

export function PickImportModal({
  isOpen,
  onConfirm,
  onCancel,
  gameEvents,
  existingImports,
  gameEvent,
}: PickImportModalProps) {
  const selectableEvents = gameEvents.filter(
    (g) => g.id !== gameEvent.id && !existingImports.includes(g.id)
  );
  const [importFromValue, setImportFromValue] = useState<string>(
    selectableEvents[0]?.id || ''
  );

  useEffect(() => {
    if (selectableEvents.length > 0) {
      setImportFromValue(selectableEvents[0].id);
    }
  }, [isOpen]);

  if (!isOpen) {
    return null;
  }

  const handleConfirm = () => {
    if (importFromValue === '') {
      return;
    }
    onConfirm(importFromValue);
    onCancel();
  };

  const handleCancel = () => {
    onCancel();
  };

  const options = selectableEvents.map((gameEvent) => {
    return (
      <option key={gameEvent.id} value={gameEvent.id}>
        {gameEvent.id}
      </option>
    );
  });

  return GenericModal({
    title: 'Pick Import',
    onCancel: handleCancel,
    onConfirm: handleConfirm,
    body: () => (
      <select
        value={importFromValue}
        onChange={(e) => setImportFromValue(e.target.value)}
        style={{
          width: '80%',
          margin: '18px',
        }}
      >
        {options}
      </select>
    ),
  });
}
