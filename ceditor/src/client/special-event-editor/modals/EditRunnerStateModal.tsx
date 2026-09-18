import { useEffect, useState } from 'react';
import { GenericModal } from '../../elements/GenericModal';
import { Button } from '../../elements/Button';
import {
  addKeyToRunnerInitialState,
  loadRunnerInitialStateText,
  parseRunnerInitialState,
  saveRunnerInitialStateText,
} from '../eventRunner/runnerInitialState';

interface EditRunnerStateModalProps {
  isOpen: boolean;
  onCancel: () => void;
}

export function EditRunnerStateModal({
  isOpen,
  onCancel,
}: EditRunnerStateModalProps) {
  const [jsonText, setJsonText] = useState(loadRunnerInitialStateText);
  const [keyInput, setKeyInput] = useState('');
  const [error, setError] = useState('');

  useEffect(() => {
    if (isOpen) {
      const text = loadRunnerInitialStateText();
      setJsonText(text);
      setKeyInput('');
      const parsed = parseRunnerInitialState(text);
      setError(parsed.ok ? '' : parsed.error);
    }
  }, [isOpen]);

  if (!isOpen) {
    return null;
  }

  const persistText = (text: string) => {
    setJsonText(text);
    saveRunnerInitialStateText(text);
    const parsed = parseRunnerInitialState(text);
    setError(parsed.ok ? '' : parsed.error);
  };

  const handleAdd = () => {
    const result = addKeyToRunnerInitialState(jsonText, keyInput);
    if (!result.ok) {
      setError(result.error);
      return;
    }
    persistText(result.text);
    setKeyInput('');
  };

  return (
    <GenericModal
      title="Edit State"
      onCancel={onCancel}
      onConfirm={onCancel}
      maxWidth="720px"
      disableCancel={true}
      fillBody={true}
      body={() => (
        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            flex: 1,
            height: '100%',
            minHeight: 0,
            gap: '10px',
          }}
        >
          <div style={{ color: '#858585', fontSize: '13px', flexShrink: 0 }}>
            Starting storage for Run Event. Keys use dotted paths like
            vars.hasSpokenToGateGuardJerry. Add sets the key to "true".
          </div>
          <div
            style={{
              display: 'flex',
              gap: '8px',
              alignItems: 'center',
              flexShrink: 0,
            }}
          >
            <input
              type="text"
              value={keyInput}
              onChange={(event) => setKeyInput(event.target.value)}
              onKeyDown={(event) => {
                if (event.key === 'Enter') {
                  event.preventDefault();
                  handleAdd();
                }
              }}
              placeholder="vars.hasSpokenToGateGuardJerry"
              spellCheck={false}
              style={{
                flex: 1,
                padding: '6px 8px',
                backgroundColor: '#1e1e1e',
                border: '1px solid #3e3e42',
                borderRadius: '4px',
                color: '#d4d4d4',
                fontFamily: 'ui-monospace, SFMono-Regular, Menlo, Consolas, monospace',
                fontSize: '13px',
              }}
            />
            <Button variant="small" onClick={handleAdd}>
              Add
            </Button>
          </div>
          {error ? (
            <div style={{ color: '#f48771', fontSize: '12px', flexShrink: 0 }}>
              {error}
            </div>
          ) : null}
          <textarea
            value={jsonText}
            onChange={(event) => persistText(event.target.value)}
            spellCheck={false}
            style={{
              flex: 1,
              minHeight: 0,
              width: '100%',
              boxSizing: 'border-box',
              padding: '10px',
              backgroundColor: '#1e1e1e',
              border: '1px solid #3e3e42',
              borderRadius: '4px',
              color: '#d4d4d4',
              fontFamily: 'ui-monospace, SFMono-Regular, Menlo, Consolas, monospace',
              fontSize: '13px',
              lineHeight: 1.4,
              resize: 'none',
            }}
          />
        </div>
      )}
    />
  );
}
