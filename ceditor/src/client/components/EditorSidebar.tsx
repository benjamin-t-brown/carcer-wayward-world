import { useEffect, useRef, useState } from 'react';
import { Button } from '../elements/Button';

export interface EditorSidebarCreateMenuItem {
  label: string;
  onClick: () => void;
}

export interface EditorSidebarProps {
  children: React.ReactNode;
  searchTerm?: string;
  onSearchChange?: (value: string) => void;
  searchPlaceholder?: string;
  createLabel?: string;
  onCreate?: () => void;
  /** When set, shows a dropdown of create actions instead of a single button. */
  createMenuLabel?: string;
  createMenuItems?: EditorSidebarCreateMenuItem[];
  /** Place create button before search (e.g. Special Events). Default: search then create. */
  createFirst?: boolean;
  /** Content between create button and search when createFirst is set. */
  afterCreate?: React.ReactNode;
  /** Content between search and the card list (e.g. type filters). */
  afterSearch?: React.ReactNode;
}

export function EditorSidebar({
  children,
  searchTerm = '',
  onSearchChange,
  searchPlaceholder = 'Search...',
  createLabel,
  onCreate,
  createMenuLabel = '+ New',
  createMenuItems,
  createFirst = false,
  afterCreate,
  afterSearch,
}: EditorSidebarProps) {
  const showSearch = onSearchChange !== undefined;
  const showCreateMenu =
    createMenuItems !== undefined && createMenuItems.length > 0;
  const showCreate =
    !showCreateMenu && onCreate !== undefined && createLabel !== undefined;

  const [menuOpen, setMenuOpen] = useState(false);
  const menuRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    if (!menuOpen) {
      return;
    }
    const onPointerDown = (event: MouseEvent) => {
      if (
        menuRef.current &&
        !menuRef.current.contains(event.target as Node)
      ) {
        setMenuOpen(false);
      }
    };
    const onKeyDown = (event: KeyboardEvent) => {
      if (event.key === 'Escape') {
        setMenuOpen(false);
      }
    };
    document.addEventListener('mousedown', onPointerDown);
    document.addEventListener('keydown', onKeyDown);
    return () => {
      document.removeEventListener('mousedown', onPointerDown);
      document.removeEventListener('keydown', onKeyDown);
    };
  }, [menuOpen]);

  const createButton = showCreate ? (
    <div className="item-actions">
      <Button variant="primary" onClick={onCreate}>
        {createLabel}
      </Button>
    </div>
  ) : null;

  const createMenu = showCreateMenu ? (
    <div className="item-actions editor-create-menu" ref={menuRef}>
      <Button
        variant="primary"
        onClick={() => setMenuOpen((open) => !open)}
        ariaLabel={createMenuLabel}
      >
        {createMenuLabel} ▾
      </Button>
      {menuOpen ? (
        <div className="editor-create-menu-dropdown" role="menu">
          {createMenuItems.map((item) => (
            <button
              key={item.label}
              type="button"
              className="editor-create-menu-item"
              role="menuitem"
              onClick={() => {
                setMenuOpen(false);
                item.onClick();
              }}
            >
              {item.label}
            </button>
          ))}
        </div>
      ) : null}
    </div>
  ) : null;

  const createControl = createMenu ?? createButton;

  const searchBox = showSearch ? (
    <div className="search-box">
      <input
        type="text"
        value={searchTerm}
        onChange={(e) => onSearchChange(e.target.value)}
        placeholder={searchPlaceholder}
      />
    </div>
  ) : null;

  return (
    <div className="editor-sidebar">
      {createFirst ? (
        <>
          {createControl}
          {afterCreate}
          {searchBox}
          {afterSearch}
        </>
      ) : (
        <>
          {searchBox}
          {createControl}
          {afterSearch}
        </>
      )}
      {children}
    </div>
  );
}
