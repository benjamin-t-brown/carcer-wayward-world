import { useState, useEffect } from 'react';
import { Choice, GameEvent } from '../../types/assets';
import { Button } from '../../elements/Button';
import { VariableWidget } from '../react-components/VariableWidget';
import { EditorNodeChoice } from '../cmpts/ChoiceNodeComponent';
import { notifyStateUpdated } from '../seEditorState';
import { ExecWidget } from '../react-components/ExecWidget';

interface EditChoiceNodeModalProps {
  isOpen: boolean;
  node: EditorNodeChoice | undefined;
  gameEvent: GameEvent;
  onCancel: () => void;
  ctx: CanvasRenderingContext2D;
}

const ChoiceTextInput = (props: {
  value: string;
  disabled: boolean;
  onChange: (value: string) => void;
  placeholder: string;
  monospace: boolean;
  borderColor?: string;
  spellCheck: boolean;
  widthPct: number;
}) => {
  const {
    value,
    disabled,
    onChange,
    placeholder,
    monospace,
    borderColor,
    spellCheck,
    widthPct,
  } = props;
  return (
    <input
      type="text"
      value={value}
      disabled={disabled}
      onChange={(e) => onChange(e.target.value)}
      spellCheck={spellCheck}
      style={{
        width: `calc(${widthPct}% - 12px)`,
        padding: '6px',
        backgroundColor: '#1e1e1e',
        border: '1px solid ' + (borderColor ? borderColor : '#3e3e42'),
        borderRadius: '4px',
        color: '#d4d4d4',
        fontFamily: monospace ? 'monospace' : 'arial',
        fontSize: '12px',
      }}
      placeholder={placeholder}
    />
  );
};

const UpDownMover = (props: {
  index: number;
  handleMoveChoiceDir: (index: number, direction: 'up' | 'down') => void;
}) => {
  const { index, handleMoveChoiceDir } = props;
  return (
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
        onClick={function () {
          return handleMoveChoiceDir(index, 'up');
        }}
      >
        <span>up</span>
      </button>
      <button
        style={{
          width: '50%',
        }}
        onClick={function () {
          return handleMoveChoiceDir(index, 'down');
        }}
      >
        <span>dn</span>
      </button>
    </div>
  );
};

export function EditChoiceNodeModal({
  isOpen,
  node,
  gameEvent,
  onCancel,
  ctx,
}: EditChoiceNodeModalProps) {
  const [choices, setChoices] = useState<Choice[]>([]);
  const [text, setText] = useState('');

  useEffect(() => {
    if (node) {
      const seNode = node.toSENode();
      setChoices([...seNode.choices]);
      setText(seNode.text || '');
    }
  }, [node]);

  if (!isOpen || !node) {
    return null;
  }

  const handleEditNodeConfirm = () => {
    node.text = text;
    node.buildFromChoices(choices, ctx);
    notifyStateUpdated();
    onCancel();
  };

  const handleAddChoice = () => {
    const newChoice: Choice = {
      text: '',
      conditionStr: '',
      next: '',
    };
    setChoices([...choices, newChoice]);
  };

  const handleRemoveChoice = (index: number) => {
    const newChoices = choices.filter((_, i) => i !== index);
    setChoices(newChoices);
  };

  const handleUpdateChoice = (
    index: number,
    field: keyof Choice,
    value: string
  ) => {
    const newChoices = [...choices];
    newChoices[index] = {
      ...newChoices[index],
      [field]: value,
    };
    setChoices(newChoices);
  };

  const handleMoveChoiceDir = (index: number, direction: 'up' | 'down') => {
    const newChoices = [...choices];
    if (direction === 'up') {
      if (index === 0) {
        return;
      }
      const choice = newChoices[index];
      newChoices[index] = newChoices[index - 1];
      newChoices[index - 1] = choice;
    } else {
      if (index === newChoices.length - 1) {
        return;
      }
      const choice = newChoices[index];
      newChoices[index] = newChoices[index + 1];
      newChoices[index + 1] = choice;
    }
    setChoices(newChoices);
  };

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
          maxWidth: '90vw',
          width: '90%',
          maxHeight: '80vh',
          overflow: 'auto',
        }}
        onClick={(e) => e.stopPropagation()}
      >
        <h2
          style={{
            color: '#4ec9b0',
            marginBottom: '20px',
            marginTop: 0,
          }}
        >
          Edit Choice Node
        </h2>

        <div
          style={{
            color: '#999',
            marginBottom: '20px',
            fontSize: '14px',
          }}
        >
          GameEvent: <span style={{ color: '#00d4d4' }}>{gameEvent.id}</span> |
          Node: <span style={{ color: '#d4d400' }}>{node.id}</span>
        </div>
        <div
          style={{
            display: 'flex',
            gap: '10px',
            width: '100%',
            marginBottom: '20px',
          }}
        >
          <div style={{ width: '50%' }}>
            <VariableWidget gameEvent={gameEvent} />
          </div>
          <div style={{ width: '50%' }}>
            <ExecWidget gameEvent={gameEvent} />
          </div>
        </div>
        <div style={{ marginBottom: '20px' }}>
          <div>
            <div
              style={{
                color: '#d4d4d4',
                fontSize: '14px',
                fontWeight: 'bold',
                marginBottom: '12px',
              }}
            >
              Text
            </div>
            <textarea
              value={text}
              onChange={(e) => setText(e.target.value)}
              style={{
                width: '100%',
                padding: '6px',
                backgroundColor: '#1e1e1e',
                border: '1px solid #3e3e42',
                borderRadius: '4px',
                color: '#d4d4d4',
                fontFamily: 'arial',
                fontSize: '16px',
              }}
            />
          </div>
        </div>

        <div style={{ marginBottom: '20px' }}>
          <div
            style={{
              display: 'flex',
              justifyContent: 'space-between',
              alignItems: 'center',
              marginBottom: '4px',
            }}
          >
            <label
              style={{
                color: '#d4d4d4',
                fontSize: '14px',
                fontWeight: 'bold',
              }}
            >
              Choices
            </label>
            <Button onClick={handleAddChoice}>+ Add Choice</Button>
          </div>

          {choices.length === 0 && (
            <div
              style={{
                color: '#666',
                fontStyle: 'italic',
                padding: '20px',
                textAlign: 'center',
                border: '1px dashed #3e3e42',
                borderRadius: '4px',
              }}
            >
              No choices. Click "Add Choice" to add one.
            </div>
          )}

          <div
            id="choice-node-choices"
            style={{
              height: '350px',
              overflow: 'auto',
            }}
          >
            {choices.map((choiceItem, index) => {
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
                    <UpDownMover
                      index={index}
                      handleMoveChoiceDir={handleMoveChoiceDir}
                    />
                    <div
                      style={{
                        width: 'calc(100% - 100px - 48px)',
                      }}
                    >
                      <ChoiceTextInput
                        value={choiceItem.conditionStr ?? ''}
                        widthPct={40}
                        disabled={index === 0}
                        onChange={(value) =>
                          handleUpdateChoice(index, 'conditionStr', value)
                        }
                        placeholder={
                          index !== 0
                            ? 'Enter condition text...'
                            : 'Default Condition'
                        }
                        monospace={true}
                        borderColor={
                          choiceItem.conditionStr ? '#862' : '#3e3e42'
                        }
                        spellCheck={false}
                      />
                      <ChoiceTextInput
                        value={choiceItem.text}
                        widthPct={60}
                        disabled={false}
                        onChange={(value) =>
                          handleUpdateChoice(index, 'text', value)
                        }
                        placeholder="Enter choice text..."
                        monospace={false}
                        spellCheck={true}
                      />
                      <br />
                      <ChoiceTextInput
                        value={choiceItem.evalStr ?? ''}
                        widthPct={40}
                        disabled={false}
                        onChange={(value) =>
                          handleUpdateChoice(index, 'evalStr', value)
                        }
                        placeholder={'Enter eval text...'}
                        monospace={true}
                        borderColor={choiceItem.evalStr ? '#862' : '#3e3e42'}
                        spellCheck={false}
                      />
                      <ChoiceTextInput
                        value={choiceItem.prefixText ?? ''}
                        widthPct={60}
                        disabled={false}
                        onChange={(value) =>
                          handleUpdateChoice(index, 'prefixText', value)
                        }
                        placeholder={'Enter prefix text...'}
                        monospace={false}
                        borderColor={choiceItem.prefixText ? '#862' : '#3e3e42'}
                        spellCheck={true}
                      />
                    </div>
                    <Button
                      variant="danger"
                      onClick={function () {
                        return handleRemoveChoice(index);
                      }}
                    >
                      Delete
                    </Button>
                  </div>
                </div>
              );
            })}
          </div>
        </div>

        <div
          style={{
            display: 'flex',
            justifyContent: 'flex-end',
            gap: '10px',
            marginTop: '30px',
          }}
        >
          <Button onClick={handleEditNodeConfirm}>Save</Button>
          <Button variant="secondary" onClick={onCancel}>
            Cancel
          </Button>
        </div>
      </div>
    </div>
  );
}
