import { useState, useEffect } from 'react';
import { GameEvent, Variable } from '../../types/assets';
import { Button } from '../../elements/Button';
import { useAssets } from '../../contexts/AssetsContext';
import { randomId } from '../../utils/mathUtils';

interface VariableEditorModalProps {
  isOpen: boolean;
  gameEvent: GameEvent | null;
  onConfirm: (gameEvent: GameEvent) => void;
  onCancel: () => void;
}

interface VariableEntryProps {
  variableName: string;
  value: string | number | boolean;
  variableId: string;
  importFrom: string;
  handleDeleteVariable: (variableId: string) => void;
  hideErrors: () => void;
  handleMoveUp: () => void;
  handleMoveDown: () => void;
  canMoveUp: boolean;
  canMoveDown: boolean;
}

const VariableEntry = ({
  variableName,
  value,
  variableId,
  handleDeleteVariable,
  hideErrors,
  handleMoveUp,
  handleMoveDown,
  canMoveUp,
  canMoveDown,
}: VariableEntryProps) => {
  return (
    <div
      style={{ display: 'flex', gap: '10px', alignItems: 'center' }}
      onClick={() => hideErrors()}
    >
      <div
        style={{
          display: 'flex',
          flexDirection: 'column',
          gap: '2px',
        }}
      >
        <button
          style={{
            width: '24px',
            height: '20px',
            textAlign: 'center',
            fontSize: '12px',
            backgroundColor: 'rgba(255, 255, 255, 0.2)',
            border: 'none',
            cursor: 'pointer',
            color: 'white',
          }}
          onClick={(e) => {
            e.stopPropagation();
            if (canMoveUp) {
              handleMoveUp();
            }
          }}
          disabled={!canMoveUp}
        >
          <span>↑</span>
        </button>
        <button
          style={{
            width: '24px',
            height: '20px',
            textAlign: 'center',
            fontSize: '12px',
            backgroundColor: 'rgba(255, 255, 255, 0.2)',
            border: 'none',
            cursor: 'pointer',
            color: 'white',
          }}
          onClick={(e) => {
            e.stopPropagation();
            if (canMoveDown) {
              handleMoveDown();
            }
          }}
          disabled={!canMoveDown}
        >
          <span>↓</span>
        </button>
      </div>
      <div
        style={{
          width: '75px',
          fontSize: '10px',
        }}
      >
        {variableId}
      </div>
      <input
        type="text"
        defaultValue={String(variableName)}
        id={`${variableId}_key`}
        style={{ width: '200px', padding: '2px' }}
      />
      <span>=</span>
      <input
        type="text"
        defaultValue={String(value)}
        id={`${variableId}_value`}
        style={{ width: '400px', padding: '2px' }}
      />
      <button
        style={{
          width: '24px',
          height: '20px',
          textAlign: 'center',
          fontSize: '12px',
          paddingLeft: '2px',
          backgroundColor: 'transparent',
          border: 'none',
          cursor: 'pointer',
        }}
        onClick={(e) => {
          e.stopPropagation();
          handleDeleteVariable(variableId);
        }}
      >
        <span>❌</span>
      </button>
    </div>
  );
};

export function VariableEditorModal({
  isOpen,
  gameEvent,
  onConfirm,
  onCancel,
}: VariableEditorModalProps) {
  const { gameEvents } = useAssets();
  const [vars, setVars] = useState<Variable[]>(
    gameEvent?.vars.filter((variable) => variable.importFrom === '') || []
  );
  const [importVars, setImportVars] = useState<Variable[]>(
    gameEvent?.vars.filter((variable) => variable.importFrom !== '') || []
  );
  const [errorText, setErrorText] = useState<string>('');
  const [importFromValue, setImportFromValue] = useState<string>(
    gameEvent?.id || ''
  );

  useEffect(() => {
    setVars(
      gameEvent?.vars.filter((variable) => variable.importFrom === '') || []
    );
    setImportVars(
      gameEvent?.vars.filter((variable) => variable.importFrom !== '') || []
    );
    setImportFromValue(gameEvent?.id || '');
  }, [gameEvent]);

  const handleConfirm = () => {
    const newVars: Variable[] = [];
    for (const variable of vars) {
      const elem = document.getElementById(
        `${variable.id}_key`
      ) as HTMLInputElement;
      const elemValue = document.getElementById(
        `${variable.id}_value`
      ) as HTMLInputElement;
      if (elem) {
        variable.key = elem.value;
      }
      if (elemValue) {
        variable.value = elemValue.value;
      }
      newVars.push(variable);
    }
    const errors = validateVars(newVars);

    for (const variable of importVars) {
      if (!variable.importFrom) {
        errors.push('Import from is required for variable: ' + variable.id);
      }
      newVars.push(variable);
    }
    if (errors.length > 0) {
      setErrorText(errors.join('<br />'));
      return;
    } else {
      if (gameEvent) {
        gameEvent.vars = newVars;
        onConfirm(gameEvent);
        onCancel();
      }
    }
  };

  const handleCancel = () => {
    setVars(
      gameEvent?.vars.filter((variable) => variable.importFrom === '') || []
    );
    setImportVars(
      gameEvent?.vars.filter((variable) => variable.importFrom !== '') || []
    );
    onCancel();
  };

  const handleAddVariable = () => {
    setErrorText('');
    setVars([...vars, { id: randomId(), key: '', value: '', importFrom: '' }]);
  };

  const handleAddImport = () => {
    if (importFromValue === '') {
      setErrorText('Please select a game event to import from.');
      return;
    }
    if (importFromValue === gameEvent?.id) {
      setErrorText('You cannot import from the same game event.');
      return;
    }
    // Check if the import already exists
    const alreadyImported = importVars.some(
      (variable) => variable.importFrom === importFromValue
    );
    if (alreadyImported) {
      setErrorText('This game event is already imported.');
      return;
    }
    setErrorText('');
    setImportVars([
      ...importVars,
      { id: randomId(), key: '', value: '', importFrom: importFromValue },
    ]);
  };

  const handleDeleteVariable = (variableId: string) => {
    setErrorText('');
    setVars(vars.filter((variable) => variable.id !== variableId));
  };

  const handleDeleteImport = (variableId: string) => {
    setErrorText('');
    setImportVars(importVars.filter((variable) => variable.id !== variableId));
  };

  const handleMoveVariableUp = (index: number) => {
    if (index === 0) return;
    setErrorText('');
    const newVars = [...vars];
    [newVars[index - 1], newVars[index]] = [newVars[index], newVars[index - 1]];
    setVars(newVars);
  };

  const handleMoveVariableDown = (index: number) => {
    if (index === vars.length - 1) return;
    setErrorText('');
    const newVars = [...vars];
    [newVars[index], newVars[index + 1]] = [newVars[index + 1], newVars[index]];
    setVars(newVars);
  };

  const validateVars = (vars: Variable[]) => {
    const errors: string[] = [];
    const seenKeys = new Set<string>();
    for (const variable of vars) {
      const trimmedKey = variable.key?.trim();
      if (!trimmedKey) {
        errors.push('Variable name is required for variable: ' + variable.id);
      } else {
        if (seenKeys.has(trimmedKey)) {
          errors.push(
            `Duplicate variable name: "${trimmedKey}" for variable: ${variable.id}`
          );
        }
        seenKeys.add(trimmedKey);
      }
    }
    return errors;
  };

  if (!isOpen || !gameEvent) {
    return null;
  }

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
          maxWidth: '60vw',
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
          onClick={onCancel}
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
          Edit Variables
        </h2>
        {errorText && (
          <div
            style={{ color: '#f48771', marginBottom: '20px' }}
            dangerouslySetInnerHTML={{ __html: errorText }}
          ></div>
        )}
        <div
          style={{
            marginBottom: '20px',
            display: 'flex',
            flexDirection: 'column',
            gap: '5px',
          }}
        >
          {vars.map((variable, index) => (
            <VariableEntry
              key={variable.id}
              variableName={variable.key}
              variableId={variable.id}
              value={variable.value}
              importFrom={variable.importFrom}
              handleDeleteVariable={handleDeleteVariable}
              hideErrors={() => setErrorText('')}
              handleMoveUp={() => handleMoveVariableUp(index)}
              handleMoveDown={() => handleMoveVariableDown(index)}
              canMoveUp={index > 0}
              canMoveDown={index < vars.length - 1}
            />
          ))}
        </div>

        <div
          style={{
            display: 'flex',
            gap: '10px',
            width: '200px',
            flexDirection: 'column',
            marginBottom: '20px',
          }}
        >
          <Button variant="small" onClick={() => handleAddVariable()}>
            + Add Variable
          </Button>
        </div>

        <h3
          style={{
            marginBottom: '20px',
          }}
        >
          Imports
        </h3>

        {importVars.length > 0 && (
          <ul
            style={{
              margin: '20px',
            }}
          >
            {importVars.map((variable) => (
              <li
                key={variable.id}
                style={{
                  marginLeft: '20px',
                  display: 'flex',
                  gap: '10px',
                  alignItems: 'center',
                }}
              >
                <span>{variable.importFrom}</span>
                <button
                  style={{
                    width: '24px',
                    height: '20px',
                    textAlign: 'center',
                    fontSize: '12px',
                    paddingLeft: '2px',
                    backgroundColor: 'transparent',
                    border: 'none',
                    cursor: 'pointer',
                  }}
                  onClick={() => handleDeleteImport(variable.id)}
                >
                  <span>❌</span>
                </button>
              </li>
            ))}
          </ul>
        )}

        <div
          style={{
            display: 'flex',
            gap: '10px',
            flexDirection: 'column',
            width: '200px',
          }}
        >
          <select
            value={importFromValue}
            onChange={(e) => setImportFromValue(e.target.value)}
          >
            {gameEvents.map((gameEvent) => (
              <option key={gameEvent.id} value={gameEvent.id}>
                {gameEvent.id}
              </option>
            ))}
          </select>

          <Button variant="small" onClick={() => handleAddImport()}>
            + Add Import
          </Button>
        </div>

        <div
          style={{
            display: 'flex',
            gap: '10px',
            justifyContent: 'flex-end',
          }}
        >
          <Button variant="primary" onClick={() => handleConfirm()}>
            Confirm
          </Button>
          <Button variant="secondary" onClick={() => handleCancel()}>
            Cancel
          </Button>
        </div>
      </div>
    </div>
  );
}
