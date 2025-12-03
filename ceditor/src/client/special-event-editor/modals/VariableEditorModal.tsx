import { useState, useEffect } from 'react';
import { GameEvent, Variable } from '../../types/assets';
import { Button } from '../../elements/Button';
import { useAssets } from '../../contexts/AssetsContext';
import { randomId } from '../../utils/mathUtils';
import { GenericModal } from '../../elements/GenericModal';
import { PickImportModal } from './PickImportModal';

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
  index: number;
  handleUpdateVariable: (
    index: number,
    field: keyof Variable,
    value: string
  ) => void;
  handleDeleteVariable: (variableId: string) => void;
  hideErrors: () => void;
  handleMoveUp: (index: number) => void;
  handleMoveDown: (index: number) => void;
}

const VariableEntry = ({
  variableName,
  value,
  variableId,
  index,
  handleDeleteVariable,
  hideErrors,
  handleMoveUp,
  handleMoveDown,
  handleUpdateVariable,
}: VariableEntryProps) => {
  return (
    <div
      key={index}
      style={{
        marginBottom: '4px',
        padding: '4px',
      }}
    >
      <div
        style={{
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
        }}
      >
        <div
          style={{
            color: '#d4d4d4',
            fontSize: '14px',
            fontWeight: 'bold',
            width: '20px',
            marginBottom: '8px',
          }}
        >
          {index + 1}.
        </div>
        <div
          style={{
            width: '48px',
            display: 'flex',
            alignItems: 'center',
            height: '100%',
            marginBottom: '8px',
            gap: '0px',
          }}
        >
          <button
            style={{
              width: '50%',
            }}
            onClick={() => handleMoveUp(index)}
          >
            <span>up</span>
          </button>
          <button
            style={{
              width: '50%',
            }}
            onClick={() => handleMoveDown(index)}
          >
            <span>dn</span>
          </button>
        </div>
        <div
          style={{
            marginBottom: '8px',
            width: 'calc(100% - 100px - 48px)',
          }}
        >
          <input
            type="text"
            value={variableName}
            onChange={(e) => handleUpdateVariable(index, 'key', e.target.value)}
            style={{
              width: '40%',
              padding: '6px',
              backgroundColor: '#1e1e1e',
              border: '1px solid #3e3e42',
              borderRadius: '4px',
              color: '#d4d4d4',
              fontFamily: 'monospace',
              fontSize: '12px',
            }}
            placeholder="Enter variable name..."
          />
          <input
            type="text"
            value={String(value)}
            onChange={(e) =>
              handleUpdateVariable(index, 'value', e.target.value)
            }
            style={{
              width: '59%',
              padding: '6px',
              backgroundColor: '#1e1e1e',
              border: '1px solid #3e3e42',
              borderRadius: '4px',
              color: '#d4d4d4',
              fontFamily: 'arial',
              fontSize: '12px',
            }}
            placeholder="Enter variable value..."
          />
        </div>
        <Button
          variant="danger"
          onClick={() => handleDeleteVariable(variableId)}
        >
          Delete
        </Button>
      </div>
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
  const [showPickImportModal, setShowPickImportModal] =
    useState<boolean>(false);

  useEffect(() => {
    setVars(
      gameEvent?.vars.filter((variable) => variable.importFrom === '') || []
    );
    setImportVars(
      gameEvent?.vars.filter((variable) => variable.importFrom !== '') || []
    );
  }, [gameEvent]);

  const handleUpdateVariable = (
    index: number,
    field: keyof Variable,
    value: string
  ) => {
    const newVars = [...vars];
    newVars[index] = { ...newVars[index], [field]: value };
    setVars(newVars);
  };

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
        setErrorText('');
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
    setErrorText('');
    onCancel();
  };

  const handleAddVariable = () => {
    setErrorText('');
    setVars([...vars, { id: randomId(), key: '', value: '', importFrom: '' }]);
  };

  const handleAddImport = (importFrom: string) => {
    // Check if the import already exists
    const alreadyImported = importVars.some(
      (variable) => variable.importFrom === importFrom
    );
    if (alreadyImported) {
      setErrorText('This game event is already imported.');
      return;
    }
    setErrorText('');
    setImportVars([
      ...importVars,
      { id: randomId(), key: '', value: '', importFrom },
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
    if (index === 0) {
      return;
    }
    setErrorText('');
    const newVars = [...vars];
    [newVars[index - 1], newVars[index]] = [newVars[index], newVars[index - 1]];
    setVars(newVars);
  };

  const handleMoveVariableDown = (index: number) => {
    if (index === vars.length - 1) {
      return;
    }
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
        errors.push(
          'Variable name is required for variable at index ' +
            (vars.indexOf(variable) + 1)
        );
      } else {
        if (seenKeys.has(trimmedKey)) {
          errors.push(
            `Duplicate variable name: "${trimmedKey}" for variable at index ${
              vars.indexOf(variable) + 1
            }`
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

  return GenericModal({
    title: 'Edit Variables',
    onCancel: handleCancel,
    onConfirm: handleConfirm,
    body: () => (
      <>
        <div
          style={{
            maxHeight: '45px',
            overflow: 'auto',
            marginBottom: '8px',
            background: '#222',
          }}
        >
          {importVars.map((variable) => (
            <div key={variable.id}>
              import {variable.importFrom}{' '}
              <button
                style={{
                  backgroundColor: 'transparent',
                }}
                onClick={() => handleDeleteImport(variable.id)}
              >
                <span role="img" aria-label="delete">
                  ❌
                </span>
              </button>
            </div>
          ))}
        </div>
        <div
          style={{
            display: 'flex',
            justifyContent: 'flex-end',
            alignItems: 'center',
            marginBottom: '4px',
            gap: '10px',
          }}
        >
          <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
            <Button
              variant="secondary"
              onClick={() => setShowPickImportModal(true)}
            >
              + Add Import
            </Button>
          </div>
          <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
            <Button onClick={handleAddVariable}>+ Add Variable</Button>
          </div>
        </div>
        <div
          style={{
            marginBottom: '20px',
            height: '400px',
            overflow: 'auto',
          }}
        >
          {vars.length === 0 && (
            <div
              style={{
                color: '#666',
                fontStyle: 'italic',
                padding: '20px',
                textAlign: 'center',
                borderRadius: '4px',
              }}
            >
              No variables. Click "Add Variable" to add one.
            </div>
          )}
          {vars.map((variable, index) => (
            <VariableEntry
              key={index}
              index={index}
              variableName={variable.key}
              variableId={variable.id}
              value={variable.value}
              // importFrom={variable.importFrom}
              handleDeleteVariable={handleDeleteVariable}
              handleUpdateVariable={handleUpdateVariable}
              hideErrors={() => setErrorText('')}
              handleMoveUp={() => handleMoveVariableUp(index)}
              handleMoveDown={() => handleMoveVariableDown(index)}
            />
          ))}
        </div>
        <div
          style={{
            marginBottom: '20px',
            height: '50px',
            overflow: 'auto',
            backgroundColor: errorText ? '#343434' : 'transparent',
          }}
        >
          {errorText ? (
            <div
              style={{ color: '#f48771', marginBottom: '20px' }}
              dangerouslySetInnerHTML={{ __html: errorText }}
            ></div>
          ) : (
            <div style={{ color: '#d4d4d4', marginBottom: '20px' }}>
              No errors.
            </div>
          )}
        </div>
        <PickImportModal
          isOpen={showPickImportModal}
          onConfirm={handleAddImport}
          onCancel={() => setShowPickImportModal(false)}
          gameEvents={gameEvents}
          existingImports={importVars.map((variable) => variable.importFrom)}
          gameEvent={gameEvent}
        />
      </>
    ),
  });
}
