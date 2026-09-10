import type { ReactNode, UIEvent } from 'react';
import { GenericModal } from './GenericModal';

interface MediaPickerModalProps {
  title: string;
  searchTerm: string;
  searchPlaceholder: string;
  onSearchTermChange: (value: string) => void;
  selectedName: string;
  legacyValue?: string;
  resultCount: number;
  visibleCount: number;
  hasMore: boolean;
  onShowMore: () => void;
  onConfirm: () => void;
  onCancel: () => void;
  sidebar?: ReactNode;
  children: ReactNode;
}

export function MediaPickerModal({
  title,
  searchTerm,
  searchPlaceholder,
  onSearchTermChange,
  selectedName,
  legacyValue,
  resultCount,
  visibleCount,
  hasMore,
  onShowMore,
  onConfirm,
  onCancel,
  sidebar,
  children,
}: MediaPickerModalProps) {
  const handleGridScroll = (event: UIEvent<HTMLDivElement>) => {
    const grid = event.currentTarget;
    if (
      hasMore &&
      grid.scrollHeight - grid.scrollTop - grid.clientHeight < 160
    ) {
      onShowMore();
    }
  };

  return (
    <GenericModal
      title={title}
      fillBody
      maxWidth="960px"
      confirmLabel="Use selection"
      confirmDisabled={!selectedName}
      onConfirm={onConfirm}
      onCancel={onCancel}
      body={() => (
        <div className="media-picker">
          <div className="media-picker-filters">
            <input
              type="search"
              value={searchTerm}
              onChange={(event) => onSearchTermChange(event.target.value)}
              placeholder={searchPlaceholder}
              aria-label={searchPlaceholder}
              autoFocus
            />
            <span className="media-picker-count" role="status">
              {resultCount === visibleCount
                ? `${resultCount} result${resultCount === 1 ? '' : 's'}`
                : `Showing ${visibleCount} of ${resultCount}`}
            </span>
          </div>
          {legacyValue ? (
            <p className="media-picker-legacy">
              “{legacyValue}” is not currently loaded. It will be retained until
              another value is selected.
            </p>
          ) : null}
          <div
            className={`media-picker-layout ${sidebar ? '' : 'media-picker-layout-full'}`.trim()}
          >
            {sidebar ? (
              <aside className="media-picker-sidebar">{sidebar}</aside>
            ) : null}
            <div className="media-picker-grid" onScroll={handleGridScroll}>
              {children}
              {hasMore ? (
                <button
                  type="button"
                  className="btn-secondary media-picker-more"
                  onClick={onShowMore}
                >
                  Show more
                </button>
              ) : null}
            </div>
          </div>
        </div>
      )}
    />
  );
}
