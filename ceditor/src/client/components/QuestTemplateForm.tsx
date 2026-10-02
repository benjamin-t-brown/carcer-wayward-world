import { useState } from 'react';
import { TextInput } from '../elements/TextInput';
import { TextArea } from '../elements/TextArea';
import { Button } from '../elements/Button';
import {
  QuestStep,
  QuestTemplate,
  createDefaultQuestRewardItem,
  createDefaultQuestRewards,
  createDefaultQuestStep,
  createDefaultQuestTemplate,
  normalizeQuestRewardItems,
} from '../types/assets';
import { EditorEmptyState } from './EditorEmptyState';
import { NumberInput } from '../elements/NumberInput';
import { useAssets } from '../contexts/AssetsContext';
import { ItemSearchInput } from '../tile-editor/react-components/SelectedTileInfo/ItemSearchInput';

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

function questIdPlaceholder(questId: string): string {
  return questId.trim() || '<quest id>';
}

function questStartedStorageKey(questId: string): string {
  return `vars.quests.${questIdPlaceholder(questId)}.step`;
}

function questCompleteStorageKey(questId: string): string {
  return `vars.quests.${questIdPlaceholder(questId)}.step=complete`;
}

function questStepStorageKey(questId: string, stepId: string): string {
  const step = stepId.trim() || '<step id>';
  return `${questStartedStorageKey(questId)}=${step}`;
}

function questCompletedStorageKey(questId: string, stepId: string): string {
  const quest = questIdPlaceholder(questId);
  const step = stepId.trim() || '<step id>';
  return `vars.quests.${quest}.completed.${step}`;
}

function questShownStorageKey(questId: string, stepId: string): string {
  const quest = questIdPlaceholder(questId);
  const step = stepId.trim() || '<step id>';
  return `vars.quests.${quest}.shown.${step}`;
}

function CopyableStorageKey({
  value,
  title = 'Copy storage key',
}: {
  value: string;
  title?: string;
}) {
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
      {copied ? value + ' Copied!' : value}
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
  const updateField = <K extends keyof QuestStep>(
    field: K,
    value: QuestStep[K],
  ) => {
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
              <CopyableStorageKey
                value={questStepStorageKey(questId, step.id)}
              />
              <CopyableStorageKey
                value={questCompletedStorageKey(questId, step.id)}
              />
              {!allowSubSteps ? (
                <CopyableStorageKey
                  value={questShownStorageKey(questId, step.id)}
                />
              ) : null}
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
                {
                  ...createDefaultQuestStep(),
                  id: `sub${subSteps.length + 1}`,
                },
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

function QuestRewardsEditor({
  quest,
  onChange,
}: {
  quest: QuestTemplate;
  onChange: (rewards: NonNullable<QuestTemplate['rewards']>) => void;
}) {
  const { items } = useAssets();
  const rewards = {
    ...createDefaultQuestRewards(),
    ...quest.rewards,
    coins: Math.max(0, Math.trunc(quest.rewards?.coins ?? 0)),
    experience: Math.max(0, Math.trunc(quest.rewards?.experience ?? 0)),
    items: normalizeQuestRewardItems(quest.rewards?.items),
  };

  const itemLabel = (itemName: string) => {
    const item = items.find((entry) => entry.name === itemName);
    return item?.label?.trim() || itemName;
  };

  const updateItemAmount = (index: number, amount: number) => {
    const next = rewards.items.map((entry, i) =>
      i === index ? createDefaultQuestRewardItem(entry.name, amount) : entry,
    );
    onChange({ ...rewards, items: next });
  };

  return (
    <div className="form-subsection">
      <h4>Rewards</h4>
      <p className="form-subsection-description">
        Granted the first time COMPLETE_QUEST runs. Coins and experience go to
        the player. Each item is added in the amount you set.
      </p>
      <div className="form-fields-inline">
        <NumberInput
          id="quest-reward-coins"
          name="rewardCoins"
          label="Coins"
          value={rewards.coins}
          onChange={(value) =>
            onChange({ ...rewards, coins: Math.max(0, Math.trunc(value || 0)) })
          }
          min={0}
        />
        <NumberInput
          id="quest-reward-experience"
          name="rewardExperience"
          label="Experience"
          value={rewards.experience}
          onChange={(value) =>
            onChange({
              ...rewards,
              experience: Math.max(0, Math.trunc(value || 0)),
            })
          }
          min={0}
        />
      </div>
      <div className="quest-reward-items">
        <div className="quest-reward-items-label">Items</div>
        {rewards.items.length === 0 ? (
          <p className="quest-reward-empty">No reward items</p>
        ) : (
          rewards.items.map((entry, index) => (
            <div key={`${entry.name}-${index}`} className="quest-reward-item">
              <span className="quest-reward-item-info">
                {itemLabel(entry.name)}
                <span className="quest-reward-item-name"> ({entry.name})</span>
              </span>
              <label className="quest-reward-item-amount-label">
                Amt
                <input
                  type="number"
                  min={1}
                  className="quest-reward-item-amount"
                  value={entry.amount}
                  onChange={(event) =>
                    updateItemAmount(index, Number(event.target.value))
                  }
                />
              </label>
              <Button
                type="button"
                variant="small"
                className="btn-danger"
                onClick={() =>
                  onChange({
                    ...rewards,
                    items: rewards.items.filter((_, i) => i !== index),
                  })
                }
              >
                Remove
              </Button>
            </div>
          ))
        )}
        <ItemSearchInput
          items={items}
          placeholder="Add reward item..."
          dropdownPlacement="auto"
          onSelect={(itemName) => {
            if (!itemName.trim()) {
              return;
            }
            const existing = rewards.items.findIndex(
              (entry) => entry.name === itemName,
            );
            if (existing >= 0) {
              updateItemAmount(existing, rewards.items[existing].amount + 1);
              return;
            }
            onChange({
              ...rewards,
              items: [
                ...rewards.items,
                createDefaultQuestRewardItem(itemName, 1),
              ],
            });
          }}
        />
      </div>
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
        <div className="quest-storage-keys">
          <CopyableStorageKey
            value={questStartedStorageKey(quest.id)}
            title="Copy started storage key (set when START_QUEST runs)"
          />
          <CopyableStorageKey
            value={questCompleteStorageKey(quest.id)}
            title="Copy completed storage key (vars.quests.<id>.step=complete)"
          />
        </div>
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
        <QuestRewardsEditor
          quest={quest}
          onChange={(rewards) => updateField('rewards', rewards)}
        />
        <div className="form-subsection">
          <h4>Steps</h4>
          <p className="form-subsection-description">
            NOTE: Steps are not linear. Shown or completed sub-steps appear in
            the journal; hidden incomplete ones do not.
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
