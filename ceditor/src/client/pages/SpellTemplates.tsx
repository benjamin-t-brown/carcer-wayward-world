import { useState, useRef, useEffect } from 'react';
import { CardList } from '../components/CardList';
import { EditorSidebar } from '../components/EditorSidebar';
import {
  SpellTemplate,
  validateSpellAbilityRefs,
} from '../types/spell';
import {
  SpellTemplateForm,
  createDefaultSpellTemplate,
} from '../components/SpellTemplateForm';
import { EditorHeader } from '../components/EditorHeader';
import { Notification } from '../elements/Notification';
import { useAssets } from '../contexts/AssetsContext';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { Sprite } from '../elements/Sprite';
import { trimStrings } from '../utils/jsonUtils';
import { usePersistedEditorSelection } from '../hooks/usePersistedEditorSelection';

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
  duration?: number;
}

interface SpellTemplatesProps {
  routeParams?: URLSearchParams;
}

export function SpellTemplates({ routeParams }: SpellTemplatesProps = {}) {
  const { spells, setSpells, saveSpells, abilities } = useAssets();
  const { spriteMap } = useSDL2WAssets();
  const [editIndex, setEditIndex] = useState<number>(-1);
  const [searchTerm, setSearchTerm] = useState('');
  const [notifications, setNotifications] = useState<NotificationState[]>([]);
  const notificationIdRef = useRef(0);

  const showNotification = (
    message: string,
    type: 'success' | 'error',
    duration?: number,
  ) => {
    const id = notificationIdRef.current++;
    setNotifications((prev) => [...prev, { message, type, id, duration }]);
  };

  const removeNotification = (id: number) => {
    setNotifications((prev) => prev.filter((n) => n.id !== id));
  };

  const filtered = spells.filter(
    (spell) =>
      spell.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
      spell.label.toLowerCase().includes(searchTerm.toLowerCase()),
  );

  const getActualIndex = (filteredIndex: number) =>
    spells.indexOf(filtered[filteredIndex]);

  const handleClick = (filteredIndex: number) =>
    setEditIndex(getActualIndex(filteredIndex));

  const handleClone = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    const cloned: SpellTemplate = JSON.parse(
      JSON.stringify(spells[actualIndex]),
    );
    cloned.name = cloned.name + '_copy';
    const next = spells.slice();
    next.splice(actualIndex + 1, 0, cloned);
    setSpells(next);
    setEditIndex(actualIndex + 1);
    showNotification('Spell cloned!', 'success');
  };

  const handleDelete = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    if (confirm('Delete this spell?')) {
      const next = spells.filter((_, i) => i !== actualIndex);
      setSpells(next);
      if (editIndex === actualIndex) setEditIndex(-1);
      else if (editIndex > actualIndex) setEditIndex(editIndex - 1);
    }
  };

  const handleCreateNew = () => {
    const next = [...spells, createDefaultSpellTemplate()];
    setSpells(next);
    setEditIndex(next.length - 1);
    setSearchTerm('');
  };

  const updateSpell = (spell: SpellTemplate) => {
    if (editIndex < 0) return;
    const updated = [...spells];
    updated[editIndex] = spell;
    setSpells(updated);
  };

  const handleSaveAll = async () => {
    const currentName = editIndex >= 0 ? spells[editIndex]?.name : undefined;
    const sorted = trimStrings(spells).sort((a, b) =>
      a.name.localeCompare(b.name),
    );
    const abilityRefErrors = validateSpellAbilityRefs(
      sorted,
      abilities.map((ability) => ability.name),
    );
    try {
      await saveSpells(sorted);
      setSpells(sorted);
      if (abilityRefErrors.length > 0) {
        showNotification(
          `Spells saved, but ability reference errors:\n${abilityRefErrors.join('\n')}`,
          'error',
          8000,
        );
      } else {
        showNotification('Spells saved successfully!', 'success');
      }
      if (currentName) {
        setEditIndex(sorted.findIndex((s) => s.name === currentName.trim()));
      }
    } catch (error) {
      showNotification(
        error instanceof Error ? error.message : 'Failed to save spells',
        'error',
      );
    }
  };

  usePersistedEditorSelection({
    editorKey: 'spellTemplates',
    items: spells,
    getId: (spell) => spell.name,
    selectedIndex: editIndex,
    setSelectedIndex: setEditIndex,
    routeParams,
  });

  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if ((e.ctrlKey || e.metaKey) && e.key === 's') {
        e.preventDefault();
        handleSaveAll();
      }
    };
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [spells]);

  const current = editIndex >= 0 ? spells[editIndex] : undefined;

  return (
    <div className="container editor-page">
      <EditorHeader title="Spell Templates Editor" onSave={handleSaveAll} />

      <div className="editor-page-body">
        <div className="editor-content">
          <EditorSidebar
            searchTerm={searchTerm}
            onSearchChange={setSearchTerm}
            searchPlaceholder="Search spells..."
            createLabel="+ New Spell"
            onCreate={handleCreateNew}
          >
            <CardList
              items={filtered.map((s) => ({
                name: s.name,
                label: s.label || s.name,
                icon: s.icon,
              }))}
              onItemClick={handleClick}
              onClone={handleClone}
              onDelete={handleDelete}
              selectedIndex={
                editIndex !== -1
                  ? filtered.findIndex((s) => spells.indexOf(s) === editIndex)
                  : null
              }
              renderAdditionalInfo={(item) => {
                const sprite = item.icon ? spriteMap[item.icon] : undefined;
                if (!sprite) {
                  return null;
                }
                return (
                  <div
                    className="item-info"
                    style={{
                      display: 'flex',
                      alignItems: 'center',
                      gap: '8px',
                    }}
                  >
                    <div style={{ display: 'inline-block' }}>
                      <Sprite sprite={sprite} scale={1.5} />
                    </div>
                  </div>
                );
              }}
              emptyMessage="No spells found"
            />
          </EditorSidebar>

          <div className="editor-main">
            <SpellTemplateForm spell={current} updateSpell={updateSpell} />
          </div>
        </div>
      </div>

      {notifications.map((notification) => (
        <Notification
          key={notification.id}
          message={notification.message}
          type={notification.type}
          duration={notification.duration}
          onClose={() => removeNotification(notification.id)}
        />
      ))}
    </div>
  );
}
