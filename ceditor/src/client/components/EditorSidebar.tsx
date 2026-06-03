import { Button } from '../elements/Button';

export interface EditorSidebarProps {
  children: React.ReactNode;
  searchTerm?: string;
  onSearchChange?: (value: string) => void;
  searchPlaceholder?: string;
  createLabel?: string;
  onCreate?: () => void;
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
  createFirst = false,
  afterCreate,
  afterSearch,
}: EditorSidebarProps) {
  const showSearch = onSearchChange !== undefined;
  const showCreate = onCreate !== undefined && createLabel !== undefined;

  const createButton = showCreate ? (
    <div className="item-actions">
      <Button variant="primary" onClick={onCreate}>
        {createLabel}
      </Button>
    </div>
  ) : null;

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
          {createButton}
          {afterCreate}
          {searchBox}
          {afterSearch}
        </>
      ) : (
        <>
          {searchBox}
          {createButton}
          {afterSearch}
        </>
      )}
      {children}
    </div>
  );
}
