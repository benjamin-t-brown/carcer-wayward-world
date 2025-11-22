import { useState, useRef, useEffect, useMemo } from 'react';
import { CharacterTemplate } from '../../../types/assets';

export function CharacterSearchInput({
  characters,
  onSelect,
  placeholder = 'Search characters...',
}: {
  characters: CharacterTemplate[];
  onSelect: (characterName: string) => void;
  placeholder?: string;
}) {
  const [searchTerm, setSearchTerm] = useState('');
  const [isOpen, setIsOpen] = useState(false);
  const inputRef = useRef<HTMLInputElement>(null);
  const dropdownRef = useRef<HTMLDivElement>(null);

  const filteredCharacters = useMemo(() => {
    if (!searchTerm.trim()) return characters;
    const term = searchTerm.toLowerCase();
    return characters.filter(
      (char) =>
        char.name.toLowerCase().includes(term) ||
        char.label.toLowerCase().includes(term) ||
        char.type.toLowerCase().includes(term)
    );
  }, [characters, searchTerm]);

  useEffect(() => {
    const handleClickOutside = (event: MouseEvent) => {
      if (
        dropdownRef.current &&
        !dropdownRef.current.contains(event.target as Node) &&
        inputRef.current &&
        !inputRef.current.contains(event.target as Node)
      ) {
        setIsOpen(false);
      }
    };

    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  const handleSelect = (characterName: string) => {
    onSelect(characterName);
    setSearchTerm('');
    setIsOpen(false);
  };

  return (
    <div style={{ position: 'relative', width: '100%' }}>
      <input
        ref={inputRef}
        type="text"
        value={searchTerm}
        onChange={(e) => {
          setSearchTerm(e.target.value);
          setIsOpen(true);
        }}
        onFocus={() => setIsOpen(true)}
        placeholder={placeholder}
        style={{
          width: '100%',
          padding: '6px 8px',
          border: '1px solid #3e3e42',
          backgroundColor: '#1e1e1e',
          color: '#ffffff',
          fontSize: '12px',
          borderRadius: '4px',
        }}
      />
      {isOpen && filteredCharacters.length > 0 && (
        <div
          ref={dropdownRef}
          style={{
            position: 'absolute',
            top: '100%',
            left: 0,
            right: 0,
            marginTop: '4px',
            maxHeight: '200px',
            overflowY: 'auto',
            backgroundColor: '#252526',
            border: '1px solid #3e3e42',
            borderRadius: '4px',
            zIndex: 1000,
            boxShadow: '0 4px 8px rgba(0, 0, 0, 0.3)',
          }}
        >
          {filteredCharacters.map((character) => (
            <div
              key={character.name}
              onClick={() => handleSelect(character.name)}
              style={{
                padding: '8px 12px',
                cursor: 'pointer',
                fontSize: '12px',
                color: '#ffffff',
                borderBottom: '1px solid #3e3e42',
                transition: 'background-color 0.2s',
              }}
              onMouseEnter={(e) => {
                e.currentTarget.style.backgroundColor = '#3e3e42';
              }}
              onMouseLeave={(e) => {
                e.currentTarget.style.backgroundColor = 'transparent';
              }}
            >
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
            </div>
          ))}
        </div>
      )}
    </div>
  );
}

