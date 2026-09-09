import { useState, useRef, useEffect, useLayoutEffect } from 'react';
import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
  MapGridTemplate,
  MAP_TYPES,
  sanitizeMapGridTemplates,
} from '../types/assets';
import { Button } from '../elements/Button';
import { EditorHeader } from '../components/EditorHeader';
import { Notification } from '../elements/Notification';
import { OptionSelect } from '../elements/OptionSelect';
import { useAssets } from '../contexts/AssetsContext';
import { trimStrings } from '../utils/jsonUtils';
import { DeleteModal } from '../elements/DeleteModal';
import { CreateMapModal, CreateMapConstraints } from '../components/CreateMapModal';
import { EditMapModal } from '../components/EditMapModal';
import {
  OpenMapAndSelectTileArgs,
  TileEditor,
} from '../tile-editor/TileEditor';
import {
  createEditorStateMap,
  getEditorState,
  renameEditorStateMap,
} from '../tile-editor/editorState';
import { createTilesForLayer, prepareNewMapForEditor } from '../utils/mapIndex';
import {
  assignMapToGridCell,
  findMapGridPlacement,
  getGridLayerSet,
  renameMapInGrids,
} from '../utils/mapGridIndex';
import {
  GridNavigateStitchOffset,
  GridSlotCreateRequest,
  switchMapViewport,
  switchMapViewportPreservingStitch,
  saveViewportForMap,
} from '../tile-editor/editorEvents';
import {
  findMarkerOnMap,
  locateOnCurrentMap,
} from '../tile-editor/mapLocate';
import {
  loadPersistedMapTabs,
  restoreMapTabsFromStorage,
  savePersistedMapTabs,
} from '../utils/mapTabsStorage';

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
}

/**
 * A tab is bound to a map grid (every map in the grid is reached from the one
 * tab by navigating on the canvas) or, when `gridName` is null, to a single
 * grid-less map. `activeMapName` is the map currently shown in the tab.
 */
interface OpenTab {
  gridName: string | null;
  activeMapName: string;
}

const tabKey = (tab: OpenTab) =>
  tab.gridName ? `grid:${tab.gridName}` : `map:${tab.activeMapName}`;

const createEditorStateMapForTabIfNotExists = (mapName: string) => {
  if (!mapName) {
    return;
  }
  if (!getEditorState().maps[mapName]) {
    createEditorStateMap(mapName);
  }
};

const getDuplicateMapName = (
  baseName: string,
  existingNames: Set<string>
): string => {
  let candidate = `${baseName}_copy`;
  let n = 2;
  while (existingNames.has(candidate)) {
    candidate = `${baseName}_copy${n}`;
    n++;
  }
  return candidate;
};

interface MapsProps {
  routeParams?: URLSearchParams;
}

export function Maps({ routeParams }: MapsProps = {}) {
  const { maps, setMaps, saveMaps, mapGrids, setMapGrids, saveMapGrids } =
    useAssets();
  const [openTabs, _setOpenTabs] = useState<OpenTab[]>([]);
  const [activeTabIndex, _setActiveTabIndex] = useState<number | null>(null);
  const [selectedMapIndex, setSelectedMapIndex] = useState<string>('');
  // Both header dropdowns act as one-shot pickers: the value stays '' so the
  // placeholder shows again after each selection.
  const [selectedGridIndex] = useState<string>('');
  const [notifications, setNotifications] = useState<NotificationState[]>([]);
  const notificationIdRef = useRef(0);
  const [deleteConfirm, setDeleteConfirm] = useState<{
    isOpen: boolean;
    mapIndex: number | null;
  }>({ isOpen: false, mapIndex: null });
  const [createModalOpen, setCreateModalOpen] = useState(false);
  const [gridCreateRequest, setGridCreateRequest] =
    useState<GridSlotCreateRequest | null>(null);
  const [editModalOpen, setEditModalOpen] = useState(false);
  const hasRestoredTabsRef = useRef(false);
  const isFirstPersistRef = useRef(true);
  const consumedMapParamRef = useRef(false);
  const [tabsHydrated, setTabsHydrated] = useState(false);

  const mapByName = (name: string): CarcerMapTemplate | null =>
    maps.find((m) => m.name === name) ?? null;

  const gridNameForMap = (mapName: string): string | null =>
    findMapGridPlacement(mapName, mapGrids)?.grid.name ?? null;

  const persistOpenTabs = (tabs: OpenTab[], activeIndex: number | null) => {
    savePersistedMapTabs(
      tabs.map((tab) => ({
        gridName: tab.gridName,
        activeMapName: tab.activeMapName,
      })),
      activeIndex
    );
  };

  const setOpenTabs = (tabs: OpenTab[]) => {
    tabs.forEach((tab) => {
      createEditorStateMapForTabIfNotExists(tab.activeMapName);
    });
    _setOpenTabs(tabs);
  };

  // Move the editor onto a new map: sync viewport (stitched when navigating
  // within a grid) and point editor state at it.
  const applyActiveMapChange = (
    previousMapName: string,
    previousMap: CarcerMapTemplate | null,
    nextMapName: string,
    stitchOffset?: GridNavigateStitchOffset
  ) => {
    if (!nextMapName) {
      return;
    }
    createEditorStateMapForTabIfNotExists(nextMapName);
    if (previousMapName !== nextMapName) {
      if (stitchOffset && previousMap) {
        const placement = findMapGridPlacement(previousMapName, mapGrids);
        const slotTileW = placement?.grid.mapWidth ?? previousMap.width;
        const slotTileH = placement?.grid.mapHeight ?? previousMap.height;
        switchMapViewportPreservingStitch(
          previousMapName,
          nextMapName,
          stitchOffset,
          slotTileW * previousMap.spriteWidth,
          slotTileH * previousMap.spriteHeight
        );
      } else {
        switchMapViewport(previousMapName, nextMapName);
      }
    }
    const es = getEditorState();
    es.selectedMapName = nextMapName;
    // No stroke is in flight across a focus change.
    es.activePaintMapName = '';
    es.hoveredGridMapName = '';
    // Grid-wide undo order only makes sense within one grid.
    if (gridNameForMap(previousMapName) !== gridNameForMap(nextMapName)) {
      es.gridUndoOrder = [];
    }
  };

  const setActiveTabIndex = (
    index: number | null,
    tabsForLookup: OpenTab[] = openTabs,
    stitchOffset?: GridNavigateStitchOffset
  ) => {
    const previousTab =
      activeTabIndex !== null ? tabsForLookup[activeTabIndex] : undefined;
    const previousMapName = previousTab?.activeMapName ?? '';
    const previousMap = previousMapName ? mapByName(previousMapName) : null;

    if (index !== null) {
      const tab = tabsForLookup[index];
      if (tab) {
        applyActiveMapChange(
          previousMapName,
          previousMap,
          tab.activeMapName,
          stitchOffset
        );
      }
    } else if (previousMapName) {
      saveViewportForMap(previousMapName);
    }
    _setActiveTabIndex(index);
  };

  // Swap which map a tab shows (grid navigation, or the "open a map" dropdown
  // landing on a grid that is already open). Returns the next tab list.
  const setTabActiveMap = (
    tabIndex: number,
    mapName: string,
    tabsForLookup: OpenTab[] = openTabs,
    stitchOffset?: GridNavigateStitchOffset
  ): OpenTab[] => {
    const tab = tabsForLookup[tabIndex];
    const nextTabs = tabsForLookup.map((t, i) =>
      i === tabIndex ? { ...t, activeMapName: mapName } : t
    );
    setOpenTabs(nextTabs);
    if (tabIndex === activeTabIndex && tab) {
      applyActiveMapChange(
        tab.activeMapName,
        mapByName(tab.activeMapName),
        mapName,
        stitchOffset
      );
    }
    return nextTabs;
  };

  // Open `mapName` in the tab for its grid (creating that tab if needed). If the
  // grid tab is already open, replace the map it currently shows. Grid-less maps
  // get their own single-map tab.
  const openMap = (
    mapName: string,
    stitchOffset?: GridNavigateStitchOffset
  ) => {
    if (!mapByName(mapName)) {
      return;
    }
    const gridName = gridNameForMap(mapName);
    const existingIndex = openTabs.findIndex((tab) =>
      gridName
        ? tab.gridName === gridName
        : tab.gridName === null && tab.activeMapName === mapName
    );

    if (existingIndex >= 0) {
      if (openTabs[existingIndex].activeMapName === mapName) {
        setActiveTabIndex(existingIndex, openTabs, stitchOffset);
      } else {
        // Switch to the grid tab and replace its map in one viewport hop.
        const previousTab =
          activeTabIndex !== null ? openTabs[activeTabIndex] : undefined;
        const nextTabs = openTabs.map((tab, i) =>
          i === existingIndex ? { ...tab, activeMapName: mapName } : tab
        );
        setOpenTabs(nextTabs);
        applyActiveMapChange(
          previousTab?.activeMapName ?? '',
          previousTab ? mapByName(previousTab.activeMapName) : null,
          mapName,
          stitchOffset
        );
        _setActiveTabIndex(existingIndex);
      }
      getEditorState().selectedMapName = mapName;
      return;
    }

    const newTabs: OpenTab[] = [...openTabs, { gridName, activeMapName: mapName }];
    setOpenTabs(newTabs);
    setActiveTabIndex(newTabs.length - 1, newTabs, stitchOffset);
    getEditorState().selectedMapName = mapName;
  };

  // Restore before paint so a direct refresh on #/editor/maps shows tabs immediately
  useLayoutEffect(() => {
    if (hasRestoredTabsRef.current || maps.length === 0) {
      return;
    }
    hasRestoredTabsRef.current = true;

    const { tabs: restoredTabs, activeTabIndex: restoredActiveIndex } =
      restoreMapTabsFromStorage(maps, mapGrids);

    if (restoredTabs.length > 0 && restoredActiveIndex !== null) {
      restoredTabs.forEach((tab) => {
        createEditorStateMapForTabIfNotExists(tab.activeMapName);
      });
      _setOpenTabs(restoredTabs);
      setActiveTabIndex(restoredActiveIndex, restoredTabs);
    }

    setTabsHydrated(true);
  }, [maps, mapGrids]);

  // Persist tab state after hydration (avoids overwriting storage before restore)
  useEffect(() => {
    if (!tabsHydrated) {
      return;
    }
    if (isFirstPersistRef.current) {
      isFirstPersistRef.current = false;
      if (
        openTabs.length === 0 &&
        (loadPersistedMapTabs()?.tabs.length ?? 0) > 0
      ) {
        return;
      }
    }
    persistOpenTabs(openTabs, activeTabIndex);
  }, [openTabs, activeTabIndex, tabsHydrated]);

  useEffect(() => {
    if (!tabsHydrated || maps.length === 0 || consumedMapParamRef.current) {
      return;
    }
    const mapName = routeParams?.get('map');
    if (!mapName) {
      return;
    }
    consumedMapParamRef.current = true;

    if (maps.findIndex((m) => m.name === mapName) < 0) {
      return;
    }

    openMap(mapName);

    if (typeof window !== 'undefined') {
      window.location.hash = '#/editor/maps';
    }
  }, [tabsHydrated, maps, mapGrids, routeParams, openTabs]);

  const showNotification = (message: string, type: 'success' | 'error') => {
    const id = notificationIdRef.current++;
    setNotifications((prev) => [...prev, { message, type, id }]);
  };

  const removeNotification = (id: number) => {
    setNotifications((prev) => prev.filter((n) => n.id !== id));
  };

  // Map options for OptionSelect
  const mapOptions = [
    { value: '', label: '-- Open a map --' },
    ...maps.map((map, index) => ({
      value: index.toString(),
      label: map.name,
    })),
  ];

  const handleMapSelect = (value: string) => {
    if (value === '') {
      return;
    }

    const mapIndex = parseInt(value, 10);
    if (isNaN(mapIndex) || mapIndex < 0 || mapIndex >= maps.length) {
      return;
    }

    // Opens the map in its grid's tab; if that grid tab is already open, the
    // chosen map replaces whatever it was showing.
    openMap(maps[mapIndex].name);
  };

  // Grid options for OptionSelect
  const gridOptions = [
    { value: '', label: '-- Open a grid --' },
    ...mapGrids.map((grid, index) => ({
      value: index.toString(),
      label: grid.name,
    })),
  ];

  const firstAssignedMapInGrid = (grid: MapGridTemplate): string | null => {
    for (const row of grid.cells) {
      for (const cell of row ?? []) {
        const name = cell?.trim();
        if (name && mapByName(name)) {
          return name;
        }
      }
    }
    return null;
  };

  const handleGridSelect = (value: string) => {
    if (value === '') {
      return;
    }
    const grid = mapGrids[parseInt(value, 10)];
    if (!grid) {
      return;
    }

    // Already open: just switch to that tab, leaving its current map alone.
    const existingIndex = openTabs.findIndex(
      (tab) => tab.gridName === grid.name
    );
    if (existingIndex >= 0) {
      setActiveTabIndex(existingIndex);
      return;
    }

    const firstMap = firstAssignedMapInGrid(grid);
    if (!firstMap) {
      showNotification(
        `Map grid "${grid.label || grid.name}" has no maps assigned yet.`,
        'error'
      );
      return;
    }
    openMap(firstMap);
  };

  // Callback to open a map tab and select a tile
  const handleOpenMapAndSelectTile = (args: OpenMapAndSelectTileArgs) => {
    const { mapName, markerName, pos, level } = args;
    const mapIndex = maps.findIndex((m) => m.name === mapName);
    if (mapIndex < 0) {
      return;
    }

    const mapData = maps[mapIndex];

    // Open the map in its grid's tab (or switch to it / replace its map).
    openMap(mapName);

    let location: { level: number; tileIndex: number } | null = null;

    if (markerName) {
      location = findMarkerOnMap(mapData, markerName);
    } else if (pos !== undefined) {
      const { x, y } = pos;
      const tileLevel = level ?? 0;
      if (x >= 0 && y >= 0 && x < mapData.width && y < mapData.height) {
        location = {
          level: tileLevel,
          tileIndex: y * mapData.width + x,
        };
      }
    }

    if (location) {
      setTimeout(() => {
        if (!getEditorState().maps[mapName]) {
          createEditorStateMap(mapName);
        }
        getEditorState().selectedMapName = mapName;
        locateOnCurrentMap(mapData, location!);
      }, 100);
    }
  };

  const handleCreateMap = (newMap: CarcerMapTemplate) => {
    const prepared = prepareNewMapForEditor(newMap);
    setMaps([...maps, prepared]);

    // A brand-new map has no grid yet, so it opens in its own single-map tab.
    const newTabs: OpenTab[] = [
      ...openTabs,
      { gridName: null, activeMapName: prepared.name },
    ];
    setOpenTabs(newTabs);
    setActiveTabIndex(newTabs.length - 1, newTabs);
    setCreateModalOpen(false);
    showNotification('Map created!', 'success');
  };

  // Canvas grid navigation: move within the active grid tab, keeping the
  // stitched world fixed under the camera.
  const handleNavigateToGridMap = (
    mapName: string,
    stitchOffset: GridNavigateStitchOffset
  ) => {
    if (maps.findIndex((m) => m.name === mapName) < 0) {
      return;
    }
    const activeTab =
      activeTabIndex !== null ? openTabs[activeTabIndex] : undefined;
    if (
      activeTab &&
      activeTab.gridName &&
      gridNameForMap(mapName) === activeTab.gridName
    ) {
      setTabActiveMap(activeTabIndex!, mapName, openTabs, stitchOffset);
      getEditorState().selectedMapName = mapName;
      return;
    }
    openMap(mapName, stitchOffset);
  };

  const handleCreateGridMap = (request: GridSlotCreateRequest) => {
    setGridCreateRequest(request);
  };

  const handleConfirmGridCreateMap = async (newMap: CarcerMapTemplate) => {
    if (!gridCreateRequest) {
      return;
    }

    const prepared = prepareNewMapForEditor(newMap);

    // Seed the new map with the grid's layer stack so the whole grid shares one.
    const grid = mapGrids.find((g) => g.name === gridCreateRequest.gridName);
    if (grid) {
      const mapsByName = Object.fromEntries(maps.map((m) => [m.name, m]));
      for (const layer of getGridLayerSet(grid, mapsByName)) {
        if (!prepared.layers.includes(layer)) {
          createTilesForLayer(prepared, layer);
        }
      }
    }

    const updatedMapGrids = sanitizeMapGridTemplates(
      trimStrings(
        assignMapToGridCell(
          mapGrids,
          gridCreateRequest.gridName,
          gridCreateRequest.cellX,
          gridCreateRequest.cellY,
          prepared.name
        )
      )
    );

    try {
      await saveMapGrids(updatedMapGrids);
    } catch (err) {
      showNotification(
        `Failed to save map grids: ${
          err instanceof Error ? err.message : 'Unknown error'
        }`,
        'error'
      );
      return;
    }

    setMaps([...maps, prepared]);
    setMapGrids(updatedMapGrids);

    // The map is now a cell of this grid: focus that grid's tab (opening it if
    // needed) and show the new map in it.
    const gridName = gridCreateRequest.gridName;
    createEditorStateMapForTabIfNotExists(prepared.name);
    const existingIndex = openTabs.findIndex((tab) => tab.gridName === gridName);
    if (existingIndex >= 0) {
      const nextTabs = setTabActiveMap(existingIndex, prepared.name, openTabs);
      setActiveTabIndex(existingIndex, nextTabs);
    } else {
      const nextTabs: OpenTab[] = [
        ...openTabs,
        { gridName, activeMapName: prepared.name },
      ];
      setOpenTabs(nextTabs);
      setActiveTabIndex(nextTabs.length - 1, nextTabs);
    }
    getEditorState().selectedMapName = prepared.name;
    setGridCreateRequest(null);
    showNotification('Map created and assigned to grid!', 'success');
  };

  const handleCloseTab = (tabIndex: number) => {
    const newTabs = openTabs.filter((_, index) => index !== tabIndex);
    setOpenTabs(newTabs);

    if (activeTabIndex === tabIndex) {
      // If we closed the active tab, switch to another one
      if (newTabs.length > 0) {
        // Switch to the tab that was after the closed one, or the last tab
        const newActiveIndex = Math.min(tabIndex, newTabs.length - 1);
        setActiveTabIndex(newActiveIndex);
      } else {
        setActiveTabIndex(null);
      }
    } else if (activeTabIndex !== null && activeTabIndex > tabIndex) {
      // Adjust active tab index if it was after the closed tab
      setActiveTabIndex(activeTabIndex - 1);
    }
  };

  const confirmDelete = () => {
    if (deleteConfirm.mapIndex !== null) {
      const mapIndex = deleteConfirm.mapIndex;
      const deletedName = maps[mapIndex]?.name;
      setMaps(maps.filter((_, index) => index !== mapIndex));

      // Close any tab whose current map was the one deleted.
      const newTabs = openTabs.filter(
        (tab) => tab.activeMapName !== deletedName
      );
      setOpenTabs(newTabs);

      if (activeTabIndex !== null) {
        if (newTabs.length === 0) {
          setActiveTabIndex(null);
        } else {
          setActiveTabIndex(
            Math.min(activeTabIndex, newTabs.length - 1),
            newTabs
          );
        }
      }

      if (selectedMapIndex === mapIndex.toString()) {
        setSelectedMapIndex('');
      }
    }
    setDeleteConfirm({ isOpen: false, mapIndex: null });
  };

  const updateMapInTabs = async (
    updatedMap: CarcerMapTemplate
  ): Promise<boolean> => {
    if (!activeTab) {
      return true;
    }

    const oldName = activeTab.activeMapName.trim();
    const newName = updatedMap.name.trim();
    const mapIndex = maps.findIndex((m) => m.name === activeTab.activeMapName);
    if (mapIndex < 0) {
      return true;
    }

    if (oldName && newName && oldName !== newName) {
      const updatedMapGrids = sanitizeMapGridTemplates(
        trimStrings(renameMapInGrids(mapGrids, oldName, newName))
      );

      if (findMapGridPlacement(oldName, mapGrids)) {
        try {
          await saveMapGrids(updatedMapGrids);
        } catch (err) {
          showNotification(
            `Failed to save map grids: ${
              err instanceof Error ? err.message : 'Unknown error'
            }`,
            'error'
          );
          return false;
        }
      }

      setMapGrids(updatedMapGrids);
      renameEditorStateMap(oldName, newName);
    }

    const updatedMaps = [...maps];
    updatedMaps[mapIndex] = updatedMap;
    setMaps(updatedMaps);

    if (oldName !== newName && newName) {
      const newTabs = openTabs.map((tab) =>
        tab.activeMapName === oldName
          ? { ...tab, activeMapName: newName }
          : tab
      );
      setOpenTabs(newTabs);
    }
    return true;
  };

  // Tile-data changes from the editor (paint strokes, undo, layer ops). Unlike
  // updateMapInTabs this targets whatever map identifies itself by name, so grid
  // neighbours edited in place are flushed to React state and saved by Ctrl+S.
  const handleMapDataChange = (updatedMap: CarcerMapTemplate) => {
    const idx = maps.findIndex((m) => m.name === updatedMap.name);
    if (idx < 0) {
      return;
    }
    const next = [...maps];
    next[idx] = updatedMap;
    setMaps(next);
  };

  const handleEditMap = async (updatedMap: CarcerMapTemplate) => {
    const saved = await updateMapInTabs(updatedMap);
    if (saved) {
      setEditModalOpen(false);
    }
  };

  const handleDuplicateMap = (sourceMap: CarcerMapTemplate) => {
    const existingNames = new Set(maps.map((m) => m.name));
    const copyName = getDuplicateMapName(sourceMap.name, existingNames);

    const duplicated: CarcerMapTemplate = JSON.parse(
      JSON.stringify(sourceMap)
    );
    duplicated.name = copyName;
    duplicated.label = sourceMap.label
      ? `${sourceMap.label} (Copy)`
      : copyName;

    const sourceIndex = maps.findIndex((m) => m.name === sourceMap.name);
    const insertIndex = (sourceIndex >= 0 ? sourceIndex : maps.length - 1) + 1;

    const newMaps = [...maps];
    newMaps.splice(insertIndex, 0, duplicated);
    setMaps(newMaps);

    // The copy is not assigned to any grid, so it opens in its own tab next to
    // the current one.
    const dupTab: OpenTab = { gridName: null, activeMapName: duplicated.name };
    createEditorStateMapForTabIfNotExists(duplicated.name);
    getEditorState().selectedMapName = duplicated.name;

    const newTabs = [...openTabs];
    let newActiveTabIndex: number;
    if (activeTabIndex !== null) {
      newTabs.splice(activeTabIndex + 1, 0, dupTab);
      newActiveTabIndex = activeTabIndex + 1;
    } else {
      newTabs.push(dupTab);
      newActiveTabIndex = newTabs.length - 1;
    }

    setOpenTabs(newTabs);
    setActiveTabIndex(newActiveTabIndex, newTabs);
    setEditModalOpen(true);
    showNotification('Map duplicated!', 'success');
  };

  const validateMaps = (): { isValid: boolean; error?: string } => {
    const errors: string[] = [];
    const nameCounts = new Map<string, number>();
    const mapsWithMissingFields: string[] = [];

    maps.forEach((map, index) => {
      const missingFields: string[] = [];

      // Check required string fields
      if (!map.name || map.name.trim() === '') {
        missingFields.push('name');
      }
      if (!map.label || map.label.trim() === '') {
        missingFields.push('label');
      }
      if (!map.type || !MAP_TYPES.includes(map.type)) {
        missingFields.push('type');
      }

      // Check required number fields
      if (map.width === undefined || map.width === null || map.width <= 0) {
        missingFields.push('width');
      }
      if (map.height === undefined || map.height === null || map.height <= 0) {
        missingFields.push('height');
      }

      if (missingFields.length > 0) {
        const mapIdentifier = map.name || `Map at index ${index}`;
        mapsWithMissingFields.push(
          `${mapIdentifier}: missing ${missingFields.join(', ')}`
        );
      }

      // Track names for duplicate checking
      if (map.name && map.name.trim()) {
        const count = nameCounts.get(map.name) || 0;
        nameCounts.set(map.name, count + 1);
      }
    });

    // Check for duplicate names
    const duplicateNames: string[] = [];
    nameCounts.forEach((count, name) => {
      if (count > 1) {
        duplicateNames.push(name);
      }
    });

    if (duplicateNames.length > 0) {
      errors.push(`Duplicate map names found: ${duplicateNames.join(', ')}`);
    }

    if (mapsWithMissingFields.length > 0) {
      errors.push(
        `Maps with missing required fields:\n${mapsWithMissingFields.join(
          '\n'
        )}`
      );
    }

    if (errors.length > 0) {
      return {
        isValid: false,
        error: errors.join('\n\n'),
      };
    }

    return { isValid: true };
  };

  const handleSaveAll = async () => {
    const validation = validateMaps();
    if (!validation.isValid) {
      showNotification(validation.error || 'Validation failed', 'error');
      return;
    }

    const trimmedMaps = trimStrings(maps);
    // const sortedMaps = trimmedMaps.sort((a, b) => {
    //   return a.name.localeCompare(b.name);
    // });

    try {
      await saveMaps(trimmedMaps);
      setMaps(trimmedMaps);
      showNotification('Maps saved successfully!', 'success');

      // Update tab references after sorting
      // const newTabs = openTabs.map((tab) => {
      //   const sortedIndex = sortedMaps.findIndex(
      //     (map) => map.name === tab.map.name
      //   );
      //   if (sortedIndex >= 0) {
      //     return {
      //       mapIndex: sortedIndex,
      //       map: sortedMaps[sortedIndex],
      //     };
      //   }
      //   return tab;
      // });
      // setOpenTabs(newTabs);
    } catch (err) {
      showNotification(
        `Error saving: ${err instanceof Error ? err.message : 'Unknown error'}`,
        'error'
      );
    }
  };

  // Global hotkey: Ctrl+S to save
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      // Check for Ctrl+S (Windows/Linux) or Cmd+S (Mac)
      if ((e.ctrlKey || e.metaKey) && e.key === 's') {
        e.preventDefault();
        e.stopPropagation();
        handleSaveAll();
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => {
      window.removeEventListener('keydown', handleKeyDown);
    };
  }, [maps]);

  const activeTab =
    activeTabIndex !== null ? openTabs[activeTabIndex] ?? null : null;
  const activeMap = activeTab
    ? maps.find((m) => m.name === activeTab.activeMapName) ?? null
    : null;

  const gridCreateConstraints: CreateMapConstraints | undefined = (() => {
    if (!gridCreateRequest) {
      return undefined;
    }
    const base: CreateMapConstraints = {
      width: gridCreateRequest.mapWidth,
      height: gridCreateRequest.mapHeight,
      lockDimensions: true,
    };
    if (!activeMap) {
      return base;
    }
    const defaultName = getDuplicateMapName(
      activeMap.name,
      new Set(maps.map((m) => m.name))
    );
    return {
      ...base,
      defaultName,
      defaultLabel: activeMap.label ? `${activeMap.label} (Copy)` : defaultName,
    };
  })();

  return (
    <div className="container editor-page">
      <EditorHeader
        title="Maps Editor"
        onSave={handleSaveAll}
        actions={
          <>
            <OptionSelect
              className="editor-header-control"
              id="map-select"
              name="mapSelect"
              label=""
              value={selectedMapIndex}
              onChange={handleMapSelect}
              options={mapOptions}
            />
            {mapGrids.length > 0 && (
              <OptionSelect
                className="editor-header-control"
                id="grid-select"
                name="gridSelect"
                label=""
                value={selectedGridIndex}
                onChange={handleGridSelect}
                options={gridOptions}
              />
            )}
            <Button variant="primary" onClick={() => setCreateModalOpen(true)}>
              + New Map
            </Button>
          </>
        }
      />

      {openTabs.length > 0 && (
        <div className="editor-tab-bar">
          {openTabs.map((tab, index) => {
            const isActive = activeTabIndex === index;
            const hasGrid = Boolean(tab.gridName);
            return (
              <div
                key={tabKey(tab)}
                style={{
                  display: 'flex',
                  flexDirection: 'column',
                  padding: '6px 12px',
                  borderRight: '1px solid #3e3e42',
                  backgroundColor: isActive ? '#252526' : '#1e1e1e',
                  cursor: 'pointer',
                  minWidth: '170px',
                  maxWidth: '260px',
                }}
                onClick={() => setActiveTabIndex(index)}
              >
                <div style={{ display: 'flex', alignItems: 'center' }}>
                  <span
                    style={{
                      color: hasGrid
                        ? isActive
                          ? '#c586c0'
                          : '#7a5c78'
                        : isActive
                        ? '#9a9a9a'
                        : '#5a5a5a',
                      fontStyle: hasGrid ? 'normal' : 'italic',
                      fontSize: '11px',
                      textTransform: 'uppercase',
                      letterSpacing: '0.04em',
                      flex: 1,
                      overflow: 'hidden',
                      textOverflow: 'ellipsis',
                      whiteSpace: 'nowrap',
                      marginRight: '8px',
                    }}
                    title={
                      hasGrid
                        ? `Map grid: ${tab.gridName}`
                        : 'Not in a map grid'
                    }
                  >
                    {tab.gridName ?? '(no grid)'}
                  </span>
                  <button
                    onClick={(e) => {
                      e.stopPropagation();
                      setActiveTabIndex(index);
                      setEditModalOpen(true);
                    }}
                    style={{
                      background: 'none',
                      border: 'none',
                      color: '#858585',
                      cursor: 'pointer',
                      fontSize: '14px',
                      padding: '0 4px',
                      lineHeight: '1',
                      marginRight: '4px',
                    }}
                    title="Edit Map Properties"
                    onMouseEnter={(e) => {
                      e.currentTarget.style.color = '#4ec9b0';
                    }}
                    onMouseLeave={(e) => {
                      e.currentTarget.style.color = '#858585';
                    }}
                  >
                    ✎
                  </button>
                  <button
                    onClick={(e) => {
                      e.stopPropagation();
                      handleCloseTab(index);
                    }}
                    style={{
                      background: 'none',
                      border: 'none',
                      color: '#858585',
                      cursor: 'pointer',
                      fontSize: '18px',
                      padding: '0 4px',
                      lineHeight: '1',
                    }}
                    title="Close Tab"
                    onMouseEnter={(e) => {
                      e.currentTarget.style.color = '#f48771';
                    }}
                    onMouseLeave={(e) => {
                      e.currentTarget.style.color = '#858585';
                    }}
                  >
                    ×
                  </button>
                </div>
                <span
                  style={{
                    color: isActive ? '#4ec9b0' : '#858585',
                    overflow: 'hidden',
                    textOverflow: 'ellipsis',
                    whiteSpace: 'nowrap',
                    marginTop: '2px',
                  }}
                  title={tab.activeMapName}
                >
                  ▸ {tab.activeMapName}
                </span>
              </div>
            );
          })}
        </div>
      )}

      <div className="editor-page-body">
        <TileEditor
          map={activeMap ?? undefined}
          onMapUpdate={handleMapDataChange}
          onOpenMapAndSelectTile={(args: OpenMapAndSelectTileArgs) => {
            handleOpenMapAndSelectTile(args);
          }}
          onNavigateToGridMap={handleNavigateToGridMap}
          onCreateGridMap={handleCreateGridMap}
        />
      </div>

      {notifications.map((notification) => (
        <Notification
          key={notification.id}
          message={notification.message}
          type={notification.type}
          onClose={() => removeNotification(notification.id)}
        />
      ))}

      <DeleteModal
        isOpen={deleteConfirm.isOpen}
        message="Are you sure you want to delete this map?"
        onConfirm={confirmDelete}
        onCancel={() => setDeleteConfirm({ isOpen: false, mapIndex: null })}
      />

      <CreateMapModal
        isOpen={createModalOpen}
        onConfirm={handleCreateMap}
        onCancel={() => setCreateModalOpen(false)}
      />

      <CreateMapModal
        isOpen={gridCreateRequest !== null}
        onConfirm={handleConfirmGridCreateMap}
        onCancel={() => setGridCreateRequest(null)}
        constraints={gridCreateConstraints}
        existingMapNames={maps.map((m) => m.name)}
      />

      <EditMapModal
        isOpen={editModalOpen}
        map={activeMap}
        onConfirm={handleEditMap}
        onCancel={() => setEditModalOpen(false)}
        onDelete={() => {
          if (activeTab) {
            const mapIndex = maps.findIndex(
              (m) => m.name === activeTab.activeMapName
            );
            if (mapIndex >= 0) {
              setDeleteConfirm({ isOpen: true, mapIndex });
            }
          }
        }}
        onDuplicate={handleDuplicateMap}
      />
    </div>
  );
}
