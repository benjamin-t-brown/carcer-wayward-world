import {
  useState,
  useRef,
  useEffect,
  useLayoutEffect,
  useMemo,
  type CSSProperties,
  type ReactNode,
} from 'react';
import { createPortal } from 'react-dom';
import {
  SearchInputDropdownPlacement,
  shouldOpenAbove,
} from './SearchInput';

export interface SearchSelectProps<T> {
  id?: string;
  name?: string;
  label?: string;
  required?: boolean;
  items: T[];
  value: string;
  onChange: (value: string) => void;
  getItemKey: (item: T) => string;
  getItemLabel: (item: T) => string;
  searchFields: (item: T) => string[];
  renderItem?: (item: T) => ReactNode;
  placeholder?: string;
  emptyLabel?: string;
  allowEmpty?: boolean;
  maxHeight?: number;
  dropdownPlacement?: SearchInputDropdownPlacement;
  openAboveBelowViewportRatio?: number;
  className?: string;
}

export function SearchSelect<T>({
  id,
  name,
  label,
  required = false,
  items,
  value,
  onChange,
  getItemKey,
  getItemLabel,
  searchFields,
  renderItem,
  placeholder = 'Search...',
  emptyLabel = '(none)',
  allowEmpty = true,
  maxHeight = 220,
  dropdownPlacement = 'auto',
  openAboveBelowViewportRatio = 2 / 3,
  className = '',
}: SearchSelectProps<T>) {
  const [searchTerm, setSearchTerm] = useState('');
  const [isOpen, setIsOpen] = useState(false);
  const [opensAbove, setOpensAbove] = useState(false);
  const [anchorRect, setAnchorRect] = useState<DOMRect | null>(null);
  const inputRef = useRef<HTMLInputElement>(null);
  const dropdownRef = useRef<HTMLDivElement>(null);

  const selected = items.find((item) => getItemKey(item) === value);

  const updatePlacement = () => {
    const input = inputRef.current;
    if (input) {
      setAnchorRect(input.getBoundingClientRect());
    }
    setOpensAbove(
      shouldOpenAbove(dropdownPlacement, input, openAboveBelowViewportRatio)
    );
  };

  useLayoutEffect(() => {
    if (!isOpen) {
      setAnchorRect(null);
      return;
    }
    updatePlacement();
    const onReposition = () => updatePlacement();
    window.addEventListener('resize', onReposition);
    window.addEventListener('scroll', onReposition, true);
    return () => {
      window.removeEventListener('resize', onReposition);
      window.removeEventListener('scroll', onReposition, true);
    };
  }, [isOpen, dropdownPlacement, openAboveBelowViewportRatio]);

  const filteredItems = useMemo(() => {
    if (!searchTerm.trim()) {
      return items;
    }
    const term = searchTerm.toLowerCase();
    return items.filter((item) =>
      searchFields(item).some((field) => field.toLowerCase().includes(term))
    );
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
        setSearchTerm('');
      }
    };

    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  const handleSelect = (key: string) => {
    onChange(key);
    setSearchTerm('');
    setIsOpen(false);
  };

  const inputDisplayValue = isOpen
    ? searchTerm
    : selected
      ? `${getItemLabel(selected)} (${getItemKey(selected)})`
      : '';

  const dropdownStyle: CSSProperties | null = anchorRect
    ? {
        position: 'fixed',
        left: anchorRect.left,
        width: anchorRect.width,
        maxHeight: `${maxHeight}px`,
        overflowY: 'auto',
        backgroundColor: '#252526',
        border: '1px solid #3e3e42',
        borderRadius: '4px',
        zIndex: 10000,
        boxShadow: '0 4px 8px rgba(0, 0, 0, 0.3)',
        ...(opensAbove
          ? { bottom: window.innerHeight - anchorRect.top + 4 }
          : { top: anchorRect.bottom + 4 }),
      }
    : null;

  const showDropdown =
    isOpen && dropdownStyle && (allowEmpty || filteredItems.length > 0);

  const dropdown = showDropdown ? (
    <div ref={dropdownRef} style={dropdownStyle}>
      {allowEmpty ? (
        <div
          onClick={() => handleSelect('')}
          style={{
            padding: '8px 12px',
            cursor: 'pointer',
            fontSize: '12px',
            color: '#858585',
            borderBottom: '1px solid #3e3e42',
            fontStyle: 'italic',
          }}
          onMouseEnter={(e) => {
            e.currentTarget.style.backgroundColor = '#3e3e42';
          }}
          onMouseLeave={(e) => {
            e.currentTarget.style.backgroundColor = 'transparent';
          }}
        >
          {emptyLabel}
        </div>
      ) : null}
      {filteredItems.map((item) => (
        <div
          key={getItemKey(item)}
          onClick={() => handleSelect(getItemKey(item))}
          style={{
            padding: '8px 12px',
            cursor: 'pointer',
            fontSize: '12px',
            color: '#ffffff',
            borderBottom: '1px solid #3e3e42',
            transition: 'background-color 0.2s',
            backgroundColor:
              getItemKey(item) === value ? '#094771' : 'transparent',
          }}
          onMouseEnter={(e) => {
            if (getItemKey(item) !== value) {
              e.currentTarget.style.backgroundColor = '#3e3e42';
            }
          }}
          onMouseLeave={(e) => {
            e.currentTarget.style.backgroundColor =
              getItemKey(item) === value ? '#094771' : 'transparent';
          }}
        >
          {renderItem ? (
            renderItem(item)
          ) : (
            <>
              <div style={{ fontWeight: 600 }}>{getItemLabel(item)}</div>
              <div style={{ fontSize: '10px', color: '#858585', marginTop: 2 }}>
                {getItemKey(item)}
              </div>
            </>
          )}
        </div>
      ))}
      {filteredItems.length === 0 && !allowEmpty ? (
        <div
          style={{
            padding: '8px 12px',
            fontSize: '12px',
            color: '#858585',
          }}
        >
          No matches
        </div>
      ) : null}
    </div>
  ) : null;

  const groupClass = `form-group ${className}`.trim();

  return (
    <div className={groupClass}>
      {label ? (
        <label htmlFor={id}>
          {label}
          {required && ' *'}
        </label>
      ) : null}
      {name ? <input type="hidden" name={name} value={value} /> : null}
      <div style={{ position: 'relative', width: '100%' }}>
        <input
          ref={inputRef}
          id={id}
          type="text"
          value={inputDisplayValue}
          onChange={(e) => {
            setSearchTerm(e.target.value);
            updatePlacement();
            setIsOpen(true);
          }}
          onFocus={() => {
            setSearchTerm('');
            updatePlacement();
            setIsOpen(true);
          }}
          placeholder={selected ? undefined : placeholder}
          required={required && !value}
          autoComplete="off"
          className="search-select-input"
          style={{
            width: '100%',
            padding: '6px 8px',
            border: '1px solid #3e3e42',
            backgroundColor: '#1e1e1e',
            color: '#ffffff',
            fontSize: '12px',
            borderRadius: '4px',
            boxSizing: 'border-box',
          }}
        />
        {dropdown ? createPortal(dropdown, document.body) : null}
      </div>
    </div>
  );
}
