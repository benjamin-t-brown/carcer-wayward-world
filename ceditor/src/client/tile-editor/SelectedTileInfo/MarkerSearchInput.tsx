import {
  SearchInput,
  SearchInputDropdownPlacement,
} from '../../elements/SearchInput';

export function MarkerSearchInput({
  markerNames,
  onSelect,
  placeholder = 'Search markers on map...',
  dropdownPlacement,
  onEnter,
  onSearchTermChange,
}: {
  markerNames: string[];
  onSelect: (markerName: string) => void;
  placeholder?: string;
  dropdownPlacement?: SearchInputDropdownPlacement;
  onEnter?: (markerName: string) => void;
  onSearchTermChange?: (term: string) => void;
}) {
  return (
    <SearchInput
      items={markerNames}
      placeholder={placeholder}
      dropdownPlacement={dropdownPlacement}
      onEnter={onEnter}
      onSearchTermChange={onSearchTermChange}
      searchFields={(name) => [name]}
      getItemKey={(name) => name}
      onSelect={(name) => onSelect(name)}
      renderItem={(name) => (
        <div
          style={{
            fontFamily: 'monospace',
            fontSize: '12px',
          }}
        >
          {name}
        </div>
      )}
    />
  );
}
