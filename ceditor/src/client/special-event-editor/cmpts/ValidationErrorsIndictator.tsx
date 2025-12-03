import { useState } from 'react';
import { centerPanzoomOnNode } from '../seEditorState';
import { CANVAS_CONTAINER_ID } from '../MapCanvasSE';

interface ValidationErrorsIndicatorProps {
  errors: {
    message: string;
    childId: string;
  }[];
}

export const ValidationErrorsIndicator = (
  props: ValidationErrorsIndicatorProps
) => {
  const [isOpen, setIsOpen] = useState(false);

  const handleClose = () => {
    setIsOpen(false);
  };

  const handleOpen = () => {
    setIsOpen(true);
  };

  const handleErrorClick = (childId: string) => {
    const canvas = document.getElementById(CANVAS_CONTAINER_ID + '-canvas');
    if (canvas) {
      centerPanzoomOnNode(canvas as HTMLCanvasElement, childId);
    }
  };

  if (!isOpen) {
    return (
      <div
        style={{
          cursor: 'pointer',
          userSelect: 'none',
        }}
        onClick={props.errors.length > 0 ? handleOpen : undefined}
      >
        {props.errors.length === 0 ? '✅' : '❌'}
      </div>
    );
  }

  return (
    <div
      style={{
        position: 'absolute',
        left: '0',
        top: '0',
        width: '300px',
        height: '200px',
        background: 'rgba(0, 0, 0, 0.5)',
        color: 'white',
        borderRadius: '5px',
        padding: '10px',
        zIndex: 1000,
      }}
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
        onClick={handleClose}
      >
        <span>×</span>
      </button>
      <div
        style={{
          marginTop: '20px',
          height: 'calc(100% - 20px)',
          overflow: 'auto',
          border: '1px solid red',
        }}
      >
        {props.errors.map((error) => (
          <div key={error.childId}>
            <span
              style={{
                cursor: 'pointer',
                color: 'lightblue',
                textDecoration: 'underline',
              }}
              onClick={() => handleErrorClick(error.childId)}
            >
              {error.childId}
            </span>
            : {error.message}
          </div>
        ))}
      </div>
    </div>
  );
};
