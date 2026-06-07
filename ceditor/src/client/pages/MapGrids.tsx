import { useState, useRef, useEffect } from 'react';
import { CardList } from '../components/CardList';
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

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
}

export function MapGrids() {
  const { mapGrids, setMapGrids, saveMapGrids } = useAssets();
  const [editIndex, setEditIndex] = useState<number>(-1);
  const [searchTerm, setSearchTerm] = useState('');
  const [notifications, setNotifications] = useState<NotificationState[]>([]);
  const notificationIdRef = useRef(0);

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

  const getActualIndex = (filteredIndex: number) =>
    mapGrids.indexOf(filtered[filteredIndex]);

  const handleClick = (filteredIndex: number) => setEditIndex(getActualIndex(filteredIndex));

  usePersistedEditorSelection({
    editorKey: 'mapGrids',
    items: mapGrids,
    getId: (grid) => grid.name,
    selectedIndex: editIndex,
    setSelectedIndex: setEditIndex,
  });

  const handleClone = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    const cloned: MapGridTemplate = JSON.parse(JSON.stringify(mapGrids[actualIndex]));
    cloned.name = cloned.name + '_copy';
    const next = mapGrids.slice();
    next.splice(actualIndex + 1, 0, cloned);
    setMapGrids(next);
    setEditIndex(actualIndex + 1);
    showNotification('Map grid cloned!', 'success');
  };

  const handleDelete = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    const grid = mapGrids[actualIndex];
    const label = grid?.label || grid?.name || 'this map grid';
    if (confirm(`Delete "${label}"?`)) {
      const next = mapGrids.filter((_, i) => i !== actualIndex);
      setMapGrids(next);
      if (editIndex === actualIndex) setEditIndex(-1);
      else if (editIndex > actualIndex) setEditIndex(editIndex - 1);
    }
  };

  const handleCreateNew = () => {
    const next = [...mapGrids, createDefaultMapGridTemplate()];
    setMapGrids(next);
    setEditIndex(next.length - 1);
    setSearchTerm('');
  };

  const updateMapGrid = (grid: MapGridTemplate) => {
    if (editIndex < 0) return;
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
      await saveMapGrids(sorted);
      setMapGrids(sorted);
      showNotification('Map grids saved successfully!', 'success');
      if (currentName) {
        setEditIndex(sorted.findIndex((g) => g.name === currentName.trim()));
      }
    } catch (error) {
      showNotification(
        error instanceof Error ? error.message : 'Failed to save map grids',
        'error'
      );
    }
  };

  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if ((e.ctrlKey || e.metaKey) && e.key === 's') {
        e.preventDefault();
        handleSaveAll();
      }
    };
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [mapGrids]);

  const current = editIndex >= 0 ? mapGrids[editIndex] : undefined;

  return (
    <div className="container editor-page">
      <EditorHeader title="Map Grids Editor" onSave={handleSaveAll} />

      <div className="editor-page-body">
        <div className="editor-content">
          <EditorSidebar
            searchTerm={searchTerm}
            onSearchChange={setSearchTerm}
            searchPlaceholder="Search map grids..."
            createLabel="+ New Map Grid"
            onCreate={handleCreateNew}
          >
            <CardList
              items={filtered.map((g) => ({
                name: g.name,
                label: g.label || g.name,
              }))}
              onItemClick={handleClick}
              onClone={handleClone}
              onDelete={handleDelete}
              selectedIndex={
                editIndex !== -1
                  ? filtered.findIndex((g) => mapGrids.indexOf(g) === editIndex)
                  : null
              }
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
