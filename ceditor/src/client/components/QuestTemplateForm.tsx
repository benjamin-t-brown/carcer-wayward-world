import { useState } from 'react';
import { TextInput } from '../elements/TextInput';
import { TextArea } from '../elements/TextArea';
import { Button } from '../elements/Button';
import {
  QuestStep,
  QuestTemplate,
  createDefaultQuestStep,
  createDefaultQuestTemplate,
} from '../types/assets';
import { EditorEmptyState } from './EditorEmptyState';

export type { QuestTemplate };
export { createDefaultQuestTemplate };

interface QuestTemplateFormProps {
  quest?: QuestTemplate;
  updateQuest: (quest: QuestTemplate) => void;
}

function moveItem<T>(items: T[], index: number, direction: -1 | 1): T[] {
  const target = index + direction;
  if (target < 0 || target >= items.length) {
    return items;
  }
  const next = [...items];
  const item = next[index];
  next[index] = next[target];
  next[target] = item;
  return next;
}

function questStepStorageKey(questId: string, stepId: string): string {
  const quest = questId.trim() || '<quest id>';
  const step = stepId.trim() || '<step id>';
  return `vars.quests.${quest}.step=${step}`;
}

function CopyableStorageKey({ value }: { value: string }) {
  const [copied, setCopied] = useState(false);

  const copy = async () => {
    try {
      await navigator.clipboard.writeText(value);
      setCopied(true);
      window.setTimeout(() => setCopied(false), 1500);
    } catch {
      // Selecting the field is enough to copy manually.
    }
  };

  return (
    <button
      type="button"
      className="quest-step-storage-key"
      title="Copy storage key"
      onClick={() => void copy()}
    >
      {copied ? 'Copied' : value}
    </button>
  );
}

function QuestStepEditor({
  step,
  questId,
  idPrefix,
  onChange,
  onRemove,
  onMove,
  allowSubSteps,
  canMoveUp,
  canMoveDown,
}: {
  step: QuestStep;
  questId: string;
  idPrefix: string;
  onChange: (step: QuestStep) => void;
  onRemove: () => void;
  onMove: (direction: -1 | 1) => void;
  allowSubSteps: boolean;
  canMoveUp: boolean;
  canMoveDown: boolean;
}) {
  const subSteps = step.subSteps ?? [];
  const updateField = <K extends keyof QuestStep>(field: K, value: QuestStep[K]) => {
    onChange({ ...step, [field]: value });
  };

  return (
    <div className="quest-step-item">
      <div className="quest-step-row">
        <div className="quest-step-fields">
          <div className="form-fields-inline">
            <div className="quest-step-id-block">
              <TextInput
                id={`${idPrefix}-id`}
                name={`${idPrefix}Id`}
                label="Step ID"
                value={step.id}
                onChange={(value) => updateField('id', value)}
                required
              />
              <CopyableStorageKey value={questStepStorageKey(questId, step.id)} />
            </div>
            <TextInput
              id={`${idPrefix}-label`}
              name={`${idPrefix}Label`}
              label="Label"
              value={step.label}
              onChange={(value) => updateField('label', value)}
            />
          </div>
          <TextArea
            id={`${idPrefix}-description`}
            name={`${idPrefix}Description`}
            label="Description"
            value={step.description}
            onChange={(value) => updateField('description', value)}
            rows={2}
          />
        </div>
        <div className="quest-step-actions">
          <Button
            type="button"
            variant="small"
            disabled={!canMoveUp}
            onClick={() => onMove(-1)}
          >
            Up
          </Button>
          <Button
            type="button"
            variant="small"
            disabled={!canMoveDown}
            onClick={() => onMove(1)}
          >
            Down
          </Button>
          <Button
            type="button"
            variant="small"
            className="btn-danger"
            onClick={onRemove}
          >
            Remove
          </Button>
        </div>
      </div>
      {allowSubSteps ? (
        <div className="quest-step-substeps">
          <h4>Sub-steps</h4>
          {subSteps.map((subStep, subIndex) => (
            <QuestStepEditor
              key={`${idPrefix}-sub-${subIndex}`}
              step={subStep}
              questId={questId}
              idPrefix={`${idPrefix}-sub-${subIndex}`}
              allowSubSteps={false}
              canMoveUp={subIndex > 0}
              canMoveDown={subIndex < subSteps.length - 1}
              onChange={(updated) => {
                const next = [...subSteps];
                next[subIndex] = updated;
                updateField('subSteps', next);
              }}
              onRemove={() =>
                updateField(
                  'subSteps',
                  subSteps.filter((_, i) => i !== subIndex),
                )
              }
              onMove={(direction) =>
                updateField('subSteps', moveItem(subSteps, subIndex, direction))
              }
            />
          ))}
          <Button
            type="button"
            variant="secondary"
            onClick={() =>
              updateField('subSteps', [
                ...subSteps,
                { ...createDefaultQuestStep(), id: `sub${subSteps.length + 1}` },
              ])
            }
          >
            + Add sub-step
          </Button>
        </div>
      ) : null}
    </div>
  );
}

export function QuestTemplateForm(props: QuestTemplateFormProps) {
  const quest = props.quest;

  if (!quest) {
    return <EditorEmptyState message="Select a quest to edit" />;
  }

  const setFormData = (data: QuestTemplate) => {
    props.updateQuest(data);
  };

  const updateField = <K extends keyof QuestTemplate>(
    field: K,
    value: QuestTemplate[K],
  ) => {
    setFormData({ ...quest, [field]: value });
  };

  const steps = quest.steps ?? [];

  return (
    <div className="item-form quest-template-form">
      <h2>Edit Quest</h2>
      <form>
        <div className="form-fields-inline">
          <TextInput
            id="quest-id"
            name="id"
            label="ID"
            value={quest.id}
            onChange={(value) => updateField('id', value)}
            required
          />
          <TextInput
            id="quest-label"
            name="label"
            label="Label"
            value={quest.label}
            onChange={(value) => updateField('label', value)}
            required
          />
        </div>
        <TextArea
          id="quest-description"
          name="description"
          label="Description"
          value={quest.description}
          onChange={(value) => updateField('description', value)}
          rows={3}
        />
        <TextArea
          id="quest-completed-description"
          name="completedDescription"
          label="Completed description"
          value={quest.completedDescription}
          onChange={(value) => updateField('completedDescription', value)}
          rows={3}
        />
        <div className="form-subsection">
          <h4>Steps</h4>
          <p className="form-subsection-description">
            Current step is vars.quests.&lt;id&gt;.step. START_QUEST sets the
            first top-level step. COMPLETE_QUEST_STEP marks
            vars.quests.&lt;id&gt;.completed.&lt;stepId&gt;. COMPLETE_QUEST sets
            the step to complete.
          </p>
          {steps.map((step, index) => (
            <QuestStepEditor
              key={`step-${index}`}
              step={step}
              questId={quest.id}
              idPrefix={`step-${index}`}
              allowSubSteps
              canMoveUp={index > 0}
              canMoveDown={index < steps.length - 1}
              onChange={(updated) => {
                const next = [...steps];
                next[index] = updated;
                updateField('steps', next);
              }}
              onRemove={() =>
                updateField(
                  'steps',
                  steps.filter((_, i) => i !== index),
                )
              }
              onMove={(direction) =>
                updateField('steps', moveItem(steps, index, direction))
              }
            />
          ))}
          <Button
            type="button"
            variant="secondary"
            onClick={() =>
              updateField('steps', [
                ...steps,
                { ...createDefaultQuestStep(), id: `step${steps.length + 1}` },
              ])
            }
          >
            + Add step
          </Button>
        </div>
      </form>
    </div>
  );
}
