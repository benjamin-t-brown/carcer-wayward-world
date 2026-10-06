import { DropTable } from '../../../types/assets';
import {
  SearchInput,
  SearchInputDropdownPlacement,
} from '../../../elements/SearchInput';

export function DropTableSearchInput({
  dropTables,
  onSelect,
  placeholder = 'Search drop tables...',
  dropdownPlacement,
}: {
  dropTables: DropTable[];
  onSelect: (dropTableName: string) => void;
  placeholder?: string;
  dropdownPlacement?: SearchInputDropdownPlacement;
}) {
  return (
    <SearchInput
      items={dropTables}
      placeholder={placeholder}
      dropdownPlacement={dropdownPlacement}
      searchFields={(table) => [table.name, table.label]}
      getItemKey={(table) => table.name}
      onSelect={(table) => onSelect(table.name)}
      renderItem={(table) => (
        <>
          <div style={{ fontWeight: 'bold' }}>{table.label || table.name}</div>
          <div
            style={{
              fontSize: '10px',
              color: '#858585',
              marginTop: '2px',
            }}
          >
            {table.name}
          </div>
        </>
      )}
    />
  );
}
