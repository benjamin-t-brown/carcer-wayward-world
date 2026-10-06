import { useMemo } from 'react';
import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { OptionSelect } from '../elements/OptionSelect';
import { SearchSelect } from '../elements/SearchSelect';
import { Button } from '../elements/Button';
import { useAssets } from '../contexts/AssetsContext';
import { EditorEmptyState } from './EditorEmptyState';
import {
  DropTable,
  DropTableEntry,
  createDefaultDropTable,
  createDefaultDropTableEntry,
} from '../types/assets';

export type { DropTable };
export { createDefaultDropTable };

type EntryKind = 'item' | 'dropTable' | 'gold' | 'food' | 'nothing';

interface DropTableFormProps {
  dropTable?: DropTable;
  updateDropTable: (dropTable: DropTable) => void;
}

function entryKind(entry: DropTableEntry): EntryKind {
  if (entry.nothing === true) {
    return 'nothing';
  }
  if (entry.dropTable?.trim()) {
    return 'dropTable';
  }
  if (entry.goldMin != null || entry.goldMax != null) {
    return 'gold';
  }
  if (entry.foodMin != null || entry.foodMax != null) {
    return 'food';
  }
  return 'item';
}

function effectiveWeight(entry: DropTableEntry): number {
  const weight = entry.weight ?? 1;
  return weight > 0 ? weight : 1;
}

function formatChancePercent(weight: number, totalWeight: number): string {
  if (totalWeight <= 0) {
    return '0%';
  }
  const percent = (weight / totalWeight) * 100;
  if (Number.isInteger(percent)) {
    return `${percent}%`;
  }
  return `${percent.toFixed(1)}%`;
}

function nonNegativeInt(value: number): number {
  return Math.max(0, Math.round(value || 0));
}

export function DropTableForm(props: DropTableFormProps) {
  const dropTable = props.dropTable;
  const { items, dropTables } = useAssets();

  const nestedDropTables = useMemo(
    () =>
      dropTables.filter(
        (table) => table.name && table.name !== dropTable?.name,
      ),
    [dropTables, dropTable?.name],
  );

  const totalWeight = useMemo(
    () =>
      (dropTable?.entries ?? []).reduce(
        (sum, entry) => sum + effectiveWeight(entry),
        0,
      ),
    [dropTable?.entries],
  );

  if (!dropTable) {
    return <EditorEmptyState message="Select a drop table to edit" />;
  }

  const setFormData = (data: DropTable) => {
    props.updateDropTable(data);
  };

  const updateField = <K extends keyof DropTable>(
    field: K,
    value: DropTable[K],
  ) => {
    setFormData({ ...dropTable, [field]: value });
  };

  const updateEntry = (index: number, entry: DropTableEntry) => {
    const entries = [...(dropTable.entries || [])];
    entries[index] = entry;
    setFormData({ ...dropTable, entries });
  };

  const setEntryKind = (index: number, kind: EntryKind) => {
    const current = dropTable.entries?.[index] ?? createDefaultDropTableEntry();
    const weight = current.weight ?? 1;
    if (kind === 'item') {
      updateEntry(index, { item: current.item ?? '', weight });
    } else if (kind === 'dropTable') {
      updateEntry(index, { dropTable: current.dropTable ?? '', weight });
    } else if (kind === 'gold') {
      updateEntry(index, {
        goldMin: current.goldMin ?? 0,
        goldMax: current.goldMax ?? 0,
        weight,
      });
    } else if (kind === 'food') {
      updateEntry(index, {
        foodMin: current.foodMin ?? 0,
        foodMax: current.foodMax ?? 0,
        weight,
      });
    } else {
      updateEntry(index, { nothing: true, weight });
    }
  };

  const addEntry = () => {
    setFormData({
      ...dropTable,
      entries: [...(dropTable.entries || []), createDefaultDropTableEntry()],
    });
  };

  const removeEntry = (index: number) => {
    setFormData({
      ...dropTable,
      entries: dropTable.entries?.filter((_, i) => i !== index) ?? [],
    });
  };

  return (
    <div className="item-form drop-table-form">
      <h2>Edit Drop Table</h2>
      <form>
        <div className="form-fields-inline">
          <TextInput
            id="drop-table-name"
            name="name"
            label="Name"
            value={dropTable.name}
            onChange={(value) => updateField('name', value)}
            required
          />
          <TextInput
            id="drop-table-label"
            name="label"
            label="Label"
            value={dropTable.label}
            onChange={(value) => updateField('label', value)}
            required
          />
        </div>

        <div className="form-subsection">
          <h4>Entries</h4>
          {dropTable.entries?.map((entry, index) => {
            const kind = entryKind(entry);
            const weight = effectiveWeight(entry);
            const chanceLabel = formatChancePercent(weight, totalWeight);
            return (
              <div key={index} className="status-item drop-table-entry-row">
                <OptionSelect
                  id={`drop-entry-kind-${index}`}
                  name={`entryKind${index}`}
                  label="Type"
                  className="drop-table-entry-kind"
                  value={kind}
                  onChange={(value) => setEntryKind(index, value as EntryKind)}
                  options={[
                    { value: 'item', label: 'Item' },
                    { value: 'dropTable', label: 'Table' },
                    { value: 'gold', label: 'Gold' },
                    { value: 'food', label: 'Food' },
                    { value: 'nothing', label: 'Nothing' },
                  ]}
                />
                <div className="drop-table-entry-main">
                  {kind === 'item' ? (
                    <SearchSelect
                      id={`drop-entry-item-${index}`}
                      name={`entryItem${index}`}
                      label="Item"
                      className="drop-table-entry-target"
                      value={entry.item ?? ''}
                      onChange={(value) =>
                        updateEntry(index, {
                          item: value,
                          weight: entry.weight ?? 1,
                        })
                      }
                      items={items}
                      getItemKey={(item) => item.name}
                      getItemLabel={(item) => item.label?.trim() || item.name}
                      searchFields={(item) => [
                        item.name,
                        item.label ?? '',
                        item.itemType ?? '',
                      ]}
                      placeholder="Search items..."
                      emptyLabel="(select item)"
                      allowEmpty
                    />
                  ) : kind === 'dropTable' ? (
                    <SearchSelect
                      id={`drop-entry-table-${index}`}
                      name={`entryDropTable${index}`}
                      label="Table"
                      className="drop-table-entry-target"
                      value={entry.dropTable ?? ''}
                      onChange={(value) =>
                        updateEntry(index, {
                          dropTable: value,
                          weight: entry.weight ?? 1,
                        })
                      }
                      items={nestedDropTables}
                      getItemKey={(table) => table.name}
                      getItemLabel={(table) => table.label?.trim() || table.name}
                      searchFields={(table) => [table.name, table.label ?? '']}
                      placeholder="Search drop tables..."
                      emptyLabel="(select table)"
                      allowEmpty
                    />
                  ) : kind === 'gold' ? (
                    <div className="drop-table-entry-range">
                      <NumberInput
                        id={`drop-entry-gold-min-${index}`}
                        name={`entryGoldMin${index}`}
                        label="Min gold"
                        value={entry.goldMin ?? 0}
                        onChange={(value) =>
                          updateEntry(index, {
                            goldMin: nonNegativeInt(value),
                            goldMax: entry.goldMax ?? 0,
                            weight: entry.weight ?? 1,
                          })
                        }
                        min={0}
                      />
                      <NumberInput
                        id={`drop-entry-gold-max-${index}`}
                        name={`entryGoldMax${index}`}
                        label="Max gold"
                        value={entry.goldMax ?? 0}
                        onChange={(value) =>
                          updateEntry(index, {
                            goldMin: entry.goldMin ?? 0,
                            goldMax: nonNegativeInt(value),
                            weight: entry.weight ?? 1,
                          })
                        }
                        min={0}
                      />
                    </div>
                  ) : kind === 'food' ? (
                    <div className="drop-table-entry-range">
                      <NumberInput
                        id={`drop-entry-food-min-${index}`}
                        name={`entryFoodMin${index}`}
                        label="Min food"
                        value={entry.foodMin ?? 0}
                        onChange={(value) =>
                          updateEntry(index, {
                            foodMin: nonNegativeInt(value),
                            foodMax: entry.foodMax ?? 0,
                            weight: entry.weight ?? 1,
                          })
                        }
                        min={0}
                      />
                      <NumberInput
                        id={`drop-entry-food-max-${index}`}
                        name={`entryFoodMax${index}`}
                        label="Max food"
                        value={entry.foodMax ?? 0}
                        onChange={(value) =>
                          updateEntry(index, {
                            foodMin: entry.foodMin ?? 0,
                            foodMax: nonNegativeInt(value),
                            weight: entry.weight ?? 1,
                          })
                        }
                        min={0}
                      />
                    </div>
                  ) : (
                    <div className="drop-table-entry-nothing">No drop</div>
                  )}
                </div>
                <div className="drop-table-entry-trailing">
                  <NumberInput
                    id={`drop-entry-weight-${index}`}
                    name={`entryWeight${index}`}
                    label="Weight"
                    value={entry.weight ?? 1}
                    onChange={(value) => {
                      const nextWeight = Math.max(1, Math.round(value || 1));
                      if (kind === 'gold') {
                        updateEntry(index, {
                          goldMin: entry.goldMin ?? 0,
                          goldMax: entry.goldMax ?? 0,
                          weight: nextWeight,
                        });
                      } else if (kind === 'food') {
                        updateEntry(index, {
                          foodMin: entry.foodMin ?? 0,
                          foodMax: entry.foodMax ?? 0,
                          weight: nextWeight,
                        });
                      } else if (kind === 'dropTable') {
                        updateEntry(index, {
                          dropTable: entry.dropTable ?? '',
                          weight: nextWeight,
                        });
                      } else if (kind === 'nothing') {
                        updateEntry(index, {
                          nothing: true,
                          weight: nextWeight,
                        });
                      } else {
                        updateEntry(index, {
                          item: entry.item ?? '',
                          weight: nextWeight,
                        });
                      }
                    }}
                    min={1}
                  />
                  <div
                    className="drop-table-entry-chance"
                    title="Chance this entry is selected when this table is rolled"
                  >
                    {chanceLabel}
                  </div>
                  <div className="drop-table-entry-remove">
                    <Button
                      type="button"
                      variant="small"
                      className="btn-danger"
                      onClick={() => removeEntry(index)}
                    >
                      Remove
                    </Button>
                  </div>
                </div>
              </div>
            );
          })}
          <Button type="button" variant="secondary" onClick={addEntry}>
            + Add Entry
          </Button>
        </div>
      </form>
    </div>
  );
}
