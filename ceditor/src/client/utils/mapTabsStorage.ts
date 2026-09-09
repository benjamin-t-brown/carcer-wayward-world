import type { CarcerMapTemplate, MapGridTemplate } from '../types/assets';
import { findMapGridPlacement } from './mapGridIndex';

const STORAGE_KEY = 'ceditor.maps.openTabs';

/**
 * A map-editor tab is bound to a map grid (all of the grid's maps are reached
 * from the one tab by navigating on the canvas) or, when `gridName` is null, to
 * a single grid-less map.
 */
export interface PersistedMapTab {
  gridName: string | null;
  activeMapName: string;
}

export interface PersistedMapTabs {
  tabs: PersistedMapTab[];
  activeTabIndex: number | null;
}

export type RestoredMapTab = PersistedMapTab;

export interface RestoredMapTabsState {
  tabs: RestoredMapTab[];
  activeTabIndex: number | null;
}

/** Pre-grid storage shape: one tab per map, no grid grouping. */
interface LegacyPersistedMapTabs {
  openMapNames: string[];
  activeMapName: string | null;
}

function readRaw(): unknown {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    return raw ? JSON.parse(raw) : null;
  } catch {
    return null;
  }
}

function isTabArray(value: unknown): value is PersistedMapTab[] {
  return Array.isArray(value);
}

export function loadPersistedMapTabs(): PersistedMapTabs | null {
  const parsed = readRaw() as Partial<PersistedMapTabs> | null;
  if (!parsed || !isTabArray(parsed.tabs)) {
    return null;
  }
  const tabs = parsed.tabs.filter(
    (tab): tab is PersistedMapTab =>
      !!tab &&
      typeof tab.activeMapName === 'string' &&
      tab.activeMapName.length > 0 &&
      (tab.gridName === null || typeof tab.gridName === 'string')
  );
  return {
    tabs,
    activeTabIndex:
      typeof parsed.activeTabIndex === 'number' ? parsed.activeTabIndex : null,
  };
}

export function savePersistedMapTabs(
  tabs: PersistedMapTab[],
  activeTabIndex: number | null
): void {
  try {
    const payload: PersistedMapTabs = { tabs, activeTabIndex };
    localStorage.setItem(STORAGE_KEY, JSON.stringify(payload));
  } catch {
    // Ignore quota / private mode errors
  }
}

/**
 * Resolve persisted tabs against the loaded maps and grids: drop tabs whose map
 * no longer exists, re-bind a tab to its map's current grid, and collapse tabs
 * that would target the same grid. Also migrates the pre-grid storage shape.
 */
export function restoreMapTabsFromStorage(
  maps: CarcerMapTemplate[],
  mapGrids: MapGridTemplate[]
): RestoredMapTabsState {
  if (maps.length === 0) {
    return { tabs: [], activeTabIndex: null };
  }

  const parsed = readRaw() as
    | (Partial<PersistedMapTabs> & Partial<LegacyPersistedMapTabs>)
    | null;
  if (!parsed) {
    return { tabs: [], activeTabIndex: null };
  }

  const mapExists = (name: string) => maps.some((m) => m.name === name);
  const gridOf = (mapName: string) =>
    findMapGridPlacement(mapName, mapGrids)?.grid.name ?? null;

  let sourceTabs: PersistedMapTab[];
  let activeMapNameHint: string | null = null;
  let activeTabIndexHint: number | null = null;

  if (isTabArray(parsed.tabs)) {
    sourceTabs = parsed.tabs;
    activeTabIndexHint =
      typeof parsed.activeTabIndex === 'number' ? parsed.activeTabIndex : null;
    if (activeTabIndexHint !== null) {
      activeMapNameHint = parsed.tabs[activeTabIndexHint]?.activeMapName ?? null;
    }
  } else if (Array.isArray(parsed.openMapNames)) {
    sourceTabs = parsed.openMapNames
      .filter((name): name is string => typeof name === 'string' && !!name)
      .map((name) => ({ gridName: gridOf(name), activeMapName: name }));
    activeMapNameHint =
      typeof parsed.activeMapName === 'string' ? parsed.activeMapName : null;
  } else {
    return { tabs: [], activeTabIndex: null };
  }

  const tabs: RestoredMapTab[] = [];
  const seenGrids = new Set<string>();
  const seenGridlessMaps = new Set<string>();

  for (const tab of sourceTabs) {
    if (!tab || !mapExists(tab.activeMapName)) {
      continue;
    }
    // Re-bind to the map's current grid; a stale grid name is dropped.
    const gridName = tab.gridName ? gridOf(tab.activeMapName) : null;
    if (gridName) {
      if (seenGrids.has(gridName)) {
        continue;
      }
      seenGrids.add(gridName);
    } else {
      if (seenGridlessMaps.has(tab.activeMapName)) {
        continue;
      }
      seenGridlessMaps.add(tab.activeMapName);
    }
    tabs.push({ gridName, activeMapName: tab.activeMapName });
  }

  if (tabs.length === 0) {
    return { tabs: [], activeTabIndex: null };
  }

  let activeTabIndex = 0;
  if (activeMapNameHint) {
    const idx = tabs.findIndex((tab) => tab.activeMapName === activeMapNameHint);
    if (idx >= 0) {
      activeTabIndex = idx;
    }
  }

  return { tabs, activeTabIndex };
}

/** Open the map editor in a new browser tab with this map active. */
export function openMapEditorInNewTab(mapName: string): void {
  const base = `${window.location.origin}${window.location.pathname}`;
  const url = `${base}#/editor/maps?map=${encodeURIComponent(mapName)}`;
  window.open(url, '_blank', 'noopener,noreferrer');
}

/** Open the map grid editor in a new browser tab with this grid selected. */
export function openMapGridEditorInNewTab(gridName: string): void {
  const base = `${window.location.origin}${window.location.pathname}`;
  const url = `${base}#/editor/mapGrids?mapGrid=${encodeURIComponent(gridName)}`;
  window.open(url, '_blank', 'noopener,noreferrer');
}
