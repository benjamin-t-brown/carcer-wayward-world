import { Button } from '../../../elements/Button';
import { MODAL_ROOT_CLASS, useEscapeToClose } from '../../../hooks/useEscapeToClose';

interface DeleteTileEventModalProps {
  isOpen: boolean;
  eventId: string;
  eventTitle?: string;
  eventExists: boolean;
  otherUseNote: string;
  isDeleting: boolean;
  onRemoveTriggerOnly: () => void;
  onRemoveAndDeleteEvent: () => void;
  onCancel: () => void;
}

export function DeleteTileEventModal({
  isOpen,
  eventId,
  eventTitle,
  eventExists,
  otherUseNote,
  isDeleting,
  onRemoveTriggerOnly,
  onRemoveAndDeleteEvent,
  onCancel,
}: DeleteTileEventModalProps) {
  const modalRef = useEscapeToClose(onCancel, isOpen);

  if (!isOpen) {
    return null;
  }

  const label = eventTitle && eventTitle !== eventId ? eventTitle : eventId;

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
      onClick={onCancel}
    >
      <div
        style={{
          backgroundColor: '#252526',
          border: '1px solid #3e3e42',
          borderRadius: '8px',
          padding: '30px',
          maxWidth: '520px',
          width: '90%',
          maxHeight: '90vh',
          overflowY: 'auto',
        }}
        onClick={(e) => e.stopPropagation()}
      >
        <h2
          style={{
            color: '#f48771',
            marginBottom: '20px',
            marginTop: 0,
          }}
        >
          Remove event trigger?
        </h2>
        <p
          style={{
            color: '#d4d4d4',
            marginTop: 0,
            marginBottom: '12px',
            lineHeight: 1.45,
            overflowWrap: 'anywhere',
          }}
        >
          {eventExists
            ? `Also delete "${label}" from the events editor?`
            : `"${label}" is not in the events editor. Remove the trigger from this tile?`}
        </p>
        {eventExists && eventTitle && eventTitle !== eventId && (
          <p
            style={{
              color: '#858585',
              fontSize: '12px',
              marginTop: 0,
              marginBottom: '12px',
              overflowWrap: 'anywhere',
            }}
          >
            {eventId}
          </p>
        )}
        {otherUseNote && (
          <p
            style={{
              color: '#dcdcaa',
              fontSize: '12px',
              marginTop: 0,
              marginBottom: '20px',
              lineHeight: 1.45,
              overflowWrap: 'anywhere',
            }}
          >
            {otherUseNote}
          </p>
        )}
        <div
          style={{
            display: 'flex',
            gap: '10px',
            justifyContent: 'flex-end',
            flexWrap: 'wrap',
          }}
        >
          {eventExists && (
            <Button
              variant="danger"
              onClick={onRemoveAndDeleteEvent}
              disabled={isDeleting}
            >
              {isDeleting ? 'Deleting...' : 'Also delete event'}
            </Button>
          )}
          <Button
            variant="primary"
            onClick={onRemoveTriggerOnly}
            disabled={isDeleting}
          >
            {eventExists ? 'Remove trigger only' : 'Remove trigger'}
          </Button>
          <Button variant="secondary" onClick={onCancel} disabled={isDeleting}>
            Cancel
          </Button>
        </div>
      </div>
    </div>
  );
}
