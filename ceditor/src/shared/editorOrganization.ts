/**
 * Editor-only folder layout for asset sidebars. The game never reads this file.
 * Record identity stays the asset id/name; placements only say which folder holds it.
 */

import { isAssetId, type AssetId } from './assetRegistry';

export type OrganizableAssetId = Exclude<AssetId, 'maps'>;

export interface FolderNode {
  id: string;
  name: string;
  children: FolderNode[];
}

export interface AssetOrganization {
  folders: FolderNode[];
  /** Record id or name -> folder id. Missing key means the record sits at the root. */
  placements: Record<string, string>;
}

export type EditorOrganizationFile = Partial<
  Record<OrganizableAssetId, AssetOrganization>
>;

export const EMPTY_ASSET_ORGANIZATION: AssetOrganization = {
  folders: [],
  placements: {},
};

export function isOrganizableAssetId(id: string): id is OrganizableAssetId {
  return isAssetId(id) && id !== 'maps';
}

export function findFolder(
  folders: FolderNode[],
  folderId: string,
): FolderNode | null {
  for (const folder of folders) {
    if (folder.id === folderId) {
      return folder;
    }
    const nested = findFolder(folder.children, folderId);
    if (nested) {
      return nested;
    }
  }
  return null;
}

/** Parent folder id, null when the folder is at the root, undefined when missing. */
export function findParentId(
  folders: FolderNode[],
  folderId: string,
  parentId: string | null = null,
): string | null | undefined {
  for (const folder of folders) {
    if (folder.id === folderId) {
      return parentId;
    }
    const nested = findParentId(folder.children, folderId, folder.id);
    if (nested !== undefined) {
      return nested;
    }
  }
  return undefined;
}

export function siblingFolders(
  org: AssetOrganization,
  parentId: string | null,
): FolderNode[] {
  if (parentId == null) {
    return org.folders;
  }
  return findFolder(org.folders, parentId)?.children ?? [];
}

export function resolvedItemFolder(
  org: AssetOrganization,
  itemId: string,
): string | null {
  const folderId = org.placements[itemId];
  if (!folderId || !findFolder(org.folders, folderId)) {
    return null;
  }
  return folderId;
}

/** Folders that contain a visible item, plus their ancestors. */
export function revealFolderIds(
  org: AssetOrganization,
  itemIds: readonly string[],
): Set<string> {
  const reveal = new Set<string>();
  for (const itemId of itemIds) {
    let current = resolvedItemFolder(org, itemId);
    while (current) {
      if (reveal.has(current)) {
        break;
      }
      reveal.add(current);
      const parent = findParentId(org.folders, current);
      if (parent == null) {
        break;
      }
      current = parent;
    }
  }
  return reveal;
}

export function canMoveFolder(
  org: AssetOrganization,
  folderId: string,
  newParentId: string | null,
): boolean {
  if (!findFolder(org.folders, folderId)) {
    return false;
  }
  if (newParentId == null) {
    return true;
  }
  if (newParentId === folderId) {
    return false;
  }
  const target = findFolder(org.folders, newParentId);
  if (!target) {
    return false;
  }
  return findFolder([findFolder(org.folders, folderId)!], newParentId) == null;
}

export function withSortedPlacements(org: AssetOrganization): AssetOrganization {
  const placements: Record<string, string> = {};
  for (const key of Object.keys(org.placements).sort((a, b) =>
    a.localeCompare(b),
  )) {
    placements[key] = org.placements[key];
  }
  return { folders: org.folders, placements };
}

export function organizationsEqual(
  a: AssetOrganization,
  b: AssetOrganization,
): boolean {
  return JSON.stringify(withSortedPlacements(a)) === JSON.stringify(withSortedPlacements(b));
}

function uniqueFolderName(siblings: FolderNode[], base: string): string {
  const names = new Set(siblings.map((folder) => folder.name));
  if (!names.has(base)) {
    return base;
  }
  let index = 2;
  while (names.has(`${base} ${index}`)) {
    index += 1;
  }
  return `${base} ${index}`;
}

export function createFolder(
  org: AssetOrganization,
  parentId: string | null,
  folderId: string,
  baseName = 'New Folder',
): AssetOrganization {
  const parent =
    parentId && findFolder(org.folders, parentId) ? parentId : null;
  const name = uniqueFolderName(siblingFolders(org, parent), baseName);
  const node: FolderNode = { id: folderId, name, children: [] };
  if (parent == null) {
    return { ...org, folders: [...org.folders, node] };
  }
  const folders = insertChildren(org.folders, parent, [node]);
  return folders ? { ...org, folders } : { ...org, folders: [...org.folders, node] };
}

export function renameFolder(
  org: AssetOrganization,
  folderId: string,
  name: string,
): AssetOrganization {
  const trimmed = name.trim();
  if (!trimmed) {
    return org;
  }
  const folders = renameInTree(org.folders, folderId, trimmed);
  if (!folders) {
    return org;
  }
  return { ...org, folders };
}

export function deleteFolder(
  org: AssetOrganization,
  folderId: string,
): AssetOrganization {
  const parentId = findParentId(org.folders, folderId);
  if (parentId === undefined) {
    return org;
  }
  const removed = removeFolder(org.folders, folderId);
  if (!removed.removed) {
    return org;
  }
  const folders =
    parentId == null
      ? [...removed.folders, ...removed.removed.children]
      : insertChildren(removed.folders, parentId, removed.removed.children) ??
        removed.folders;
  const placements: Record<string, string> = {};
  for (const [itemId, placedIn] of Object.entries(org.placements)) {
    if (placedIn !== folderId) {
      placements[itemId] = placedIn;
      continue;
    }
    if (parentId) {
      placements[itemId] = parentId;
    }
  }
  return { folders, placements };
}

export function moveFolder(
  org: AssetOrganization,
  folderId: string,
  newParentId: string | null,
): AssetOrganization {
  if (!canMoveFolder(org, folderId, newParentId)) {
    return org;
  }
  const currentParent = findParentId(org.folders, folderId);
  if (currentParent === undefined || currentParent === newParentId) {
    return org;
  }
  const removed = removeFolder(org.folders, folderId);
  if (!removed.removed) {
    return org;
  }
  if (newParentId == null) {
    return { ...org, folders: [...removed.folders, removed.removed] };
  }
  const folders = insertChildren(removed.folders, newParentId, [removed.removed]);
  if (!folders) {
    return org;
  }
  return { ...org, folders };
}

export function moveItem(
  org: AssetOrganization,
  itemId: string,
  folderId: string | null,
): AssetOrganization {
  if (!itemId) {
    return org;
  }
  if (folderId == null) {
    if (!(itemId in org.placements)) {
      return org;
    }
    const placements = { ...org.placements };
    delete placements[itemId];
    return { ...org, placements };
  }
  if (!findFolder(org.folders, folderId)) {
    return org;
  }
  if (org.placements[itemId] === folderId) {
    return org;
  }
  return {
    ...org,
    placements: { ...org.placements, [itemId]: folderId },
  };
}

export function rekeyItem(
  org: AssetOrganization,
  oldId: string,
  newId: string,
): AssetOrganization {
  if (!oldId || !newId || oldId === newId || !(oldId in org.placements)) {
    return org;
  }
  const placements = { ...org.placements };
  const folderId = placements[oldId];
  delete placements[oldId];
  placements[newId] = folderId;
  return { ...org, placements };
}

export function removeItem(
  org: AssetOrganization,
  itemId: string,
): AssetOrganization {
  if (!(itemId in org.placements)) {
    return org;
  }
  const placements = { ...org.placements };
  delete placements[itemId];
  return { ...org, placements };
}

export function copyPlacement(
  org: AssetOrganization,
  sourceId: string,
  newId: string,
): AssetOrganization {
  if (!newId || sourceId === newId) {
    return org;
  }
  const folderId = org.placements[sourceId];
  if (!folderId) {
    return removeItem(org, newId);
  }
  return moveItem(org, newId, folderId);
}

export function pruneMissing(
  org: AssetOrganization,
  liveIds: readonly string[],
): AssetOrganization {
  const live = new Set(liveIds);
  let changed = false;
  const placements: Record<string, string> = {};
  for (const [itemId, folderId] of Object.entries(org.placements)) {
    if (!live.has(itemId)) {
      changed = true;
      continue;
    }
    placements[itemId] = folderId;
  }
  return changed ? { ...org, placements } : org;
}

export function filingFolderId(
  org: AssetOrganization,
  selectedFolderId: string | null,
  selectedItemId: string | null,
): string | null {
  if (selectedFolderId && findFolder(org.folders, selectedFolderId)) {
    return selectedFolderId;
  }
  if (selectedItemId) {
    return resolvedItemFolder(org, selectedItemId);
  }
  return null;
}

export function normalizeAssetOrganization(raw: unknown): AssetOrganization {
  if (!raw || typeof raw !== 'object' || Array.isArray(raw)) {
    return { folders: [], placements: {} };
  }
  const record = raw as { folders?: unknown; placements?: unknown };
  const seen = new Set<string>();
  const folders = normalizeFolders(record.folders, seen);
  const known = seen;
  const placements: Record<string, string> = {};
  if (record.placements && typeof record.placements === 'object' && !Array.isArray(record.placements)) {
    for (const [itemId, folderId] of Object.entries(
      record.placements as Record<string, unknown>,
    )) {
      if (typeof folderId === 'string' && known.has(folderId)) {
        placements[itemId] = folderId;
      }
    }
  }
  return withSortedPlacements({ folders, placements });
}

export function normalizeOrganizationFile(raw: unknown): EditorOrganizationFile {
  if (!raw || typeof raw !== 'object' || Array.isArray(raw)) {
    return {};
  }
  const file: EditorOrganizationFile = {};
  for (const [key, value] of Object.entries(raw as Record<string, unknown>)) {
    if (!isOrganizableAssetId(key)) {
      continue;
    }
    file[key] = normalizeAssetOrganization(value);
  }
  return file;
}

function normalizeFolders(raw: unknown, seen: Set<string>): FolderNode[] {
  if (!Array.isArray(raw)) {
    return [];
  }
  const folders: FolderNode[] = [];
  for (const entry of raw) {
    if (!entry || typeof entry !== 'object' || Array.isArray(entry)) {
      continue;
    }
    const node = entry as { id?: unknown; name?: unknown; children?: unknown };
    if (typeof node.id !== 'string' || !node.id || seen.has(node.id)) {
      continue;
    }
    seen.add(node.id);
    const name =
      typeof node.name === 'string' && node.name.trim()
        ? node.name.trim()
        : 'Folder';
    folders.push({
      id: node.id,
      name,
      children: normalizeFolders(node.children, seen),
    });
  }
  return folders;
}

function removeFolder(
  folders: FolderNode[],
  folderId: string,
): { folders: FolderNode[]; removed: FolderNode | null } {
  for (let index = 0; index < folders.length; index += 1) {
    const folder = folders[index];
    if (folder.id === folderId) {
      return {
        removed: folder,
        folders: [...folders.slice(0, index), ...folders.slice(index + 1)],
      };
    }
    const nested = removeFolder(folder.children, folderId);
    if (nested.removed) {
      const updated = { ...folder, children: nested.folders };
      return {
        removed: nested.removed,
        folders: [...folders.slice(0, index), updated, ...folders.slice(index + 1)],
      };
    }
  }
  return { folders, removed: null };
}

function insertChildren(
  folders: FolderNode[],
  parentId: string,
  children: FolderNode[],
): FolderNode[] | null {
  for (let index = 0; index < folders.length; index += 1) {
    const folder = folders[index];
    if (folder.id === parentId) {
      const updated = {
        ...folder,
        children: [...folder.children, ...children],
      };
      return [...folders.slice(0, index), updated, ...folders.slice(index + 1)];
    }
    const nested = insertChildren(folder.children, parentId, children);
    if (nested) {
      const updated = { ...folder, children: nested };
      return [...folders.slice(0, index), updated, ...folders.slice(index + 1)];
    }
  }
  return null;
}

function renameInTree(
  folders: FolderNode[],
  folderId: string,
  name: string,
): FolderNode[] | null {
  for (let index = 0; index < folders.length; index += 1) {
    const folder = folders[index];
    if (folder.id === folderId) {
      if (folder.name === name) {
        return null;
      }
      const updated = { ...folder, name };
      return [...folders.slice(0, index), updated, ...folders.slice(index + 1)];
    }
    const nested = renameInTree(folder.children, folderId, name);
    if (nested) {
      const updated = { ...folder, children: nested };
      return [...folders.slice(0, index), updated, ...folders.slice(index + 1)];
    }
  }
  return null;
}
