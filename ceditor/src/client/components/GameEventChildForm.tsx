import { useState } from 'react';
import { TextInput } from '../elements/TextInput';
import { TextArea } from '../elements/TextArea';
import { OptionSelect } from '../elements/OptionSelect';
import { Button } from '../elements/Button';

export type GameEventChildType =
  | 'KEYWORD'
  | 'CHOICE'
  | 'SWITCH'
  | 'EXEC'
  | 'END';

export interface GameEventChildKeyword {
  eventChildType: 'KEYWORD';
  id: string;
  keywords: Record<string, any>; // Simplified for now
}

export interface GameEventChildChoice {
  eventChildType: 'CHOICE';
  id: string;
  text: string;
  choices: Array<{
    text: string;
    conditionStr?: string;
    next: string;
  }>;
}

export interface GameEventChildSwitch {
  eventChildType: 'SWITCH';
  id: string;
  defaultNext: string;
  cases: Array<{
    conditionStr: string;
    next: string;
  }>;
}

export interface GameEventChildExec {
  eventChildType: 'EXEC';
  id: string;
  paragraphs: string;
  execStr: string;
  next: string;
}

export interface GameEventChildEnd {
  eventChildType: 'END';
  id: string;
  next: string;
}

export type GameEventChild =
  | GameEventChildKeyword
  | GameEventChildChoice
  | GameEventChildSwitch
  | GameEventChildExec
  | GameEventChildEnd;

export type GameEventChildUnion = GameEventChildKeyword &
  GameEventChildChoice &
  GameEventChildSwitch &
  GameEventChildExec &
  GameEventChildEnd;

interface GameEventChildFormProps {
  child: GameEventChild;
  index: number;
  isExpanded: boolean;
  onToggle: () => void;
  onUpdate: (child: GameEventChild) => void;
  onDelete: () => void;
  onClone: () => void;
}

const CHILD_TYPES: { value: GameEventChildType; label: string }[] = [
  { value: 'KEYWORD', label: 'Keyword' },
  { value: 'CHOICE', label: 'Choice' },
  { value: 'SWITCH', label: 'Switch' },
  { value: 'EXEC', label: 'Exec' },
  { value: 'END', label: 'End' },
];

export function createDefaultChild(): GameEventChild {
  return {
    eventChildType: 'EXEC',
    id: '',
    paragraphs: '',
    execStr: '',
    next: '',
  };
}

export function GameEventChildForm({
  child,
  index,
  isExpanded,
  onToggle,
  onUpdate,
  onDelete,
  onClone,
}: GameEventChildFormProps) {
  const [formData, setFormData] = useState<GameEventChild>(child);

  const updateField = <K extends keyof GameEventChildUnion>(
    field: K,
    value: GameEventChildUnion[K]
  ) => {
    const updated = { ...formData, [field]: value } as GameEventChildUnion;
    setFormData(updated);
    onUpdate(updated);
  };

  const handleTypeChange = (newType: GameEventChildType) => {
    // When type changes, we need to create a new child of that type
    // For now, just update the type - in a real implementation you might want
    // to warn the user about data loss
    updateField(
      'eventChildType',
      newType as GameEventChildUnion['eventChildType']
    );
  };

  const renderForm = () => {
    switch (formData.eventChildType) {
      case 'KEYWORD':
        const keywordChild = formData as GameEventChildKeyword;
        return (
          <>
            <TextInput
              id={`child-${index}-id`}
              name="id"
              label="ID"
              value={keywordChild.id}
              onChange={(value) =>
                updateField('id', value as GameEventChildUnion['id'])
              }
              placeholder="e.g., keyword1"
              required
            />
            <div
              style={{ color: '#858585', fontSize: '12px', marginTop: '10px' }}
            >
              Keyword editing will be implemented later
            </div>
          </>
        );

      case 'CHOICE':
        const choiceChild = formData as GameEventChildChoice;
        const updateChoice = (choiceIndex: number, field: 'text' | 'conditionStr' | 'next', value: string) => {
          const newChoices = [...(choiceChild.choices || [])];
          if (!newChoices[choiceIndex]) {
            newChoices[choiceIndex] = { text: '', next: '' };
          }
          newChoices[choiceIndex] = { ...newChoices[choiceIndex], [field]: value };
          updateField('choices', newChoices as GameEventChildUnion['choices']);
        };
        const addChoice = () => {
          const newChoices = [...(choiceChild.choices || []), { text: '', next: '' }];
          updateField('choices', newChoices as GameEventChildUnion['choices']);
        };
        const removeChoice = (choiceIndex: number) => {
          const newChoices = (choiceChild.choices || []).filter((_, i) => i !== choiceIndex);
          updateField('choices', newChoices as GameEventChildUnion['choices']);
        };
        return (
          <>
            <TextInput
              id={`child-${index}-id`}
              name="id"
              label="ID"
              value={choiceChild.id}
              onChange={(value) =>
                updateField('id', value as GameEventChildUnion['id'])
              }
              placeholder="e.g., choice1"
              required
            />
            <TextArea
              id={`child-${index}-text`}
              name="text"
              label="Text"
              value={choiceChild.text}
              onChange={(value) =>
                updateField('text', value as GameEventChildUnion['text'])
              }
              placeholder="Choice text"
              rows={3}
            />
            <div className="form-group">
              <label>Choices</label>
              {(choiceChild.choices || []).map((choice, choiceIndex) => (
                <div
                  key={choiceIndex}
                  style={{
                    border: '1px solid #3e3e42',
                    borderRadius: '4px',
                    padding: '15px',
                    marginBottom: '15px',
                    backgroundColor: '#1e1e1e',
                  }}
                >
                  <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '10px' }}>
                    <strong style={{ color: '#4ec9b0' }}>Choice {choiceIndex + 1}</strong>
                    <Button
                      variant="small"
                      className="btn-danger"
                      onClick={() => removeChoice(choiceIndex)}
                    >
                      Remove
                    </Button>
                  </div>
                  <TextInput
                    id={`child-${index}-choice-${choiceIndex}-text`}
                    name={`choice-${choiceIndex}-text`}
                    label="Text"
                    value={choice.text || ''}
                    onChange={(value) => updateChoice(choiceIndex, 'text', value)}
                    placeholder="Choice text"
                  />
                  <TextInput
                    id={`child-${index}-choice-${choiceIndex}-condition`}
                    name={`choice-${choiceIndex}-condition`}
                    label="Condition (optional)"
                    value={choice.conditionStr || ''}
                    onChange={(value) => updateChoice(choiceIndex, 'conditionStr', value)}
                    placeholder="Condition string"
                  />
                  <TextInput
                    id={`child-${index}-choice-${choiceIndex}-next`}
                    name={`choice-${choiceIndex}-next`}
                    label="Next"
                    value={choice.next || ''}
                    onChange={(value) => updateChoice(choiceIndex, 'next', value)}
                    placeholder="ID of next child"
                  />
                </div>
              ))}
              <Button variant="secondary" onClick={addChoice}>
                + Add Choice
              </Button>
            </div>
          </>
        );

      case 'SWITCH':
        const switchChild = formData as GameEventChildSwitch;
        return (
          <>
            <TextInput
              id={`child-${index}-id`}
              name="id"
              label="ID"
              value={switchChild.id}
              onChange={(value) =>
                updateField('id', value as GameEventChildUnion['id'])
              }
              placeholder="e.g., switch1"
              required
            />
            <TextInput
              id={`child-${index}-default-next`}
              name="defaultNext"
              label="Default Next"
              value={switchChild.defaultNext}
              onChange={(value) =>
                updateField(
                  'defaultNext',
                  value as GameEventChildUnion['defaultNext']
                )
              }
              placeholder="ID of default next child"
            />
            <div
              style={{ color: '#858585', fontSize: '12px', marginTop: '10px' }}
            >
              Cases array editing will be implemented later
            </div>
          </>
        );

      case 'EXEC':
        const execChild = formData as GameEventChildExec;
        return (
          <>
            <TextInput
              id={`child-${index}-id`}
              name="id"
              label="ID"
              value={execChild.id}
              onChange={(value) =>
                updateField('id', value as GameEventChildUnion['id'])
              }
              placeholder="e.g., exec1"
              required
            />
            <TextArea
              id={`child-${index}-paragraphs`}
              name="paragraphs"
              label="Paragraphs"
              value={execChild.paragraphs || ''}
              onChange={(value) =>
                updateField('paragraphs', value as GameEventChildUnion['paragraphs'])
              }
              placeholder="Paragraphs text"
              rows={4}
            />
            <TextArea
              id={`child-${index}-exec-str`}
              name="execStr"
              label="Exec String"
              value={execChild.execStr}
              onChange={(value) =>
                updateField('execStr', value as GameEventChildUnion['execStr'])
              }
              placeholder="Execution string"
              rows={3}
            />
            <TextInput
              id={`child-${index}-next`}
              name="next"
              label="Next"
              value={execChild.next}
              onChange={(value) =>
                updateField('next', value as GameEventChildUnion['next'])
              }
              placeholder="ID of next child"
            />
          </>
        );

      case 'END':
        const endChild = formData as GameEventChildEnd;
        return (
          <>
            <TextInput
              id={`child-${index}-id`}
              name="id"
              label="ID"
              value={endChild.id}
              onChange={(value) =>
                updateField('id', value as GameEventChildUnion['id'])
              }
              placeholder="e.g., end1"
              required
            />
            <TextInput
              id={`child-${index}-next`}
              name="next"
              label="Next"
              value={endChild.next}
              onChange={(value) =>
                updateField('next', value as GameEventChildUnion['next'])
              }
              placeholder="ID of next child"
            />
          </>
        );
    }
  };

  return (
    <div
      style={{
        border: '1px solid #3e3e42',
        borderRadius: '4px',
        marginBottom: '10px',
        overflow: 'hidden',
      }}
    >
      {/* Accordion Header */}
      <div
        style={{
          padding: '12px 15px',
          backgroundColor: isExpanded ? '#2d2d30' : '#1e1e1e',
          borderBottom: isExpanded ? '1px solid #3e3e42' : 'none',
          cursor: 'pointer',
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
          userSelect: 'none',
        }}
        onClick={onToggle}
      >
        <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
          <span
            style={{
              color: '#4ec9b0',
              fontSize: '14px',
              fontWeight: 'bold',
              minWidth: '80px',
            }}
          >
            ID: {formData.id || 'No ID'}
          </span>
          <span style={{ color: '#858585', fontSize: '12px' }}>
            {formData.eventChildType}
          </span>
        </div>
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: '8px',
          }}
          onClick={(e) => e.stopPropagation()}
        >
          <button
            onClick={(ev) => {
              ev.stopPropagation();
              ev.preventDefault();
              onClone();
            }}
            style={{
              padding: '4px 8px',
              backgroundColor: '#3e3e42',
              border: '1px solid #3e3e42',
              borderRadius: '4px',
              color: '#d4d4d4',
              cursor: 'pointer',
              fontSize: '12px',
            }}
          >
            Clone
          </button>
          <button
            onClick={(ev) => {
              ev.stopPropagation();
              ev.preventDefault();
              onDelete();
            }}
            style={{
              padding: '4px 8px',
              backgroundColor: '#5a1d1d',
              border: '1px solid #a1260d',
              borderRadius: '4px',
              color: '#f48771',
              cursor: 'pointer',
              fontSize: '12px',
            }}
          >
            Delete
          </button>
          <span
            style={{
              color: '#858585',
              fontSize: '18px',
              marginLeft: '8px',
              transform: isExpanded ? 'rotate(90deg)' : 'rotate(0deg)',
              transition: 'transform 0.2s',
            }}
          >
            ▶
          </span>
        </div>
      </div>

      {/* Accordion Content */}
      {isExpanded && (
        <div
          style={{
            padding: '20px',
            backgroundColor: '#252526',
          }}
        >
          <OptionSelect
            id={`child-${index}-type`}
            name="childType"
            label="Child Type"
            value={formData.eventChildType}
            onChange={(value) => handleTypeChange(value as GameEventChildType)}
            options={CHILD_TYPES}
            required
          />
          <div style={{ marginTop: '20px' }}>{renderForm()}</div>
        </div>
      )}
    </div>
  );
}
