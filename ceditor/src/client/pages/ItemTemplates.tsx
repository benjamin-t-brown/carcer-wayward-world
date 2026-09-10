import {
  ItemTemplateForm,
  createDefaultItem,
} from '../components/ItemTemplateForm';
import { useAssets } from '../contexts/AssetsContext';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { Sprite } from '../elements/Sprite';
import type { ItemTemplate } from '../types/assets';
import { prepareItemsForSave } from '../utils/formSavePreparation';
import {
  TemplateEditorPage,
  type TemplateEditorDescriptor,
} from './TemplateEditorPage';
import { validateItemTemplatesBeforeSave } from './itemTemplatesModel';

interface ItemTemplatesProps {
  routeParams?: URLSearchParams;
}

export function ItemTemplates({ routeParams }: ItemTemplatesProps = {}) {
  const { items, setItems, saveItems } = useAssets();
  const { spriteMap } = useSDL2WAssets();

  const descriptor: TemplateEditorDescriptor<ItemTemplate> = {
    editorKey: 'itemTemplates',
    title: 'Item Templates Editor',
    entityNoun: 'item',
    entityNounPlural: 'items',
    searchPlaceholder: 'Search items...',
    createLabel: '+ New Item',
    emptyMessage: 'No items found',
    getId: (item) => item.name,
    setId: (item, id) => {
      item.name = id;
    },
    getLabel: (item) => item.label,
    createDefault: createDefaultItem,
    matchesSearch: (item, term) =>
      item.name.toLowerCase().includes(term) ||
      item.label.toLowerCase().includes(term) ||
      item.itemType.toLowerCase().includes(term),
    prepareForSave: prepareItemsForSave,
    validateBeforeSave: validateItemTemplatesBeforeSave,
    formatSaveError: (error) =>
      `Error saving: ${error instanceof Error ? error.message : 'Unknown error'}`,
    deleteConfirmMessage: 'Are you sure you want to delete this item?',
    formWrapperId: 'item-form',
    scrollCardIntoView: true,
    scrollFormIntoView: true,
    renderCardMedia: (item) => {
      const sprite = spriteMap[item.icon];
      return sprite ? (
        <div style={{ display: 'inline-block' }}>
          <Sprite sprite={sprite} scale={1.5} />
        </div>
      ) : null;
    },
    renderCardMeta: (item) => (
      <span className="item-type">{item.itemType}</span>
    ),
  };

  return (
    <TemplateEditorPage
      descriptor={descriptor}
      items={items}
      setItems={setItems}
      saveItems={saveItems}
      routeParams={routeParams}
      renderForm={(item, update) => (
        <ItemTemplateForm item={item} updateItem={update} />
      )}
    />
  );
}
