/** Editor routes / asset-type ids used as localStorage keys. */
export type EditorSelectionKey =
  | 'itemTemplates'
  | 'abilityTemplates'
  | 'statusEffectTemplates'
  | 'characterTemplates'
  | 'tilesetTemplates'
  | 'featTemplates'
  | 'specialEvents';

const STORAGE_KEY = 'ceditor.editorSelection';

/** Hash query param names (URL overrides stored selection). */
export const EDITOR_SELECTION_ROUTE_PARAMS: Partial<
  Record<EditorSelectionKey, string>
> = {
  itemTemplates: 'item',
  abilityTemplates: 'ability',
  statusEffectTemplates: 'statusEffect',
  characterTemplates: 'character',
  tilesetTemplates: 'tileset',
  specialEvents: 'event',
};

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
