import { CharacterTemplate } from '../../types/assets';
import {
  SearchInput,
  SearchInputDropdownPlacement,
} from '../../elements/SearchInput';

export function CharacterSearchInput({
  characters,
  onSelect,
  placeholder = 'Search characters...',
  dropdownPlacement,
  onEnter,
  onSearchTermChange,
}: {
  characters: CharacterTemplate[];
  onSelect: (characterName: string) => void;
  placeholder?: string;
  dropdownPlacement?: SearchInputDropdownPlacement;
  onEnter?: (characterName: string) => void;
  onSearchTermChange?: (term: string) => void;
}) {
  return (
    <SearchInput
      items={characters}
      placeholder={placeholder}
      dropdownPlacement={dropdownPlacement}
      onEnter={onEnter}
      onSearchTermChange={onSearchTermChange}
      searchFields={(character) => [
        character.name,
        character.label,
        character.type,
      ]}
      getItemKey={(character) => character.name}
      onSelect={(character) => onSelect(character.name)}
      renderItem={(character) => (
        <>
          <div style={{ fontWeight: 'bold' }}>{character.label}</div>
          <div
            style={{
              fontSize: '10px',
              color: '#858585',
              marginTop: '2px',
            }}
          >
            {character.name} • {character.type}
          </div>
        </>
      )}
    />
  );
}
