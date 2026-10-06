import { DropTable } from '../types/assets';
import {
  DropTableForm,
  createDefaultDropTable,
} from '../components/DropTableForm';
import { useAssets } from '../contexts/AssetsContext';
import {
  TemplateEditorPage,
  TemplateEditorDescriptor,
} from './TemplateEditorPage';

export function DropTables({ routeParams }: { routeParams?: URLSearchParams } = {}) {
  const { dropTables, setDropTables, saveDropTables } = useAssets();

  const descriptor: TemplateEditorDescriptor<DropTable> = {
    editorKey: 'dropTables',
    preserveAsset: 'dropTables',
    title: 'Drop Tables Editor',
    entityNoun: 'drop table',
    entityNounPlural: 'drop tables',
    searchPlaceholder: 'Search drop tables...',
    createLabel: '+ New Drop Table',
    emptyMessage: 'No drop tables found',
    deleteConfirmMessage: 'Are you sure you want to delete this drop table?',
    getId: (t) => t.name,
    setId: (t, id) => {
      t.name = id;
    },
    getLabel: (t) => t.label || t.name,
    createDefault: createDefaultDropTable,
    matchesSearch: (t, term) =>
      t.name.toLowerCase().includes(term) ||
      t.label.toLowerCase().includes(term),
    compare: (a, b) => {
      const cmp = a.name.localeCompare(b.name);
      return cmp === 0 ? a.label.localeCompare(b.label) : cmp;
    },
    toCardItem: (t) => ({
      name: t.name,
      label: t.label || t.name,
      entryCount: t.entries?.length ?? 0,
    }),
    renderAdditionalInfo: (item) => (
      <div className="item-info">
        <span className="item-type">{String(item.entryCount ?? 0)} entries</span>
      </div>
    ),
    formWrapperId: 'drop-table-form',
    scrollCardIntoView: true,
    validateAfterSave: (sorted) =>
      sorted
        .filter((t) => !t.name?.trim())
        .map(() => 'Every drop table must have a name'),
  };

  return (
    <TemplateEditorPage
      descriptor={descriptor}
      items={dropTables}
      setItems={setDropTables}
      saveItems={saveDropTables}
      routeParams={routeParams}
      renderForm={(table, update) => (
        <DropTableForm dropTable={table} updateDropTable={update} />
      )}
    />
  );
}
