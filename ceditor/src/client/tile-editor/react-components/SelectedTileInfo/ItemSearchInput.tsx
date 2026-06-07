import { ItemTemplate } from '../../../types/assets';
import {
  SearchInput,
  SearchInputDropdownPlacement,
} from '../../../elements/SearchInput';

export function ItemSearchInput({
  items,
  onSelect,
  placeholder = 'Search items...',
  dropdownPlacement,
}: {
  items: ItemTemplate[];
  onSelect: (itemName: string) => void;
  placeholder?: string;
  dropdownPlacement?: SearchInputDropdownPlacement;
}) {
  return (
    <SearchInput
      items={items}
      placeholder={placeholder}
      dropdownPlacement={dropdownPlacement}
      searchFields={(item) => [item.name, item.label, item.itemType]}
      getItemKey={(item) => item.name}
      onSelect={(item) => onSelect(item.name)}
      renderItem={(item) => (
        <>
          <div style={{ fontWeight: 'bold' }}>{item.label}</div>
          <div
            style={{
              fontSize: '10px',
              color: '#858585',
              marginTop: '2px',
            }}
          >
            {item.name} • {item.itemType}
          </div>
        </>
      )}
    />
  );
}
