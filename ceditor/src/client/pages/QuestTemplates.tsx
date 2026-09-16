import { QuestTemplate } from '../types/assets';
import {
  QuestTemplateForm,
  createDefaultQuestTemplate,
} from '../components/QuestTemplateForm';
import { useAssets } from '../contexts/AssetsContext';
import {
  TemplateEditorPage,
  TemplateEditorDescriptor,
} from './TemplateEditorPage';

export function QuestTemplates({
  routeParams,
}: {
  routeParams?: URLSearchParams;
} = {}) {
  const { quests, setQuests, saveQuests } = useAssets();

  const descriptor: TemplateEditorDescriptor<QuestTemplate> = {
    editorKey: 'questTemplates',
    title: 'Quests Editor',
    entityNoun: 'quest',
    entityNounPlural: 'quests',
    searchPlaceholder: 'Search quests...',
    createLabel: '+ New Quest',
    emptyMessage: 'No quests found',
    deleteConfirmMessage: 'Are you sure you want to delete this quest?',
    getId: (q) => q.id,
    setId: (q, id) => {
      q.id = id;
    },
    getLabel: (q) => q.label,
    createDefault: createDefaultQuestTemplate,
    matchesSearch: (q, t) =>
      q.id.toLowerCase().includes(t) ||
      q.label.toLowerCase().includes(t) ||
      (q.description || '').toLowerCase().includes(t),
    compare: (a, b) => {
      const cmp = a.id.localeCompare(b.id);
      return cmp === 0 ? a.label.localeCompare(b.label) : cmp;
    },
    toCardItem: (q) => ({
      name: q.id,
      label: q.label || q.id,
      stepCount: q.steps?.length ?? 0,
    }),
    renderAdditionalInfo: (item) => (
      <div className="item-info">
        <span className="item-type">{String(item.stepCount ?? 0)} steps</span>
      </div>
    ),
    formWrapperId: 'quest-form',
    scrollCardIntoView: true,
    validateAfterSave: (sorted) => {
      const errors: string[] = [];
      const ids = new Set<string>();
      for (const quest of sorted) {
        if (!quest.id.trim()) {
          errors.push('A quest is missing an id');
          continue;
        }
        if (ids.has(quest.id)) {
          errors.push(`Duplicate quest id: ${quest.id}`);
        }
        ids.add(quest.id);
        const stepIds = new Set<string>();
        for (const step of quest.steps ?? []) {
          if (!step.id.trim()) {
            errors.push(`Quest ${quest.id} has a step with no id`);
            continue;
          }
          if (stepIds.has(step.id)) {
            errors.push(`Quest ${quest.id} has duplicate step id: ${step.id}`);
          }
          stepIds.add(step.id);
        }
      }
      return errors;
    },
  };

  return (
    <TemplateEditorPage
      descriptor={descriptor}
      items={quests}
      setItems={setQuests}
      saveItems={saveQuests}
      routeParams={routeParams}
      renderForm={(quest, update) => (
        <QuestTemplateForm quest={quest} updateQuest={update} />
      )}
    />
  );
}
