import { SpellTemplate, validateSpellAbilityRefs } from '../types/spell';
import {
  SpellTemplateForm,
  createDefaultSpellTemplate,
} from '../components/SpellTemplateForm';
import { useAssets } from '../contexts/AssetsContext';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { Sprite } from '../elements/Sprite';
import {
  TemplateEditorPage,
  TemplateEditorDescriptor,
} from './TemplateEditorPage';

interface SpellTemplatesProps {
  routeParams?: URLSearchParams;
}

export function SpellTemplates({ routeParams }: SpellTemplatesProps = {}) {
  const { spells, setSpells, saveSpells, abilities } = useAssets();
  const { spriteMap } = useSDL2WAssets();

  const descriptor: TemplateEditorDescriptor<SpellTemplate> = {
    editorKey: 'spellTemplates',
    title: 'Spell Templates Editor',
    entityNoun: 'spell',
    entityNounPlural: 'spells',
    searchPlaceholder: 'Search spells...',
    createLabel: '+ New Spell',
    emptyMessage: 'No spells found',
    getId: (s) => s.name,
    setId: (s, id) => {
      s.name = id;
    },
    getLabel: (s) => s.label,
    createDefault: createDefaultSpellTemplate,
    matchesSearch: (s, t) =>
      s.name.toLowerCase().includes(t) || s.label.toLowerCase().includes(t),
    toCardItem: (s) => ({
      name: s.name,
      label: s.label || s.name,
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
      validateSpellAbilityRefs(
        sorted,
        abilities.map((ability) => ability.name)
      ),
    afterSaveErrorDuration: 8000,
  };

  return (
    <TemplateEditorPage
      descriptor={descriptor}
      items={spells}
      setItems={setSpells}
      saveItems={saveSpells}
      routeParams={routeParams}
      renderForm={(spell, update) => (
        <SpellTemplateForm spell={spell} updateSpell={update} />
      )}
    />
  );
}
