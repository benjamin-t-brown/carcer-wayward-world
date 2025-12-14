import { useState, useRef, useEffect, useMemo } from 'react';

export interface SearchInputProps<T> {
  items: T[];
  onSelect: (item: T) => void;
  placeholder?: string;
  searchFields: (item: T) => string[];
  renderItem: (item: T) => React.ReactNode;
  getItemKey: (item: T) => string;
  maxHeight?: number;
}

export function SearchInput<T>({
  items,
  onSelect,
  placeholder = 'Search...',
  searchFields,
  renderItem,
  getItemKey,
  maxHeight = 200,
}: SearchInputProps<T>) {
  const [searchTerm, setSearchTerm] = useState('');
  const [isOpen, setIsOpen] = useState(false);
  const inputRef = useRef<HTMLInputElement>(null);
  const dropdownRef = useRef<HTMLDivElement>(null);

  const filteredItems = useMemo(() => {
    if (!searchTerm.trim()) return items;
    const term = searchTerm.toLowerCase();
    return items.filter((item) => {
      const fields = searchFields(item);
      return fields.some((field) => field.toLowerCase().includes(term));
    });
  }, [items, searchTerm, searchFields]);

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

  const handleSelect = (item: T) => {
    onSelect(item);
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
      {isOpen && filteredItems.length > 0 && (
        <div
          ref={dropdownRef}
          style={{
            position: 'absolute',
            top: '100%',
            left: 0,
            right: 0,
            marginTop: '4px',
            maxHeight: `${maxHeight}px`,
            overflowY: 'auto',
            backgroundColor: '#252526',
            border: '1px solid #3e3e42',
            borderRadius: '4px',
            zIndex: 1000,
            boxShadow: '0 4px 8px rgba(0, 0, 0, 0.3)',
          }}
        >
          {filteredItems.map((item) => (
            <div
              key={getItemKey(item)}
              onClick={() => handleSelect(item)}
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
              {renderItem(item)}
            </div>
          ))}
        </div>
      )}
    </div>
  );
}

