import { useState, useRef, useEffect } from 'react';
import { CardList } from '../components/CardList';
import { EditorSidebar } from '../components/EditorSidebar';
import { AbilityTemplate } from '../types/combat';
import {
  AbilityTemplateForm,
  createDefaultAbilityTemplate,
} from '../components/AbilityTemplateForm';
import { EditorHeader } from '../components/EditorHeader';
import { Notification } from '../elements/Notification';
import { useAssets } from '../contexts/AssetsContext';
import { trimStrings } from '../utils/jsonUtils';
import {
  applyAbilityDeleteToItems,
  applyAbilityDeleteToStatusEffects,
  planAbilityDeleteImpacts,
  AbilityDeleteImpact,
} from '../types/assets';
import { AbilityDeleteConfirmModal } from '../components/AbilityDeleteConfirmModal';
import { usePersistedEditorSelection } from '../hooks/usePersistedEditorSelection';

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
}

interface AbilityTemplatesProps {
  routeParams?: URLSearchParams;
}

export function AbilityTemplates({ routeParams }: AbilityTemplatesProps = {}) {
  const { abilities, setAbilities, saveAbilities, items, setItems, statusEffects, setStatusEffects } =
    useAssets();
  const [editIndex, setEditIndex] = useState<number>(-1);
  const [searchTerm, setSearchTerm] = useState('');
  const [notifications, setNotifications] = useState<NotificationState[]>([]);
  const notificationIdRef = useRef(0);
  const [abilityDeleteConfirm, setAbilityDeleteConfirm] = useState<{
    actualIndex: number;
    abilityName: string;
    abilityLabel: string;
    impacts: AbilityDeleteImpact[];
  } | null>(null);

  const showNotification = (message: string, type: 'success' | 'error') => {
    const id = notificationIdRef.current++;
    setNotifications((prev) => [...prev, { message, type, id }]);
  };

  const removeNotification = (id: number) => {
    setNotifications((prev) => prev.filter((n) => n.id !== id));
  };

  const filtered = abilities.filter(
    (ability) =>
      ability.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
      ability.label.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const getActualIndex = (filteredIndex: number) =>
    abilities.indexOf(filtered[filteredIndex]);

  const handleClick = (filteredIndex: number) => setEditIndex(getActualIndex(filteredIndex));

  const handleClone = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    const cloned: AbilityTemplate = JSON.parse(JSON.stringify(abilities[actualIndex]));
    cloned.name = cloned.name + '_copy';
    const next = abilities.slice();
    next.splice(actualIndex + 1, 0, cloned);
    setAbilities(next);
    setEditIndex(actualIndex + 1);
    showNotification('Ability cloned!', 'success');
  };

  const handleDelete = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    const ability = abilities[actualIndex];
    if (!ability) {
      return;
    }

    const abilityName = ability.name.trim();
    if (!abilityName) {
      confirmDeleteAbility(actualIndex);
      return;
    }

    const impacts = planAbilityDeleteImpacts(abilityName, items, statusEffects);
    if (impacts.length === 0) {
      confirmDeleteAbility(actualIndex);
      return;
    }

    setAbilityDeleteConfirm({
      actualIndex,
      abilityName,
      abilityLabel: ability.label || abilityName,
      impacts,
    });
  };

  const confirmDeleteAbility = (actualIndex: number) => {
    const ability = abilities[actualIndex];
    const abilityName = ability?.name.trim() ?? '';

    const next = abilities.filter((_, i) => i !== actualIndex);
    setAbilities(next);

    if (abilityName) {
      setItems(applyAbilityDeleteToItems(abilityName, items));
      setStatusEffects(
        applyAbilityDeleteToStatusEffects(abilityName, statusEffects),
      );
    }

    if (editIndex === actualIndex) setEditIndex(-1);
    else if (editIndex > actualIndex) setEditIndex(editIndex - 1);

    setAbilityDeleteConfirm(null);
  };

  const handleCreateNew = () => {
    const next = [...abilities, createDefaultAbilityTemplate()];
    setAbilities(next);
    setEditIndex(next.length - 1);
    setSearchTerm('');
  };

  const updateAbility = (ability: AbilityTemplate) => {
    if (editIndex < 0) return;
    const updated = [...abilities];
    updated[editIndex] = ability;
    setAbilities(updated);
  };

  const handleSaveAll = async () => {
    const currentName = editIndex >= 0 ? abilities[editIndex]?.name : undefined;
    const sorted = trimStrings(abilities).sort((a, b) => a.name.localeCompare(b.name));
    try {
      await saveAbilities(sorted);
      setAbilities(sorted);
      showNotification('Abilities saved successfully!', 'success');
      if (currentName) {
        setEditIndex(sorted.findIndex((a) => a.name === currentName.trim()));
      }
    } catch (error) {
      showNotification(
        error instanceof Error ? error.message : 'Failed to save abilities',
        'error'
      );
    }
  };

  usePersistedEditorSelection({
    editorKey: 'abilityTemplates',
    items: abilities,
    getId: (ability) => ability.name,
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
  }, [abilities]);

  const current = editIndex >= 0 ? abilities[editIndex] : undefined;

  return (
    <div className="container editor-page">
      <EditorHeader title="Ability Templates Editor" onSave={handleSaveAll} />

      <div className="editor-page-body">
        <div className="editor-content">
        <EditorSidebar
          searchTerm={searchTerm}
          onSearchChange={setSearchTerm}
          searchPlaceholder="Search abilities..."
          createLabel="+ New Ability"
          onCreate={handleCreateNew}
        >
          <CardList
            items={filtered.map((a) => ({
              name: a.name,
              label: a.label || a.name,
            }))}
            onItemClick={handleClick}
            onClone={handleClone}
            onDelete={handleDelete}
            selectedIndex={
              editIndex !== -1
                ? filtered.findIndex((a) => abilities.indexOf(a) === editIndex)
                : null
            }
            emptyMessage="No abilities found"
          />
        </EditorSidebar>

        <div className="editor-main">
          <AbilityTemplateForm ability={current} updateAbility={updateAbility} />
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

      <AbilityDeleteConfirmModal
        isOpen={abilityDeleteConfirm !== null}
        title="Delete ability?"
        description={
          abilityDeleteConfirm ? (
            <>
              This will permanently delete ability{' '}
              <strong style={{ color: '#e0e0e0' }}>
                {abilityDeleteConfirm.abilityLabel}
              </strong>{' '}
              <span style={{ color: '#858585' }}>
                ({abilityDeleteConfirm.abilityName})
              </span>{' '}
              and update{' '}
              {abilityDeleteConfirm.impacts.length === 1
                ? '1 asset'
                : `${abilityDeleteConfirm.impacts.length} assets`}
              :
            </>
          ) : null
        }
        impacts={abilityDeleteConfirm?.impacts ?? []}
        confirmLabel="Delete ability & update assets"
        onConfirm={() => {
          if (abilityDeleteConfirm) {
            confirmDeleteAbility(abilityDeleteConfirm.actualIndex);
          }
        }}
        onCancel={() => setAbilityDeleteConfirm(null)}
      />
    </div>
  );
}
