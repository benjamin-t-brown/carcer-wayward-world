import { useState, useRef, ReactNode } from 'react';
import { AssetTree, scrollAssetTreeItemIntoView } from '../components/AssetTree';
import { EditorSidebar } from '../components/EditorSidebar';
import { useAssetOrganization } from '../contexts/OrganizationContext';
import type { OrganizableAssetId } from '../../shared/editorOrganization';
import { EditorHeader } from '../components/EditorHeader';
import { Notification } from '../elements/Notification';
import { trimStrings } from '../utils/jsonUtils';
import { usePersistedEditorSelection } from '../hooks/usePersistedEditorSelection';
import { useSaveHotkey } from '../hooks/useSaveHotkey';
import { EditorSelectionKey } from '../utils/editorSelectionStorage';
import type { AssetId } from '../../shared/assetRegistry';

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
  duration?: number;
}

type CardItem = {
  name: string;
  label?: string;
  /** When set (including ''), overrides the default subtitle of `name`. */
  subtitle?: string;
} & Record<string, unknown>;

/**
 * Everything a plain "list + form" template editor page needs. Pages that add
 * tab state, canvas editors, autosave, cascading deletes, or a create menu
 * (Maps, SpecialEvents, Tilesets, MapGrids, Abilities, Items) are intentionally
 * NOT built on this.
 */
export interface TemplateEditorDescriptor<T> {
  editorKey: EditorSelectionKey;
  /** This page's asset list. Reload leaves it untouched. */
  preserveAsset: AssetId;
  title: string; // 'Spell Templates Editor'
  entityNoun: string; // 'spell' — used in clone/delete copy
  entityNounPlural: string; // 'spells'
  searchPlaceholder: string;
  createLabel: string;
  emptyMessage: string;

  getId(item: T): string;
  setId(item: T, id: string): void;
  getLabel(item: T): string;
  createDefault(): T;
  matchesSearch(item: T, lowerCaseTerm: string): boolean;
  /** Sort order applied on save. Default: getId localeCompare. */
  compare?(a: T, b: T): number;

  /** Row passed to the sidebar tree. Default: { name: getId, label: getLabel || getId }. */
  toCardItem?(item: T): CardItem;
  renderAdditionalInfo?(cardItem: CardItem): ReactNode;

  /** Text in the delete confirm(). Default: `Delete this <noun>?`. */
  deleteConfirmMessage?: string;
  /** id attribute on a wrapper div around the form. */
  formWrapperId?: string;
  /** Scroll the affected card into view after clone / create. */
  scrollCardIntoView?: boolean;

  /**
   * Extra validation run against the sorted list after a successful save.
   * Returned strings surface as an error toast; the save itself still happened.
   */
  validateAfterSave?(sorted: T[]): string[];
  /** Toast duration (ms) when validateAfterSave returns errors. */
  afterSaveErrorDuration?: number;
}

interface TemplateEditorPageProps<T> {
  descriptor: TemplateEditorDescriptor<T>;
  items: T[];
  setItems: (items: T[]) => void;
  saveItems: (items: T[]) => Promise<T[]>;
  renderForm: (item: T | undefined, update: (item: T) => void) => ReactNode;
  routeParams?: URLSearchParams;
}

function capitalize(s: string): string {
  return s.charAt(0).toUpperCase() + s.slice(1);
}

export function TemplateEditorPage<T>({
  descriptor: d,
  items,
  setItems,
  saveItems,
  renderForm,
  routeParams,
}: TemplateEditorPageProps<T>) {
  const [editIndex, setEditIndex] = useState<number>(-1);
  const [searchTerm, setSearchTerm] = useState('');
  const [notifications, setNotifications] = useState<NotificationState[]>([]);
  const notificationIdRef = useRef(0);
  const org = useAssetOrganization(d.preserveAsset as OrganizableAssetId);

  const showNotification = (
    message: string,
    type: 'success' | 'error',
    duration?: number
  ) => {
    const id = notificationIdRef.current++;
    setNotifications((prev) => [...prev, { message, type, id, duration }]);
  };
  const removeNotification = (id: number) => {
    setNotifications((prev) => prev.filter((n) => n.id !== id));
  };

  const lowerTerm = searchTerm.toLowerCase();
  const filtered = items.filter((item) => d.matchesSearch(item, lowerTerm));

  const scrollCardIntoView = (itemId: string) => {
    if (!d.scrollCardIntoView) {
      return;
    }
    scrollAssetTreeItemIntoView(itemId);
  };

  const handleClick = (itemId: string) => {
    const actualIndex = items.findIndex((item) => d.getId(item) === itemId);
    if (actualIndex >= 0) {
      setEditIndex(actualIndex);
    }
  };

  const handleClone = (itemId: string) => {
    const actualIndex = items.findIndex((item) => d.getId(item) === itemId);
    if (actualIndex < 0) {
      return;
    }
    const cloned: T = JSON.parse(JSON.stringify(items[actualIndex]));
    d.setId(cloned, d.getId(cloned) + '_copy');
    org.copyPlacement(itemId, d.getId(cloned));
    const next = items.slice();
    const clonedIndex = actualIndex + 1;
    next.splice(clonedIndex, 0, cloned);
    setItems(next);
    setEditIndex(clonedIndex);
    showNotification(`${capitalize(d.entityNoun)} cloned!`, 'success');
    scrollCardIntoView(d.getId(cloned));
  };

  const handleDelete = (itemId: string) => {
    const actualIndex = items.findIndex((item) => d.getId(item) === itemId);
    if (actualIndex < 0) {
      return;
    }
    if (
      confirm(d.deleteConfirmMessage ?? `Delete this ${d.entityNoun}?`)
    ) {
      org.removeItem(itemId);
      const next = items.filter((_, i) => i !== actualIndex);
      setItems(next);
      if (editIndex === actualIndex) {
        setEditIndex(-1);
      } else if (editIndex > actualIndex) {
        setEditIndex(editIndex - 1);
      }
    }
  };

  const handleCreateNew = () => {
    const created = d.createDefault();
    const selectedId = editIndex >= 0 ? d.getId(items[editIndex]) : null;
    org.fileNewItem(d.getId(created), selectedId);
    const next = [...items, created];
    setItems(next);
    setEditIndex(next.length - 1);
    setSearchTerm('');
    scrollCardIntoView(d.getId(created));
  };

  const updateItem = (item: T) => {
    if (editIndex < 0) {
      return;
    }
    org.rekeyItem(d.getId(items[editIndex]), d.getId(item));
    const updated = [...items];
    updated[editIndex] = item;
    setItems(updated);
  };

  const handleSaveAll = async () => {
    const currentId =
      editIndex >= 0 ? d.getId(items[editIndex]) : undefined;
    const compare =
      d.compare ?? ((a: T, b: T) => d.getId(a).localeCompare(d.getId(b)));
    const sorted = trimStrings(items).sort(compare);
    try {
      const saved = await saveItems(sorted);
      setItems(saved);
      const errors = d.validateAfterSave?.(saved) ?? [];
      if (errors.length > 0) {
        showNotification(
          `${capitalize(d.entityNounPlural)} saved, but errors:\n${errors.join(
            '\n'
          )}`,
          'error',
          d.afterSaveErrorDuration
        );
      } else {
        showNotification(
          `${capitalize(d.entityNounPlural)} saved successfully!`,
          'success'
        );
      }
      if (currentId) {
        setEditIndex(saved.findIndex((x) => d.getId(x) === currentId.trim()));
      }
    } catch (error) {
      showNotification(
        error instanceof Error
          ? error.message
          : `Failed to save ${d.entityNounPlural}`,
        'error'
      );
    }
  };

  usePersistedEditorSelection({
    editorKey: d.editorKey,
    items,
    getId: d.getId,
    selectedIndex: editIndex,
    setSelectedIndex: setEditIndex,
    routeParams,
  });

  useSaveHotkey(handleSaveAll);

  const current = editIndex >= 0 ? items[editIndex] : undefined;

  const toCardItem =
    d.toCardItem ??
    ((item: T): CardItem => ({
      name: d.getId(item),
      label: d.getLabel(item) || d.getId(item),
    }));

  const form = renderForm(current, updateItem);

  return (
    <div className="container editor-page">
      <EditorHeader
        title={d.title}
        onSave={handleSaveAll}
        preserveAsset={d.preserveAsset}
      />

      <div className="editor-page-body">
        <div className="editor-content">
          <EditorSidebar
            searchTerm={searchTerm}
            onSearchChange={setSearchTerm}
            searchPlaceholder={d.searchPlaceholder}
            createLabel={d.createLabel}
            onCreate={handleCreateNew}
          >
            <AssetTree
              assetId={d.preserveAsset as OrganizableAssetId}
              items={filtered.map((item) => {
                const card = toCardItem(item);
                const label =
                  typeof card.label === 'string' && card.label
                    ? card.label
                    : card.name;
                return {
                  id: d.getId(item),
                  label,
                  subtitle:
                    typeof card.subtitle === 'string' ? card.subtitle : card.name,
                  extra: d.renderAdditionalInfo?.(card),
                };
              })}
              allItemIds={items.map((item) => d.getId(item))}
              selectedId={editIndex >= 0 ? d.getId(items[editIndex]) : null}
              onSelectItem={handleClick}
              onClone={handleClone}
              onDelete={handleDelete}
              filterActive={searchTerm.trim() !== ''}
              emptyMessage={d.emptyMessage}
            />
          </EditorSidebar>

          <div className="editor-main">
            {d.formWrapperId ? <div id={d.formWrapperId}>{form}</div> : form}
          </div>
        </div>
      </div>

      {notifications.map((n) => (
        <Notification
          key={n.id}
          message={n.message}
          type={n.type}
          duration={n.duration}
          onClose={() => removeNotification(n.id)}
        />
      ))}
    </div>
  );
}
