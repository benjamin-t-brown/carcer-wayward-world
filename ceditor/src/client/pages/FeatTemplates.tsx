import { FeatTemplate } from '../types/assets';
import {
  FeatTemplateForm,
  createDefaultFeatTemplate,
} from '../components/FeatTemplateForm';
import { useAssets } from '../contexts/AssetsContext';
import {
  TemplateEditorPage,
  TemplateEditorDescriptor,
} from './TemplateEditorPage';

export function FeatTemplates() {
  const { feats, setFeats, saveFeats } = useAssets();

  const descriptor: TemplateEditorDescriptor<FeatTemplate> = {
    editorKey: 'featTemplates',
    title: 'Feat Templates Editor',
    entityNoun: 'feat',
    entityNounPlural: 'feats',
    searchPlaceholder: 'Search feats...',
    createLabel: '+ New Feat',
    emptyMessage: 'No feats found',
    deleteConfirmMessage: 'Are you sure you want to delete this feat?',
    getId: (f) => f.id,
    setId: (f, id) => {
      f.id = id;
    },
    getLabel: (f) => f.label,
    createDefault: createDefaultFeatTemplate,
    matchesSearch: (f, t) =>
      f.id.toLowerCase().includes(t) || f.label.toLowerCase().includes(t),
    compare: (a, b) => {
      const cmp = a.id.localeCompare(b.id);
      return cmp === 0 ? a.label.localeCompare(b.label) : cmp;
    },
    toCardItem: (f) => ({
      name: f.id,
      label: f.label || f.id,
      implementation: f.implementation || 'DATA',
    }),
    renderAdditionalInfo: (item) => (
      <div className="item-info">
        <span className="item-type">
          {(item.implementation as string) || 'DATA'}
        </span>
      </div>
    ),
    formWrapperId: 'feat-form',
    scrollCardIntoView: true,
  };

  return (
    <TemplateEditorPage
      descriptor={descriptor}
      items={feats}
      setItems={setFeats}
      saveItems={saveFeats}
      renderForm={(feat, update) => (
        <FeatTemplateForm feat={feat} updateFeat={update} />
      )}
    />
  );
}
