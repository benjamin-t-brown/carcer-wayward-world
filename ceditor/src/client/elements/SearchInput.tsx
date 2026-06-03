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

export type SearchInputDropdownPlacement = 'below' | 'above' | 'auto';

export interface SearchInputProps<T> {
  items: T[];
  onSelect: (item: T) => void;
  placeholder?: string;
  searchFields: (item: T) => string[];
  renderItem: (item: T) => ReactNode;
  getItemKey: (item: T) => string;
  maxHeight?: number;
  /**
   * Where the results list opens relative to the input (default: below).
   * `auto` opens above when the input is at or below `openAboveBelowViewportRatio`
   * of the viewport height.
   */
  dropdownPlacement?: SearchInputDropdownPlacement;
  /** Used when placement is `auto`; default 2/3 — lower third opens upward. */
  openAboveBelowViewportRatio?: number;
  /** Fired when the user presses Enter with the current input text. */
  onEnter?: (term: string) => void;
  /** Keeps parent state in sync with the input value (e.g. for a submit button). */
  onSearchTermChange?: (term: string) => void;
}

function shouldOpenAbove(
  placement: SearchInputDropdownPlacement,
  inputEl: HTMLInputElement | null,
  openAboveBelowViewportRatio: number
): boolean {
  if (placement === 'above') {
    return true;
  }
  if (placement === 'below') {
    return false;
  }
  if (!inputEl) {
    return false;
  }
  const threshold = window.innerHeight * openAboveBelowViewportRatio;
  return inputEl.getBoundingClientRect().top >= threshold;
}

export function SearchInput<T>({
  items,
  onSelect,
  placeholder = 'Search...',
  searchFields,
  renderItem,
  getItemKey,
  maxHeight = 200,
  dropdownPlacement = 'below',
  openAboveBelowViewportRatio = 2 / 3,
  onEnter,
  onSearchTermChange,
}: SearchInputProps<T>) {
  const [searchTerm, setSearchTerm] = useState('');
  const [isOpen, setIsOpen] = useState(false);
  const [opensAbove, setOpensAbove] = useState(false);
  const [anchorRect, setAnchorRect] = useState<DOMRect | null>(null);
  const inputRef = useRef<HTMLInputElement>(null);
  const dropdownRef = useRef<HTMLDivElement>(null);

  const updatePlacement = () => {
    const input = inputRef.current;
    if (input) {
      setAnchorRect(input.getBoundingClientRect());
    }
    setOpensAbove(
      shouldOpenAbove(
        dropdownPlacement,
        input,
        openAboveBelowViewportRatio
      )
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
    onSearchTermChange?.('');
    setIsOpen(false);
  };

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

  const dropdown =
    isOpen && filteredItems.length > 0 && dropdownStyle ? (
      <div ref={dropdownRef} style={dropdownStyle}>
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
    ) : null;

  return (
    <div style={{ position: 'relative', width: '100%' }}>
      <input
        ref={inputRef}
        type="text"
        value={searchTerm}
        onChange={(e) => {
          setSearchTerm(e.target.value);
          onSearchTermChange?.(e.target.value);
          updatePlacement();
          setIsOpen(true);
        }}
        onKeyDown={(e) => {
          if (e.key === 'Enter' && onEnter) {
            e.preventDefault();
            onEnter(searchTerm.trim());
            setIsOpen(false);
          }
        }}
        onFocus={() => {
          updatePlacement();
          setIsOpen(true);
        }}
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
      {dropdown ? createPortal(dropdown, document.body) : null}
    </div>
  );
}
