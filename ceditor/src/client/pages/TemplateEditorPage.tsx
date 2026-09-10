import { useState, type ReactNode } from 'react';
import { CardList } from '../components/CardList';
import { EditorSidebar } from '../components/EditorSidebar';
import { EditorHeader } from '../components/EditorHeader';
import { Notification } from '../elements/Notification';
import { prepareTemplateRecordsForSave } from '../utils/formSavePreparation';
import { usePersistedEditorSelection } from '../hooks/usePersistedEditorSelection';
import { EditorSelectionKey } from '../utils/editorSelectionStorage';
import {
  itemAtSourceIndex,
  recordKeyAtSourceIndex,
  replaceAtSourceIndex,
  sourceIndexFromVisibleIndex,
  visibleIndexFromSourceIndex,
} from '../utils/editorListSelection';
import { useEditorNotifications } from '../hooks/useEditorNotifications';

type CardItem = { name: string; label?: string } & Record<string, unknown>;

/**
 * Everything a plain "list + form" template editor page needs. Pages that add
 * tab state, canvas editors, autosave, cascading deletes, or a create menu
 * (Maps, SpecialEvents, Tilesets, MapGrids, Abilities) are intentionally not
 * built on this.
 */
export interface TemplateEditorDescriptor<T> {
  editorKey: EditorSelectionKey;
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
  /** Optional domain preparation replacing the default trim + compare step. */
  prepareForSave?(items: T[]): T[];
  /** Validation before any preparation or persistence. Return an error to abort. */
  validateBeforeSave?(items: T[]): string | null;
  /** Optional page-specific save failure copy. */
  formatSaveError?(error: unknown): string;

  /** Row passed to CardList. Default: { name: getId, label: getLabel || getId }. */
  toCardItem?(item: T): CardItem;
  /** Typed card slots for media and short metadata, rendered in that order. */
  renderCardMedia?(item: T): ReactNode;
  renderCardMeta?(item: T): ReactNode;
  /** Legacy CardItem extension point retained for existing descriptors. */
  renderAdditionalInfo?(cardItem: CardItem): ReactNode;

  /** Text in the delete confirm(). Default: `Delete this <noun>?`. */
  deleteConfirmMessage?: string;
  /** id attribute on a wrapper div around the form. */
  formWrapperId?: string;
  /** Scroll the affected card into view after clone / create. */
  scrollCardIntoView?: boolean;
  /** Scroll the selected form into view after select / clone / create / restore. */
  scrollFormIntoView?: boolean;

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
  saveItems: (items: T[]) => Promise<void>;
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
  const { notifications, showNotification, removeNotification } =
    useEditorNotifications();

  const lowerTerm = searchTerm.toLowerCase();
  const filtered = items.filter((item) => d.matchesSearch(item, lowerTerm));

  const getActualIndex = (filteredIndex: number) =>
    sourceIndexFromVisibleIndex(items, filtered, filteredIndex);

  const scrollFormIntoView = () => {
    const formWrapperId = d.formWrapperId;
    if (!d.scrollFormIntoView || !formWrapperId) {
      return;
    }
    setTimeout(() => {
      document
        .getElementById(formWrapperId)
        ?.scrollIntoView({ behavior: 'smooth' });
    }, 100);
  };

  const scrollCardIntoView = (index: number) => {
    if (!d.scrollCardIntoView) {
      return;
    }
    setTimeout(() => {
      document
        .getElementById(`item-card-${index}`)
        ?.scrollIntoView({ behavior: 'smooth' });
    }, 100);
  };

  const handleClick = (filteredIndex: number) => {
    setEditIndex(getActualIndex(filteredIndex));
    scrollFormIntoView();
  };

  const handleClone = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    const original = itemAtSourceIndex(items, actualIndex);
    if (original === undefined) {
      return;
    }
    const cloned: T = JSON.parse(JSON.stringify(original));
    d.setId(cloned, d.getId(cloned) + '_copy');
    const next = items.slice();
    const clonedIndex = actualIndex + 1;
    next.splice(clonedIndex, 0, cloned);
    setItems(next);
    setEditIndex(clonedIndex);
    showNotification(`${capitalize(d.entityNoun)} cloned!`, 'success');
    scrollFormIntoView();
    scrollCardIntoView(clonedIndex);
  };

  const handleDelete = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    if (confirm(d.deleteConfirmMessage ?? `Delete this ${d.entityNoun}?`)) {
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
    const next = [...items, d.createDefault()];
    setItems(next);
    setEditIndex(next.length - 1);
    setSearchTerm('');
    scrollFormIntoView();
    scrollCardIntoView(next.length - 1);
  };

  const updateItem = (item: T) => {
    if (editIndex < 0 || editIndex >= items.length) {
      return;
    }
    setItems(replaceAtSourceIndex(items, editIndex, item));
  };

  const handleSaveAll = async () => {
    const validationError = d.validateBeforeSave?.(items);
    if (validationError) {
      showNotification(validationError, 'error');
      return;
    }

    const current = itemAtSourceIndex(items, editIndex);
    const currentId = current ? d.getId(current) : undefined;
    const sorted = d.prepareForSave
      ? d.prepareForSave(items)
      : prepareTemplateRecordsForSave(items, d.getId, d.compare);
    try {
      await saveItems(sorted);
      const errors = d.validateAfterSave?.(sorted) ?? [];
      if (errors.length > 0) {
        showNotification(
          `${capitalize(d.entityNounPlural)} saved, but errors:\n${errors.join(
            '\n',
          )}`,
          'error',
          d.afterSaveErrorDuration,
        );
      } else {
        showNotification(
          `${capitalize(d.entityNounPlural)} saved successfully!`,
          'success',
        );
      }
      if (currentId) {
        setEditIndex(sorted.findIndex((x) => d.getId(x) === currentId.trim()));
      }
    } catch (error) {
      showNotification(
        d.formatSaveError?.(error) ??
          (error instanceof Error
            ? error.message
            : `Failed to save ${d.entityNounPlural}`),
        'error',
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
    onRestored: scrollFormIntoView,
  });

  const current = itemAtSourceIndex(items, editIndex);

  const toCardItem =
    d.toCardItem ??
    ((item: T): CardItem => ({
      name: d.getId(item),
      label: d.getLabel(item) || d.getId(item),
    }));

  const selectedCardIndex = visibleIndexFromSourceIndex(
    items,
    filtered,
    editIndex,
  );

  const renderCardSlot =
    (render: ((item: T) => ReactNode) | undefined) =>
    (_cardItem: CardItem, visibleIndex: number) => {
      const item = filtered[visibleIndex];
      if (item === undefined) {
        return null;
      }
      return render?.(item);
    };

  const renderMedia = d.renderCardMedia
    ? renderCardSlot(d.renderCardMedia)
    : undefined;
  const renderAdditionalInfo = d.renderCardMeta
    ? renderCardSlot(d.renderCardMeta)
    : d.renderAdditionalInfo;

  const form = renderForm(current, updateItem);

  return (
    <div className="container editor-page">
      <EditorHeader title={d.title} onSave={handleSaveAll} />

      <div className="editor-page-body">
        <div className="editor-content">
          <EditorSidebar
            searchTerm={searchTerm}
            onSearchChange={setSearchTerm}
            searchPlaceholder={d.searchPlaceholder}
            createLabel={d.createLabel}
            onCreate={handleCreateNew}
          >
            <CardList
              items={filtered.map(toCardItem)}
              onItemClick={handleClick}
              onClone={handleClone}
              onDelete={handleDelete}
              selectedIndex={selectedCardIndex}
              renderMedia={renderMedia}
              renderAdditionalInfo={renderAdditionalInfo}
              emptyMessage={d.emptyMessage}
              getCardId={(visibleIndex) =>
                `item-card-${getActualIndex(visibleIndex)}`
              }
              getItemKey={(_cardItem, visibleIndex) => {
                const sourceIndex = getActualIndex(visibleIndex);
                return recordKeyAtSourceIndex(items, sourceIndex, d.getId);
              }}
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
