import { useState, useRef, useEffect } from 'react';
import { CardList } from '../components/CardList';
import { EditorSidebar } from '../components/EditorSidebar';
import { StatusEffectTemplate } from '../types/combat';
import {
  StatusEffectTemplateForm,
  createDefaultStatusEffectTemplate,
} from '../components/StatusEffectTemplateForm';
import { EditorHeader } from '../components/EditorHeader';
import { Notification } from '../elements/Notification';
import { useAssets } from '../contexts/AssetsContext';
import { trimStrings } from '../utils/jsonUtils';

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
}

export function StatusEffectTemplates() {
  const { statusEffects, setStatusEffects, saveStatusEffects } = useAssets();
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

  const filtered = statusEffects.filter(
    (status) =>
      status.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
      status.description.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const getActualIndex = (filteredIndex: number) =>
    statusEffects.indexOf(filtered[filteredIndex]);

  const handleClick = (filteredIndex: number) => setEditIndex(getActualIndex(filteredIndex));

  const handleClone = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    const cloned: StatusEffectTemplate = JSON.parse(
      JSON.stringify(statusEffects[actualIndex])
    );
    cloned.name = cloned.name + '_copy';
    const next = statusEffects.slice();
    next.splice(actualIndex + 1, 0, cloned);
    setStatusEffects(next);
    setEditIndex(actualIndex + 1);
    showNotification('Status effect cloned!', 'success');
  };

  const handleDelete = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    if (confirm('Delete this status effect?')) {
      const next = statusEffects.filter((_, i) => i !== actualIndex);
      setStatusEffects(next);
      if (editIndex === actualIndex) setEditIndex(-1);
      else if (editIndex > actualIndex) setEditIndex(editIndex - 1);
    }
  };

  const handleCreateNew = () => {
    const next = [...statusEffects, createDefaultStatusEffectTemplate()];
    setStatusEffects(next);
    setEditIndex(next.length - 1);
    setSearchTerm('');
  };

  const updateStatusEffect = (status: StatusEffectTemplate) => {
    if (editIndex < 0) return;
    const next = [...statusEffects];
    next[editIndex] = status;
    setStatusEffects(next);
  };

  const handleSaveAll = async () => {
    const currentName = editIndex >= 0 ? statusEffects[editIndex]?.name : undefined;
    const sorted = trimStrings(statusEffects).sort((a, b) => a.name.localeCompare(b.name));
    try {
      await saveStatusEffects(sorted);
      setStatusEffects(sorted);
      showNotification('Status effects saved successfully!', 'success');
      if (currentName) {
        setEditIndex(sorted.findIndex((s) => s.name === currentName.trim()));
      }
    } catch (error) {
      showNotification(
        error instanceof Error ? error.message : 'Failed to save status effects',
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
  }, [statusEffects]);

  const current = editIndex >= 0 ? statusEffects[editIndex] : undefined;

  return (
    <div className="container editor-page">
      <EditorHeader title="Status Effect Templates Editor" onSave={handleSaveAll} />

      <div className="editor-page-body">
        <div className="editor-content">
        <EditorSidebar
          searchTerm={searchTerm}
          onSearchChange={setSearchTerm}
          searchPlaceholder="Search status effects..."
          createLabel="+ New Status Effect"
          onCreate={handleCreateNew}
        >
          <CardList
            items={filtered.map((s) => ({
              name: s.name,
              label: s.name,
            }))}
            onItemClick={handleClick}
            onClone={handleClone}
            onDelete={handleDelete}
            selectedIndex={
              editIndex !== -1
                ? filtered.findIndex((s) => statusEffects.indexOf(s) === editIndex)
                : null
            }
            emptyMessage="No status effects found"
          />
        </EditorSidebar>

        <div className="editor-main">
          <StatusEffectTemplateForm
            statusEffect={current}
            updateStatusEffect={updateStatusEffect}
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
