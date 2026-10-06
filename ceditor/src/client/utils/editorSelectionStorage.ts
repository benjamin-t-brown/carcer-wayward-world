import { AssetId, ASSET_ROUTE_PARAM_BY_ID } from '../../shared/assetRegistry';
import { readHashRoute } from './hashRoute';

/** Editor routes / asset-type ids used as localStorage keys. */
export type EditorSelectionKey = AssetId;

const STORAGE_KEY = 'ceditor.editorSelection';

/** Hash query param names (URL overrides stored selection). */
export const EDITOR_SELECTION_ROUTE_PARAMS: Partial<
  Record<EditorSelectionKey, string>
> = ASSET_ROUTE_PARAM_BY_ID;

type SelectionStore = Partial<Record<EditorSelectionKey, string>>;

function readStore(): SelectionStore {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) {
      return {};
    }
    const parsed = JSON.parse(raw) as SelectionStore;
    return parsed && typeof parsed === 'object' ? parsed : {};
  } catch {
    return {};
  }
}

function writeStore(store: SelectionStore): void {
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(store));
  } catch {
    // Ignore quota / private mode errors
  }
}

export function loadEditorSelection(key: EditorSelectionKey): string | null {
  const id = readStore()[key];
  return typeof id === 'string' && id.length > 0 ? id : null;
}

export function saveEditorSelection(
  key: EditorSelectionKey,
  entityId: string | null
): void {
  const store = readStore();
  if (entityId) {
    store[key] = entityId;
  } else {
    delete store[key];
  }
  writeStore(store);
}

export function resolveSelectionFromRoute(
  key: EditorSelectionKey,
  routeParams?: URLSearchParams
): string | null {
  const paramName = EDITOR_SELECTION_ROUTE_PARAMS[key];
  if (!paramName || !routeParams) {
    return null;
  }
  const value = routeParams.get(paramName);
  return value && value.length > 0 ? value : null;
}

/** Keep the hash query param in sync with the selected entity (e.g. ?character=). */
export function syncSelectionToRoute(
  key: EditorSelectionKey,
  entityId: string | null,
  options?: { history?: 'push' | 'replace' }
): void {
  const paramName = EDITOR_SELECTION_ROUTE_PARAMS[key];
  if (!paramName || typeof window === 'undefined') {
    return;
  }

  const expectedPath = `/editor/${key}`;
  const { path, params } = readHashRoute();
  if (path !== expectedPath) {
    return;
  }

  const current = params.get(paramName);
  if (entityId) {
    if (current === entityId) {
      return;
    }
    params.set(paramName, entityId);
  } else if (current !== null) {
    params.delete(paramName);
  } else {
    return;
  }

  const query = params.toString();
  const nextHash = `#${path}${query ? `?${query}` : ''}`;
  if (window.location.hash === nextHash) {
    return;
  }

  const historyMode = options?.history ?? 'push';
  if (historyMode === 'replace') {
    window.history.replaceState(
      window.history.state,
      '',
      `${window.location.pathname}${window.location.search}${nextHash}`
    );
    window.dispatchEvent(new HashChangeEvent('hashchange'));
    return;
  }

  // Assigning hash pushes a history entry and fires hashchange for back/forward.
  window.location.hash = nextHash;
}
