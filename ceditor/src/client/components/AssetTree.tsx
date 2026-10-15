import { useEffect, useRef, useState, type DragEvent, type ReactNode } from 'react';
import { Button } from '../elements/Button';
import { ListCardActions } from '../elements/ListCardActions';
import { useAssetOrganization } from '../contexts/OrganizationContext';
import {
  canMoveFolder,
  findFolder,
  resolvedItemFolder,
  revealFolderIds,
  type FolderNode,
  type OrganizableAssetId,
} from '../../shared/editorOrganization';

export interface AssetTreeItem {
  id: string;
  label: string;
  subtitle?: string;
  extra?: ReactNode;
  /** Small mark in the file-icon column, left of the label. */
  icon?: ReactNode;
}

export interface AssetTreeProps {
  assetId: OrganizableAssetId;
  /** Records that pass the current search and type filters. */
  items: AssetTreeItem[];
  /** Every record id, including ones hidden by the filter. */
  allItemIds: readonly string[];
  selectedId?: string | null;
  onSelectItem: (id: string) => void;
  onClone: (id: string) => void;
  onDelete: (id: string) => void;
  filterActive?: boolean;
  emptyMessage?: string;
  /** Replaces the default label, subtitle, extra, and actions body. */
  renderItem?: (item: AssetTreeItem) => ReactNode;
  /** Item order inside each folder. Default: label, then id. */
  compareItems?: (a: AssetTreeItem, b: AssetTreeItem) => number;
}

interface DragPayload {
  assetId: string;
  kind: 'item' | 'folder';
  id: string;
}

let activeDrag: DragPayload | null = null;

export function assetTreeItemElementId(itemId: string): string {
  return `asset-tree-item-${encodeURIComponent(itemId)}`;
}

export function scrollAssetTreeItemIntoView(itemId: string) {
  window.setTimeout(() => {
    document
      .getElementById(assetTreeItemElementId(itemId))
      ?.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
  }, 100);
}

function expandedStorageKey(assetId: string): string {
  return `ceditor.assetTree.expanded.${assetId}`;
}

function loadExpanded(assetId: string): Set<string> {
  try {
    const raw = localStorage.getItem(expandedStorageKey(assetId));
    if (!raw) {
      return new Set();
    }
    const parsed = JSON.parse(raw) as unknown;
    if (!Array.isArray(parsed)) {
      return new Set();
    }
    return new Set(parsed.filter((id): id is string => typeof id === 'string'));
  } catch {
    return new Set();
  }
}

function saveExpanded(assetId: string, expanded: Set<string>) {
  localStorage.setItem(
    expandedStorageKey(assetId),
    JSON.stringify([...expanded].sort()),
  );
}

function byName(a: FolderNode, b: FolderNode): number {
  return a.name.localeCompare(b.name) || a.id.localeCompare(b.id);
}

function byLabel(a: AssetTreeItem, b: AssetTreeItem): number {
  return a.label.localeCompare(b.label) || a.id.localeCompare(b.id);
}

function stopRowControl(event: { stopPropagation: () => void }) {
  event.stopPropagation();
}

export function AssetTree({
  assetId,
  items,
  allItemIds,
  selectedId = null,
  onSelectItem,
  onClone,
  onDelete,
  filterActive = false,
  emptyMessage = 'No items found',
  renderItem,
  compareItems = byLabel,
}: AssetTreeProps) {
  const {
    organization,
    selectedFolderId,
    selectFolder,
    createFolder,
    renameFolder,
    deleteFolder,
    moveItem,
    moveFolder,
    pruneMissing,
    filingFolderId,
  } = useAssetOrganization(assetId);
  const [expanded, setExpanded] = useState<Set<string>>(() => loadExpanded(assetId));
  const [renamingId, setRenamingId] = useState<string | null>(null);
  const [dropTarget, setDropTarget] = useState<string | null>(null);
  const allItemIdsRef = useRef(allItemIds);
  allItemIdsRef.current = allItemIds;
  const liveKey = allItemIds.join('\0');

  useEffect(() => {
    setExpanded(loadExpanded(assetId));
    setRenamingId(null);
  }, [assetId]);

  useEffect(() => {
    pruneMissing(allItemIdsRef.current);
  }, [pruneMissing, liveKey]);

  useEffect(() => {
    const onKeyDown = (event: KeyboardEvent) => {
      if (event.key !== 'F2' || event.defaultPrevented) {
        return;
      }
      if (renamingId) {
        return;
      }
      const target = event.target;
      if (
        target instanceof HTMLElement &&
        (target.tagName === 'INPUT' ||
          target.tagName === 'TEXTAREA' ||
          target.isContentEditable)
      ) {
        return;
      }
      if (
        !selectedFolderId ||
        !findFolder(organization.folders, selectedFolderId)
      ) {
        return;
      }
      event.preventDefault();
      setRenamingId(selectedFolderId);
    };
    window.addEventListener('keydown', onKeyDown);
    return () => window.removeEventListener('keydown', onKeyDown);
  }, [organization.folders, renamingId, selectedFolderId]);

  const reveal = filterActive
    ? revealFolderIds(
        organization,
        items.map((item) => item.id),
      )
    : null;

  const expandIds = (ids: Array<string | null>) => {
    setExpanded((current) => {
      const next = new Set(current);
      let changed = false;
      for (const id of ids) {
        if (id && !next.has(id)) {
          next.add(id);
          changed = true;
        }
      }
      if (!changed) {
        return current;
      }
      saveExpanded(assetId, next);
      return next;
    });
  };

  const toggleExpanded = (folderId: string) => {
    setExpanded((current) => {
      const next = new Set(current);
      if (next.has(folderId)) {
        next.delete(folderId);
      } else {
        next.add(folderId);
      }
      saveExpanded(assetId, next);
      return next;
    });
  };

  const handleCreateFolder = () => {
    const parentId = filingFolderId(selectedId);
    const folderId = createFolder(parentId);
    expandIds([parentId, folderId]);
    setRenamingId(folderId);
  };

  const readDrag = (): DragPayload | null => {
    if (!activeDrag || activeDrag.assetId !== assetId) {
      return null;
    }
    return activeDrag;
  };

  const finishDrag = () => {
    activeDrag = null;
    setDropTarget(null);
  };

  const acceptDropToFolder = (
    event: DragEvent,
    folderId: string | null,
  ): boolean => {
    const drag = readDrag();
    if (!drag) {
      return false;
    }
    if (drag.kind === 'folder' && !canMoveFolder(organization, drag.id, folderId)) {
      return false;
    }
    event.preventDefault();
    event.stopPropagation();
    event.dataTransfer.dropEffect = 'move';
    const targetKey = folderId ?? 'root';
    if (dropTarget !== targetKey) {
      setDropTarget(targetKey);
    }
    return true;
  };

  const applyDropToFolder = (event: DragEvent, folderId: string | null) => {
    event.preventDefault();
    event.stopPropagation();
    const drag = readDrag();
    finishDrag();
    if (!drag) {
      return;
    }
    if (drag.kind === 'item') {
      moveItem(drag.id, folderId);
      if (folderId) {
        expandIds([folderId]);
      }
      return;
    }
    if (canMoveFolder(organization, drag.id, folderId)) {
      moveFolder(drag.id, folderId);
      if (folderId) {
        expandIds([folderId]);
      }
    }
  };

  const folderHasDirectItems = (folderId: string) =>
    Object.values(organization.placements).some((placed) => placed === folderId);

  const handleDeleteFolder = (folder: FolderNode) => {
    if (folder.children.length > 0 || folderHasDirectItems(folder.id)) {
      const confirmed = window.confirm(
        `Delete folder "${folder.name}"? Items and subfolders move up to the parent folder.`,
      );
      if (!confirmed) {
        return;
      }
    }
    if (renamingId === folder.id) {
      setRenamingId(null);
    }
    deleteFolder(folder.id);
  };

  const onDragStart = (
    event: DragEvent<HTMLDivElement>,
    payload: DragPayload,
  ) => {
    activeDrag = payload;
    event.dataTransfer.effectAllowed = 'move';
    event.dataTransfer.setData('text/plain', JSON.stringify(payload));
    event.stopPropagation();
  };

  const visibleRootFolders = organization.folders.filter(
    (folder) => !reveal || reveal.has(folder.id),
  );
  const rootItems = items.filter(
    (item) => resolvedItemFolder(organization, item.id) == null,
  );
  const showEmpty = visibleRootFolders.length === 0 && rootItems.length === 0;

  const renderLevel = (
    folders: FolderNode[],
    parentId: string | null,
    depth: number,
  ) => {
    const visibleFolders = [...folders]
      .filter((folder) => !reveal || reveal.has(folder.id))
      .sort(byName);
    const levelItems = items
      .filter((item) => resolvedItemFolder(organization, item.id) === parentId)
      .sort(compareItems);

    return (
      <>
        {visibleFolders.map((folder) => {
          const open = reveal
            ? reveal.has(folder.id)
            : expanded.has(folder.id);
          return (
            <div key={folder.id}>
              <FolderRow
                folder={folder}
                depth={depth}
                open={open}
                selected={selectedFolderId === folder.id}
                dropTarget={dropTarget === folder.id}
                renaming={renamingId === folder.id}
                onToggle={() => {
                  if (!reveal) {
                    toggleExpanded(folder.id);
                  }
                }}
                onSelect={() => {
                  selectFolder(folder.id);
                  if (!reveal) {
                    toggleExpanded(folder.id);
                  }
                }}
                onRename={(name) => {
                  setRenamingId(null);
                  renameFolder(folder.id, name);
                }}
                onCancelRename={() => setRenamingId(null)}
                onDelete={() => handleDeleteFolder(folder)}
                onDragStart={(event) =>
                  onDragStart(event, { assetId, kind: 'folder', id: folder.id })
                }
                onDragEnd={finishDrag}
                onDragOver={(event) => {
                  acceptDropToFolder(event, folder.id);
                }}
                onDrop={(event) => {
                  applyDropToFolder(event, folder.id);
                }}
              />
              {open
                ? renderLevel(folder.children, folder.id, depth + 1)
                : null}
            </div>
          );
        })}
        {levelItems.map((item) => (
          <ItemRow
            key={item.id}
            item={item}
            depth={depth}
            selected={selectedId === item.id}
            renderItem={renderItem}
            onSelect={() => {
              selectFolder(null);
              onSelectItem(item.id);
            }}
            onClone={() => onClone(item.id)}
            onDelete={() => onDelete(item.id)}
            onDragStart={(event) =>
              onDragStart(event, { assetId, kind: 'item', id: item.id })
            }
            onDragEnd={finishDrag}
            onDragOver={(event) => {
              const drag = readDrag();
              if (drag?.kind === 'item' && drag.id === item.id) {
                event.stopPropagation();
                return;
              }
              acceptDropToFolder(event, parentId);
            }}
            onDrop={(event) => {
              const drag = readDrag();
              if (drag?.kind === 'item' && drag.id === item.id) {
                event.stopPropagation();
                finishDrag();
                return;
              }
              applyDropToFolder(event, parentId);
            }}
          />
        ))}
      </>
    );
  };

  return (
    <div className={`asset-tree-wrap asset-tree-wrap--${assetId}`}>
      <div className="asset-tree-toolbar">
        <Button variant="small" onClick={handleCreateFolder}>
          + Folder
        </Button>
      </div>
      <div
        className={`asset-tree${dropTarget === 'root' ? ' drop-target' : ''}`}
        onDragOver={(event) => {
          acceptDropToFolder(event, null);
        }}
        onDrop={(event) => {
          applyDropToFolder(event, null);
        }}
      >
        {showEmpty ? (
          <div className="empty-state">{emptyMessage}</div>
        ) : (
          <div className="asset-tree-content">
            {renderLevel(organization.folders, null, 0)}
          </div>
        )}
        <div
          className={`asset-tree-root-drop${
            dropTarget === 'root' ? ' drop-target' : ''
          }`}
          aria-label="Drop here to move to top level"
          onDragOver={(event) => {
            acceptDropToFolder(event, null);
          }}
          onDrop={(event) => {
            applyDropToFolder(event, null);
          }}
        />
      </div>
    </div>
  );
}

interface FolderRowProps {
  folder: FolderNode;
  depth: number;
  open: boolean;
  selected: boolean;
  dropTarget: boolean;
  renaming: boolean;
  onToggle: () => void;
  onSelect: () => void;
  onRename: (name: string) => void;
  onCancelRename: () => void;
  onDelete: () => void;
  onDragStart: (event: DragEvent<HTMLDivElement>) => void;
  onDragEnd: () => void;
  onDragOver: (event: DragEvent<HTMLDivElement>) => void;
  onDrop: (event: DragEvent<HTMLDivElement>) => void;
}

function FolderRow({
  folder,
  depth,
  open,
  selected,
  dropTarget,
  renaming,
  onToggle,
  onSelect,
  onRename,
  onCancelRename,
  onDelete,
  onDragStart,
  onDragEnd,
  onDragOver,
  onDrop,
}: FolderRowProps) {
  const className = [
    'asset-tree-row',
    'asset-tree-folder',
    selected ? 'folder-selected' : '',
    dropTarget ? 'drop-target' : '',
  ]
    .filter(Boolean)
    .join(' ');

  return (
    <div
      className={className}
      style={{ paddingLeft: 4 + depth * 14 }}
      draggable={!renaming}
      onClick={onSelect}
      onDragStart={onDragStart}
      onDragEnd={onDragEnd}
      onDragOver={onDragOver}
      onDrop={onDrop}
    >
      <button
        type="button"
        className="asset-tree-chevron"
        aria-label={open ? 'Collapse folder' : 'Expand folder'}
        onMouseDown={stopRowControl}
        onClick={(event) => {
          event.stopPropagation();
          onToggle();
        }}
      >
        {open ? '▾' : '▸'}
      </button>
      {renaming ? (
        <FolderRenameInput
          initialName={folder.name}
          onRename={onRename}
          onCancel={onCancelRename}
        />
      ) : (
        <span className="asset-tree-folder-name">{folder.name}</span>
      )}
      <button
        type="button"
        className="asset-tree-folder-delete"
        aria-label="Delete folder"
        title="Delete folder"
        onMouseDown={stopRowControl}
        onClick={(event) => {
          event.stopPropagation();
          onDelete();
        }}
      >
        ×
      </button>
    </div>
  );
}

function FolderRenameInput({
  initialName,
  onRename,
  onCancel,
}: {
  initialName: string;
  onRename: (name: string) => void;
  onCancel: () => void;
}) {
  const [value, setValue] = useState(initialName);
  const finished = useRef(false);

  const commit = (next: string) => {
    if (finished.current) {
      return;
    }
    finished.current = true;
    const trimmed = next.trim();
    if (!trimmed || trimmed === initialName) {
      onCancel();
      return;
    }
    onRename(trimmed);
  };

  return (
    <input
      className="asset-tree-rename"
      autoFocus
      value={value}
      aria-label="Folder name"
      onChange={(event) => setValue(event.target.value)}
      onClick={stopRowControl}
      onDoubleClick={stopRowControl}
      onMouseDown={stopRowControl}
      onKeyDown={(event) => {
        event.stopPropagation();
        if (event.key === 'Enter') {
          commit(value);
        } else if (event.key === 'Escape') {
          if (!finished.current) {
            finished.current = true;
            onCancel();
          }
        }
      }}
      onBlur={() => commit(value)}
    />
  );
}

interface ItemRowProps {
  item: AssetTreeItem;
  depth: number;
  selected: boolean;
  renderItem?: (item: AssetTreeItem) => ReactNode;
  onSelect: () => void;
  onClone: () => void;
  onDelete: () => void;
  onDragStart: (event: DragEvent<HTMLDivElement>) => void;
  onDragEnd: () => void;
  onDragOver: (event: DragEvent<HTMLDivElement>) => void;
  onDrop: (event: DragEvent<HTMLDivElement>) => void;
}

function ItemRow({
  item,
  depth,
  selected,
  renderItem,
  onSelect,
  onClone,
  onDelete,
  onDragStart,
  onDragEnd,
  onDragOver,
  onDrop,
}: ItemRowProps) {
  return (
    <div
      id={assetTreeItemElementId(item.id)}
      className={`asset-tree-row asset-tree-item${selected ? ' selected' : ''}`}
      style={{ paddingLeft: 4 + depth * 14 }}
      draggable
      onClick={onSelect}
      onDragStart={onDragStart}
      onDragEnd={onDragEnd}
      onDragOver={onDragOver}
      onDrop={onDrop}
    >
      {item.icon ? (
        <span className="asset-tree-file-icon">{item.icon}</span>
      ) : (
        <span className="asset-tree-chevron asset-tree-chevron-spacer" aria-hidden />
      )}
      {renderItem ? (
        renderItem(item)
      ) : (
        <div className="asset-tree-item-body">
          <div className="asset-tree-item-header">
            <span className="asset-tree-item-label">{item.label || item.id}</span>
            <div
              className="item-card-actions"
              onClick={stopRowControl}
              onMouseDown={stopRowControl}
            >
              <ListCardActions onClone={onClone} onDelete={onDelete} />
            </div>
          </div>
          {item.subtitle ? <div className="item-name">{item.subtitle}</div> : null}
          {item.extra}
        </div>
      )}
    </div>
  );
}
