import { useState, useRef } from 'react';
import { AssetTree, scrollAssetTreeItemIntoView } from '../components/AssetTree';
import { EditorSidebar } from '../components/EditorSidebar';
import { AbilityTemplate } from '../types/ability';
import { AbilityTemplateForm } from '../components/AbilityTemplateForm';
import {
  createDefaultAbilityTemplate,
  createDefaultMeleeAbilityTemplate,
  createDefaultSpellAbilityTemplate,
} from '../types/ability';
import { validateSpellAbilityRefs } from '../types/spell';
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
import { useSaveHotkey } from '../hooks/useSaveHotkey';
import { useAssetOrganization } from '../contexts/OrganizationContext';

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
  duration?: number;
}

interface AbilityTemplatesProps {
  routeParams?: URLSearchParams;
}

export function AbilityTemplates({ routeParams }: AbilityTemplatesProps = {}) {
  const {
    abilities,
    setAbilities,
    saveAbilities,
    spells,
    items,
    setItems,
    statusEffects,
    setStatusEffects,
  } = useAssets();
  const [editIndex, setEditIndex] = useState<number>(-1);
  const [searchTerm, setSearchTerm] = useState('');
  const [notifications, setNotifications] = useState<NotificationState[]>([]);
  const notificationIdRef = useRef(0);
  const org = useAssetOrganization('abilityTemplates');
  const [abilityDeleteConfirm, setAbilityDeleteConfirm] = useState<{
    actualIndex: number;
    abilityName: string;
    abilityLabel: string;
    impacts: AbilityDeleteImpact[];
  } | null>(null);

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

  const filtered = abilities.filter(
    (ability) =>
      ability.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
      ability.label.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const handleClick = (abilityId: string) => {
    const actualIndex = abilities.findIndex((ability) => ability.name === abilityId);
    if (actualIndex >= 0) {
      setEditIndex(actualIndex);
    }
  };

  const handleClone = (abilityId: string) => {
    const actualIndex = abilities.findIndex((ability) => ability.name === abilityId);
    if (actualIndex < 0) {
      return;
    }
    const cloned: AbilityTemplate = JSON.parse(JSON.stringify(abilities[actualIndex]));
    cloned.name = cloned.name + '_copy';
    org.copyPlacement(abilityId, cloned.name);
    const next = abilities.slice();
    next.splice(actualIndex + 1, 0, cloned);
    setAbilities(next);
    setEditIndex(actualIndex + 1);
    showNotification('Ability cloned!', 'success');
    scrollAssetTreeItemIntoView(cloned.name);
  };

  const handleDelete = (abilityId: string) => {
    const actualIndex = abilities.findIndex((ability) => ability.name === abilityId);
    if (actualIndex < 0) {
      return;
    }
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
    if (ability?.name) {
      org.removeItem(ability.name);
    }

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

  const appendAbility = (ability: AbilityTemplate) => {
    const selectedId = editIndex >= 0 ? abilities[editIndex]?.name ?? null : null;
    org.fileNewItem(ability.name, selectedId);
    const next = [...abilities, ability];
    setAbilities(next);
    setEditIndex(next.length - 1);
    setSearchTerm('');
    scrollAssetTreeItemIntoView(ability.name);
  };

  const handleCreateNewAbility = () => {
    appendAbility(createDefaultAbilityTemplate());
  };

  const handleCreateNewSpell = () => {
    appendAbility(createDefaultSpellAbilityTemplate());
  };

  const handleCreateNewMelee = () => {
    appendAbility(createDefaultMeleeAbilityTemplate());
  };

  const updateAbility = (ability: AbilityTemplate) => {
    if (editIndex < 0) return;
    org.rekeyItem(abilities[editIndex].name, ability.name);
    const updated = [...abilities];
    updated[editIndex] = ability;
    setAbilities(updated);
  };

  const handleSaveAll = async () => {
    const currentName = editIndex >= 0 ? abilities[editIndex]?.name : undefined;
    const sorted = trimStrings(abilities).sort((a, b) => a.name.localeCompare(b.name));
    const abilityRefErrors = validateSpellAbilityRefs(
      spells,
      sorted.map((ability) => ability.name),
    );
    try {
      const saved = await saveAbilities(sorted);
      setAbilities(saved);
      if (abilityRefErrors.length > 0) {
        showNotification(
          `Abilities saved, but spell ability reference errors:\n${abilityRefErrors.join('\n')}`,
          'error',
          8000,
        );
      } else {
        showNotification('Abilities saved successfully!', 'success');
      }
      if (currentName) {
        setEditIndex(saved.findIndex((a) => a.name === currentName.trim()));
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

  useSaveHotkey(handleSaveAll);

  const current = editIndex >= 0 ? abilities[editIndex] : undefined;

  return (
    <div className="container editor-page">
      <EditorHeader
        title="Ability Templates Editor"
        onSave={handleSaveAll}
        preserveAsset="abilityTemplates"
      />

      <div className="editor-page-body">
        <div className="editor-content">
        <EditorSidebar
          searchTerm={searchTerm}
          onSearchChange={setSearchTerm}
          searchPlaceholder="Search abilities..."
          createMenuLabel="+ New"
          createMenuItems={[
            { label: '+ New Ability', onClick: handleCreateNewAbility },
            { label: '+ New Melee', onClick: handleCreateNewMelee },
            { label: '+ New Spell', onClick: handleCreateNewSpell },
          ]}
        >
          <AssetTree
            assetId="abilityTemplates"
            items={filtered.map((ability) => ({
              id: ability.name,
              label: ability.label || ability.name,
              subtitle: ability.name,
            }))}
            allItemIds={abilities.map((ability) => ability.name)}
            selectedId={editIndex >= 0 ? abilities[editIndex]?.name : null}
            onSelectItem={handleClick}
            onClone={handleClone}
            onDelete={handleDelete}
            filterActive={searchTerm.trim() !== ''}
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
          duration={notification.duration}
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
