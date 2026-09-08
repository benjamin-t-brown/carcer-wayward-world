import { StatusEffectTemplate } from '../types/ability';
import {
  StatusEffectTemplateForm,
  createDefaultStatusEffectTemplate,
} from '../components/StatusEffectTemplateForm';
import { useAssets } from '../contexts/AssetsContext';
import {
  TemplateEditorPage,
  TemplateEditorDescriptor,
} from './TemplateEditorPage';

export function StatusEffectTemplates() {
  const { statusEffects, setStatusEffects, saveStatusEffects } = useAssets();

  const descriptor: TemplateEditorDescriptor<StatusEffectTemplate> = {
    editorKey: 'statusEffectTemplates',
    title: 'Status Effect Templates Editor',
    entityNoun: 'status effect',
    entityNounPlural: 'status effects',
    searchPlaceholder: 'Search status effects...',
    createLabel: '+ New Status Effect',
    emptyMessage: 'No status effects found',
    getId: (s) => s.name,
    setId: (s, id) => {
      s.name = id;
    },
    getLabel: (s) => s.name,
    createDefault: createDefaultStatusEffectTemplate,
    matchesSearch: (s, t) =>
      s.name.toLowerCase().includes(t) ||
      s.description.toLowerCase().includes(t),
  };

  return (
    <TemplateEditorPage
      descriptor={descriptor}
      items={statusEffects}
      setItems={setStatusEffects}
      saveItems={saveStatusEffects}
      renderForm={(statusEffect, update) => (
        <StatusEffectTemplateForm
          statusEffect={statusEffect}
          updateStatusEffect={update}
        />
      )}
    />
  );
}
