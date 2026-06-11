import type { CarcerMapTemplate } from '../types/assets';

const STORAGE_KEY = 'ceditor.maps.openTabs';

export interface PersistedMapTabs {
  openMapNames: string[];
  activeMapName: string | null;
}

export interface RestoredMapTab {
  mapIndex: number;
  map: CarcerMapTemplate;
}

export interface RestoredMapTabsState {
  tabs: RestoredMapTab[];
  activeTabIndex: number | null;
}

export function loadPersistedMapTabs(): PersistedMapTabs | null {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) {
      return null;
    }
    const parsed = JSON.parse(raw) as PersistedMapTabs;
    if (!parsed || !Array.isArray(parsed.openMapNames)) {
      return null;
    }
    return {
      openMapNames: parsed.openMapNames.filter(
        (name): name is string => typeof name === 'string' && name.length > 0
      ),
      activeMapName:
        typeof parsed.activeMapName === 'string' ? parsed.activeMapName : null,
    };
  } catch {
    return null;
  }
}

export function savePersistedMapTabs(
  openMapNames: string[],
  activeMapName: string | null
): void {
  try {
    const payload: PersistedMapTabs = { openMapNames, activeMapName };
    localStorage.setItem(STORAGE_KEY, JSON.stringify(payload));
  } catch {
    // Ignore quota / private mode errors
  }
}

/** Resolve persisted tab names against the loaded map list (stable across reloads). */
export function restoreMapTabsFromStorage(
  maps: CarcerMapTemplate[]
): RestoredMapTabsState {
  const persisted = loadPersistedMapTabs();
  if (!persisted?.openMapNames.length || maps.length === 0) {
    return { tabs: [], activeTabIndex: null };
  }

  const tabs: RestoredMapTab[] = [];
  for (const mapName of persisted.openMapNames) {
    const mapIndex = maps.findIndex((m) => m.name === mapName);
    if (mapIndex >= 0) {
      tabs.push({ mapIndex, map: maps[mapIndex] });
    }
  }

  if (tabs.length === 0) {
    return { tabs: [], activeTabIndex: null };
  }

  let activeTabIndex = 0;
  if (persisted.activeMapName) {
    const idx = tabs.findIndex((tab) => tab.map.name === persisted.activeMapName);
    if (idx >= 0) {
      activeTabIndex = idx;
    }
  }

  return { tabs, activeTabIndex };
}

/** Open the map editor in a new browser tab with this map active. */
export function openMapEditorInNewTab(mapName: string): void {
  const persisted = loadPersistedMapTabs();
  const openNames = persisted?.openMapNames ?? [];
  const nextNames = openNames.includes(mapName)
    ? openNames
    : [...openNames, mapName];
  savePersistedMapTabs(nextNames, mapName);

  const base = `${window.location.origin}${window.location.pathname}`;
  const url = `${base}#/editor/maps?map=${encodeURIComponent(mapName)}`;
  window.open(url, '_blank', 'noopener,noreferrer');
}
