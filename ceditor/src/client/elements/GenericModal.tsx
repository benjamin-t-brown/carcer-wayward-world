import { Button } from './Button';

interface GenericModalProps {
  title: string;
  onCancel: () => void;
  onConfirm: () => void;
  body: () => React.ReactNode;
}

export const GenericModal = (props: GenericModalProps) => {
  return (
    <div
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
          position: 'relative',
        }}
        onClick={(e) => e.stopPropagation()}
      >
        <button
          style={{
            position: 'absolute',
            top: 1,
            right: 5,
            backgroundColor: 'transparent',
            border: 'none',
            color: '#d4d4d4',
            fontSize: '24px',
            fontWeight: 'bold',
            cursor: 'pointer',
          }}
          onClick={props.onCancel}
        >
          <span>×</span>
        </button>
        <h2
          style={{
            color: '#4ec9b0',
            marginBottom: '20px',
            marginTop: 0,
          }}
        >
          {props.title}
        </h2>

        {props.body()}

        <div
          style={{
            display: 'flex',
            gap: '10px',
            justifyContent: 'flex-end',
          }}
        >
          <Button variant="primary" onClick={props.onConfirm}>
            Confirm
          </Button>
          <Button variant="danger" onClick={props.onCancel}>
            Cancel
          </Button>
        </div>
      </div>
    </div>
  );
};
