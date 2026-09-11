import { useCallback, useEffect, useState } from 'react';
import { Button } from '../../elements/Button';
import { centerPanzoomOnNode } from '../seEditorState';
import { EventValidator } from '../eventRunner/EventValidator';
import { CANVAS_CONTAINER_ID } from './MapCanvasSE';
import type { SpecialEventEditorController } from '../SpecialEventEditorController';
import type { SpecialEventDocumentSnapshot } from '../specialEventDocument';

interface ValidationErrorsIndicatorProps {
  controller: SpecialEventEditorController;
  isOpen: boolean;
  handleOpen: () => void;
  handleClose: () => void;
  errors: {
    message: string;
    childId: string;
  }[];
}

export const ValidationErrorsIndicator = (
  props: ValidationErrorsIndicatorProps,
) => {
  const handleErrorClick = (childId: string) => {
    const canvas = document.getElementById(CANVAS_CONTAINER_ID + '-canvas');
    if (canvas) {
      centerPanzoomOnNode(
        props.controller,
        canvas as HTMLCanvasElement,
        childId,
        true,
      );
    }
  };

  if (!props.isOpen) {
    return (
      <div
        style={{
          cursor: 'pointer',
          userSelect: 'none',
        }}
        onClick={props.handleOpen}
      >
        {props.errors.length === 0 ? '✅' : '❌'}
      </div>
    );
  }

  return (
    <>
      <span>{props.errors.length === 0 ? '✅' : '❌'}</span>
      <div
        style={{
          position: 'absolute',
          left: '0',
          top: '0',
          width: '400px',
          height: '200px',
          background: 'rgba(0, 0, 0, 0.75)',
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
          onClick={props.handleClose}
        >
          <span>×</span>
        </button>
        <div
          style={{
            padding: '4px',
            marginTop: '20px',
            height: 'calc(100% - 28px)',
            overflow: 'auto',
            border:
              '1px solid ' + (props.errors.length === 0 ? 'green' : 'red'),
          }}
        >
          {props.errors.length === 0 ? <div>No errors</div> : undefined}
          {props.errors.map((error, i) => (
            <div key={error.childId + i.toString()}>
              <span
                role="img"
                aria-label="error"
                title="error"
                style={{ marginRight: '6px' }}
              >
                ❌
              </span>
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
    </>
  );
};

interface ValidationMenuButtonProps {
  controller: SpecialEventEditorController;
  getDocumentSnapshot: () => SpecialEventDocumentSnapshot;
}

export const ValidationMenuButton = ({
  controller,
  getDocumentSnapshot,
}: ValidationMenuButtonProps) => {
  const [validationErrors, setValidationErrors] = useState<
    {
      message: string;
      childId: string;
    }[]
  >([]);
  const [isOpen, setIsOpen] = useState(false);

  const handleClose = () => {
    setIsOpen(false);
  };

  const handleOpen = () => {
    setIsOpen(true);
  };

  const validateCurrentGameEvent = useCallback(() => {
    const gameEvent = getDocumentSnapshot().currentGameEvent;
    if (!gameEvent) {
      return;
    }
    const validator = new EventValidator(gameEvent);
    const errors = validator.validate();
    const runnerErrors = controller.getState().runnerErrors;
    const allErrors = [
      ...errors,
      ...runnerErrors.map((error) => ({
        message: error.message,
        childId: error.nodeId,
      })),
    ];
    setValidationErrors(allErrors);
  }, [controller, getDocumentSnapshot]);

  useEffect(() => {
    return controller.subscribe(() => {
      if (controller.getState().shouldValidate) {
        validateCurrentGameEvent();
        controller.getState().shouldValidate = false;
      }
    });
  }, [controller, validateCurrentGameEvent]);

  // Metadata changes are committed through React state rather than the canvas
  // controller. Validate after that new collection has rendered so the
  // snapshot cannot observe the previous variables or event metadata.
  useEffect(() => {
    validateCurrentGameEvent();
  }, [validateCurrentGameEvent]);

  return (
    <div
      style={{
        display: 'flex',
        alignItems: 'center',
        gap: '4px',
      }}
    >
      <Button
        variant="small"
        onClick={() => {
          validateCurrentGameEvent();
        }}
      >
        Validate
      </Button>
      <div
        style={{
          position: 'relative',
        }}
      >
        <ValidationErrorsIndicator
          controller={controller}
          isOpen={isOpen}
          handleOpen={handleOpen}
          handleClose={handleClose}
          errors={validationErrors}
        />
      </div>
    </div>
  );
};
