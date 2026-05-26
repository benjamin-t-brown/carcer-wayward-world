import { Button } from './Button';

interface GenericModalProps {
  title: string;
  onCancel: () => void;
  onConfirm: () => void;
  body: () => React.ReactNode;
  maxWidth?: string;
  disableCancel?: boolean;
  /** Body fills remaining height; scroll is delegated to modal content (e.g. sprite grid). */
  fillBody?: boolean;
}

export const GenericModal = (props: GenericModalProps) => {
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
          maxWidth: props.maxWidth ?? '90vw',
          width: '90%',
          maxHeight: '80vh',
          height: props.fillBody ? '80vh' : undefined,
          overflow: 'hidden',
          position: 'relative',
          display: 'flex',
          flexDirection: 'column',
        }}
        onClick={(e) => e.stopPropagation()}
      >
        <div style={{ flexShrink: 0, paddingRight: '28px' }}>
          <button
            type="button"
            aria-label="Close"
            style={{
              position: 'absolute',
              top: 8,
              right: 10,
              backgroundColor: 'transparent',
              border: 'none',
              color: '#d4d4d4',
              fontSize: '24px',
              fontWeight: 'bold',
              cursor: 'pointer',
              lineHeight: 1,
              padding: '4px 8px',
            }}
            onClick={props.onCancel}
          >
            <span>×</span>
          </button>
          <h2
            style={{
              color: '#4ec9b0',
              marginBottom: '16px',
              marginTop: 0,
            }}
          >
            {props.title}
          </h2>
        </div>

        <div
          style={{
            flex: 1,
            minHeight: 0,
            overflow: props.fillBody ? 'hidden' : 'auto',
            display: props.fillBody ? 'flex' : 'block',
            flexDirection: 'column',
          }}
        >
          {props.body()}
        </div>

        <div
          style={{
            flexShrink: 0,
            display: 'flex',
            gap: '10px',
            justifyContent: 'flex-end',
            marginTop: '16px',
            paddingTop: '12px',
            borderTop: '1px solid #3e3e42',
          }}
        >
          {props.disableCancel ? (
            <Button variant="primary" onClick={props.onConfirm}>
              Okay
            </Button>
          ) : (
            <>
              <Button variant="primary" onClick={props.onConfirm}>
                Confirm
              </Button>
              <Button variant="danger" onClick={props.onCancel}>
                Cancel
              </Button>
            </>
          )}
        </div>
      </div>
    </div>
  );
};
