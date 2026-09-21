import { StatusEffectTemplate } from '../types/ability';
import {
  StatusEffectTemplateForm,
  createDefaultStatusEffectTemplate,
} from '../components/StatusEffectTemplateForm';
import { useAssets } from '../contexts/AssetsContext';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { Sprite } from '../elements/Sprite';
import {
  TemplateEditorPage,
  TemplateEditorDescriptor,
} from './TemplateEditorPage';

export function StatusEffectTemplates() {
  const { statusEffects, setStatusEffects, saveStatusEffects } = useAssets();
  const { spriteMap } = useSDL2WAssets();

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
    toCardItem: (s) => ({
      name: s.name,
      label: s.name,
      icon: s.icon,
    }),
    renderAdditionalInfo: (item) => {
      const icon = item.icon as string | undefined;
      const sprite = icon ? spriteMap[icon] : undefined;
      if (!sprite) {
        return null;
      }
      return (
        <div
          className="item-info"
          style={{ display: 'flex', alignItems: 'center', gap: '8px' }}
        >
          <div style={{ display: 'inline-block' }}>
            <Sprite sprite={sprite} scale={1.5} />
          </div>
        </div>
      );
    },
    validateAfterSave: (sorted) =>
      sorted
        .filter((s) => !s.icon?.trim())
        .map((s) => `${s.name || '(unnamed)'} is missing an icon`),
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
