import { useState, useRef } from 'react';
import { AssetTree } from '../components/AssetTree';
import { EditorSidebar } from '../components/EditorSidebar';
import {
  MapGridTemplate,
  sanitizeMapGridTemplates,
} from '../types/assets';
import {
  MapGridTemplateForm,
  createDefaultMapGridTemplate,
} from '../components/MapGridTemplateForm';
import { EditorHeader } from '../components/EditorHeader';
import { Notification } from '../elements/Notification';
import { useAssets } from '../contexts/AssetsContext';
import { trimStrings } from '../utils/jsonUtils';
import { usePersistedEditorSelection } from '../hooks/usePersistedEditorSelection';
import { useSaveHotkey } from '../hooks/useSaveHotkey';
import { useAssetOrganization } from '../contexts/OrganizationContext';

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
}

export function MapGrids({
  routeParams,
}: {
  routeParams?: URLSearchParams;
}) {
  const { mapGrids, setMapGrids, saveMapGrids } = useAssets();
  const [editIndex, setEditIndex] = useState<number>(-1);
  const [searchTerm, setSearchTerm] = useState('');
  const [notifications, setNotifications] = useState<NotificationState[]>([]);
  const notificationIdRef = useRef(0);
  const org = useAssetOrganization('mapGrids');

  const showNotification = (message: string, type: 'success' | 'error') => {
    const id = notificationIdRef.current++;
    setNotifications((prev) => [...prev, { message, type, id }]);
  };

  const removeNotification = (id: number) => {
    setNotifications((prev) => prev.filter((n) => n.id !== id));
  };

  const filtered = mapGrids.filter(
    (grid) =>
      grid.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
      grid.label.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const handleClick = (gridId: string) => {
    const actualIndex = mapGrids.findIndex((grid) => grid.name === gridId);
    if (actualIndex >= 0) {
      setEditIndex(actualIndex);
    }
  };

  usePersistedEditorSelection({
    editorKey: 'mapGrids',
    items: mapGrids,
    getId: (grid) => grid.name,
    selectedIndex: editIndex,
    setSelectedIndex: setEditIndex,
    routeParams,
  });

  const handleClone = (gridId: string) => {
    const actualIndex = mapGrids.findIndex((grid) => grid.name === gridId);
    if (actualIndex < 0) {
      return;
    }
    const cloned: MapGridTemplate = JSON.parse(JSON.stringify(mapGrids[actualIndex]));
    cloned.name = cloned.name + '_copy';
    org.copyPlacement(gridId, cloned.name);
    const next = mapGrids.slice();
    next.splice(actualIndex + 1, 0, cloned);
    setMapGrids(next);
    setEditIndex(actualIndex + 1);
    showNotification('Map grid cloned!', 'success');
  };

  const handleDelete = (gridId: string) => {
    const actualIndex = mapGrids.findIndex((grid) => grid.name === gridId);
    if (actualIndex < 0) {
      return;
    }
    const grid = mapGrids[actualIndex];
    const label = grid?.label || grid?.name || 'this map grid';
    if (confirm(`Delete "${label}"?`)) {
      org.removeItem(gridId);
      const next = mapGrids.filter((_, i) => i !== actualIndex);
      setMapGrids(next);
      if (editIndex === actualIndex) setEditIndex(-1);
      else if (editIndex > actualIndex) setEditIndex(editIndex - 1);
    }
  };

  const handleCreateNew = () => {
    const created = createDefaultMapGridTemplate();
    const selectedId = editIndex >= 0 ? mapGrids[editIndex]?.name ?? null : null;
    org.fileNewItem(created.name, selectedId);
    const next = [...mapGrids, created];
    setMapGrids(next);
    setEditIndex(next.length - 1);
    setSearchTerm('');
  };

  const updateMapGrid = (grid: MapGridTemplate) => {
    if (editIndex < 0) return;
    org.rekeyItem(mapGrids[editIndex].name, grid.name);
    const next = [...mapGrids];
    next[editIndex] = grid;
    setMapGrids(next);
  };

  const validateBeforeSave = (grids: MapGridTemplate[]): string | null => {
    const names = grids.map((g) => g.name.trim());
    if (names.some((name) => !name)) {
      return 'Every map grid must have a name.';
    }
    const unique = new Set(names);
    if (unique.size !== names.length) {
      return 'Map grid names must be unique.';
    }
    return null;
  };

  const handleSaveAll = async () => {
    const currentName = editIndex >= 0 ? mapGrids[editIndex]?.name : undefined;
    const normalized = sanitizeMapGridTemplates(trimStrings(mapGrids));
    const validationError = validateBeforeSave(normalized);
    if (validationError) {
      showNotification(validationError, 'error');
      return;
    }
    const sorted = normalized.sort((a, b) => a.name.localeCompare(b.name));
    try {
      const saved = await saveMapGrids(sorted);
      setMapGrids(saved);
      showNotification('Map grids saved successfully!', 'success');
      if (currentName) {
        setEditIndex(saved.findIndex((g) => g.name === currentName.trim()));
      }
    } catch (error) {
      showNotification(
        error instanceof Error ? error.message : 'Failed to save map grids',
        'error'
      );
    }
  };

  useSaveHotkey(handleSaveAll);

  const current = editIndex >= 0 ? mapGrids[editIndex] : undefined;

  return (
    <div className="container editor-page">
      <EditorHeader
        title="Map Grids Editor"
        onSave={handleSaveAll}
        preserveAsset="mapGrids"
      />

      <div className="editor-page-body">
        <div className="editor-content">
          <EditorSidebar
            searchTerm={searchTerm}
            onSearchChange={setSearchTerm}
            searchPlaceholder="Search map grids..."
            createLabel="+ New Map Grid"
            onCreate={handleCreateNew}
          >
            <AssetTree
              assetId="mapGrids"
              items={filtered.map((grid) => ({
                id: grid.name,
                label: grid.label || grid.name,
                subtitle: grid.name,
              }))}
              allItemIds={mapGrids.map((grid) => grid.name)}
              selectedId={editIndex >= 0 ? mapGrids[editIndex]?.name : null}
              onSelectItem={handleClick}
              onClone={handleClone}
              onDelete={handleDelete}
              filterActive={searchTerm.trim() !== ''}
              emptyMessage="No map grids found"
            />
          </EditorSidebar>

          <div className="editor-main">
            <MapGridTemplateForm
              mapGrid={current}
              updateMapGrid={updateMapGrid}
            />
          </div>
        </div>
      </div>

      {notifications.map((notification) => (
        <Notification
          key={notification.id}
          message={notification.message}
          type={notification.type}
          onClose={() => removeNotification(notification.id)}
        />
      ))}
    </div>
  );
}
