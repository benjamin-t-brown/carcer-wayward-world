import { useState, useRef } from 'react';
import { AssetTree, scrollAssetTreeItemIntoView } from '../components/AssetTree';
import { EditorSidebar } from '../components/EditorSidebar';
import {
  AbilityDeleteImpact,
  CharacterTemplate,
  applyCharacterDeleteToMaps,
  planCharacterDeleteImpacts,
} from '../types/assets';
import { AbilityDeleteConfirmModal } from '../components/AbilityDeleteConfirmModal';
import {
  CharacterTemplateForm,
  createDefaultCharacter,
} from '../components/CharacterTemplateForm';
import { EditorHeader } from '../components/EditorHeader';
import { Notification } from '../elements/Notification';
import { Sprite } from '../elements/Sprite';
import { useAssets } from '../contexts/AssetsContext';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { trimStrings } from '../utils/jsonUtils';
import { usePersistedEditorSelection } from '../hooks/usePersistedEditorSelection';
import { useSaveHotkey } from '../hooks/useSaveHotkey';
import { useAssetOrganization } from '../contexts/OrganizationContext';

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
}

interface CharacterTemplatesProps {
  routeParams?: URLSearchParams;
}

function isEnemyType(type: string) {
  return type === 'ENEMY' || type === 'ENEMY_STATIC';
}

function isTownspersonType(type: string) {
  return type === 'TOWNSPERSON' || type === 'TOWNSPERSON_STATIC';
}

function CharacterTypeIcon({ type }: { type: string }) {
  const enemy = isEnemyType(type);
  return (
    <span
      className="character-type-icon"
      style={{ background: enemy ? '#a1260d' : '#4a7c59' }}
      title={enemy ? 'Enemy' : 'Townsperson'}
      aria-label={enemy ? 'Enemy' : 'Townsperson'}
    >
      {enemy ? 'E' : 'T'}
    </span>
  );
}

export function CharacterTemplates({ routeParams }: CharacterTemplatesProps = {}) {
  const { spriteMap } = useSDL2WAssets();
  const { characters, setCharacters, saveCharacters, maps, setMaps } = useAssets();
  const [editCharacterIndex, setEditCharacterIndex] = useState<number>(-1);
  const [characterDeleteConfirm, setCharacterDeleteConfirm] = useState<{
    actualIndex: number;
    characterName: string;
    characterLabel: string;
    impacts: AbilityDeleteImpact[];
  } | null>(null);
  const [searchTerm, setSearchTerm] = useState('');
  const [showEnemies, setShowEnemies] = useState(true);
  const [showTownspersons, setShowTownspersons] = useState(true);
  const [notifications, setNotifications] = useState<NotificationState[]>([]);
  const notificationIdRef = useRef(0);
  const org = useAssetOrganization('characterTemplates');

  const showNotification = (message: string, type: 'success' | 'error') => {
    const id = notificationIdRef.current++;
    setNotifications((prev) => [...prev, { message, type, id }]);
  };

  const removeNotification = (id: number) => {
    setNotifications((prev) => prev.filter((n) => n.id !== id));
  };

  // Filter characters based on search term and type filters
  const lowerSearch = searchTerm.toLowerCase();
  const filteredCharacters = characters.filter((character) => {
    const matchesType =
      (showEnemies && isEnemyType(character.type)) ||
      (showTownspersons && isTownspersonType(character.type));
    if (!matchesType) {
      return false;
    }
    return (
      character.name.toLowerCase().includes(lowerSearch) ||
      character.label.toLowerCase().includes(lowerSearch) ||
      character.type.toLowerCase().includes(lowerSearch)
    );
  });
  const characterTypeById = new Map(
    characters.map((character) => [character.name, character.type]),
  );

  const scrollToTopOfForm = () => {
    setTimeout(() => {
      document
        .getElementById('character-form')
        ?.scrollIntoView({ behavior: 'smooth' });
    }, 100);
  };

  const handleCharacterClick = (characterId: string) => {
    const actualIndex = characters.findIndex(
      (character) => character.name === characterId,
    );
    if (actualIndex < 0) {
      return;
    }
    setEditCharacterIndex(actualIndex);
    scrollToTopOfForm();
  };

  const handleClone = (characterId: string) => {
    const actualIndex = characters.findIndex(
      (character) => character.name === characterId,
    );
    if (actualIndex < 0) {
      return;
    }
    const originalCharacter = characters[actualIndex];
    const clonedCharacter: CharacterTemplate = JSON.parse(
      JSON.stringify(originalCharacter)
    );
    clonedCharacter.name = clonedCharacter.name + '_copy';
    org.copyPlacement(characterId, clonedCharacter.name);
    const newCharacters = characters.slice();
    const clonedIndex = actualIndex + 1;
    newCharacters.splice(clonedIndex, 0, clonedCharacter);
    setCharacters(newCharacters);
    setEditCharacterIndex(clonedIndex);
    showNotification('Character cloned!', 'success');
    scrollToTopOfForm();
    scrollAssetTreeItemIntoView(clonedCharacter.name);
  };

  const confirmDeleteCharacter = (actualIndex: number) => {
    const character = characters[actualIndex];
    const characterName = character?.name.trim() ?? '';
    if (character?.name) {
      org.removeItem(character.name);
    }

    const newCharacters = characters.filter((_, index) => index !== actualIndex);
    setCharacters(newCharacters);

    if (characterName) {
      setMaps(applyCharacterDeleteToMaps(characterName, maps));
    }

    if (editCharacterIndex === actualIndex) {
      setEditCharacterIndex(-1);
    } else if (editCharacterIndex > actualIndex) {
      setEditCharacterIndex(editCharacterIndex - 1);
    }

    setCharacterDeleteConfirm(null);
  };

  const handleDelete = (characterId: string) => {
    const actualIndex = characters.findIndex(
      (character) => character.name === characterId,
    );
    if (actualIndex < 0) {
      return;
    }
    const character = characters[actualIndex];
    if (!character) {
      return;
    }

    const characterName = character.name.trim();
    if (!characterName) {
      confirmDeleteCharacter(actualIndex);
      return;
    }

    const impacts = planCharacterDeleteImpacts(characterName, maps);
    if (impacts.length === 0) {
      if (confirm('Are you sure you want to delete this character?')) {
        confirmDeleteCharacter(actualIndex);
      }
      return;
    }

    setCharacterDeleteConfirm({
      actualIndex,
      characterName,
      characterLabel: character.label || characterName,
      impacts,
    });
  };

  usePersistedEditorSelection({
    editorKey: 'characterTemplates',
    items: characters,
    getId: (character) => character.name,
    selectedIndex: editCharacterIndex,
    setSelectedIndex: setEditCharacterIndex,
    routeParams,
    onRestored: () => scrollToTopOfForm(),
  });

  const handleCreateNew = () => {
    const newCharacterTemplate = createDefaultCharacter();
    const selectedId =
      editCharacterIndex >= 0 ? characters[editCharacterIndex]?.name ?? null : null;
    org.fileNewItem(newCharacterTemplate.name, selectedId);
    const newCharacters = [...characters, newCharacterTemplate];
    setCharacters(newCharacters);
    const actualIndex = newCharacters.length - 1;
    setEditCharacterIndex(actualIndex);
    scrollToTopOfForm();
    setSearchTerm('');
    scrollAssetTreeItemIntoView(newCharacterTemplate.name);
  };

  const updateCharacter = (character: CharacterTemplate) => {
    if (editCharacterIndex < 0) {
      return;
    }
    org.rekeyItem(characters[editCharacterIndex].name, character.name);
    const updatedCharacters = [...characters];
    updatedCharacters[editCharacterIndex] = character;
    setCharacters(updatedCharacters);
  };

  const validateCharacters = (): { isValid: boolean; error?: string } => {
    const errors: string[] = [];
    const nameCounts = new Map<string, number>();
    const charactersWithMissingFields: string[] = [];

    characters.forEach((character, index) => {
      const missingFields: string[] = [];

      // Check required string fields
      if (!character.type || character.type.trim() === '') {
        missingFields.push('type');
      }
      if (!character.name || character.name.trim() === '') {
        missingFields.push('name');
      }
      if (!character.label || character.label.trim() === '') {
        missingFields.push('label');
      }
      if (!character.spritesheet || character.spritesheet.trim() === '') {
        missingFields.push('spritesheet');
      }
      if (character.spriteOffset === undefined || character.spriteOffset === null) {
        missingFields.push('spriteOffset');
      }

      if (missingFields.length > 0) {
        const characterIdentifier = character.name || `Character at index ${index}`;
        charactersWithMissingFields.push(
          `${characterIdentifier}: missing ${missingFields.join(', ')}`
        );
      }

      // Track names for duplicate checking
      if (character.name && character.name.trim()) {
        const count = nameCounts.get(character.name) || 0;
        nameCounts.set(character.name, count + 1);
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
      errors.push(
        `Duplicate character names found: ${duplicateNames.join(', ')}`
      );
    }

    if (charactersWithMissingFields.length > 0) {
      errors.push(
        `Characters with missing required fields:\n${charactersWithMissingFields.join('\n')}`
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
    const validation = validateCharacters();
    if (!validation.isValid) {
      showNotification(validation.error || 'Validation failed', 'error');
      return;
    }

    const currentCharacterName =
      editCharacterIndex >= 0 ? characters[editCharacterIndex]?.name : undefined;

    const trimmedCharacters = trimStrings(characters);

    const sortedCharacters = trimmedCharacters.sort((a, b) => {
      const cmp = a.name.localeCompare(b.name);
      return cmp === 0 ? a.label.localeCompare(b.label) : cmp;
    });

    try {
      const savedCharacters = await saveCharacters(sortedCharacters);
      setCharacters(savedCharacters);
      showNotification('Characters saved successfully!', 'success');
      if (currentCharacterName) {
        const nextCharacterIndex = savedCharacters.findIndex(
          (character) => character.name === currentCharacterName.trim()
        );
        setEditCharacterIndex(nextCharacterIndex >= 0 ? nextCharacterIndex : -1);
      }
    } catch (err) {
      showNotification(
        `Error saving: ${err instanceof Error ? err.message : 'Unknown error'}`,
        'error'
      );
    }
  };

  useSaveHotkey(handleSaveAll);

  const currentCharacter = characters[editCharacterIndex];

  return (
    <div className="container editor-page">
      <EditorHeader
        title="Character Templates Editor"
        onSave={handleSaveAll}
        preserveAsset="characterTemplates"
      />

      <div className="editor-page-body">
        <div className="editor-content">
        <EditorSidebar
          searchTerm={searchTerm}
          onSearchChange={setSearchTerm}
          searchPlaceholder="Search characters..."
          createLabel="+ New Character"
          onCreate={handleCreateNew}
          afterSearch={
            <div className="event-type-filters">
              <input
                id="show-enemy-characters"
                type="checkbox"
                checked={showEnemies}
                onChange={(e) => setShowEnemies(e.target.checked)}
              />
              <label
                className="event-type-filter-label"
                htmlFor="show-enemy-characters"
              >
                Enemy
              </label>
              <input
                id="show-townsperson-characters"
                type="checkbox"
                checked={showTownspersons}
                onChange={(e) => setShowTownspersons(e.target.checked)}
              />
              <label
                className="event-type-filter-label"
                htmlFor="show-townsperson-characters"
              >
                Townsperson
              </label>
            </div>
          }
        >
          <AssetTree
            assetId="characterTemplates"
            items={filteredCharacters.map((character) => {
              const spriteName = `${character.spritesheet}_${character.spriteOffset}`;
              const sprite = spriteMap[spriteName];
              const label = character.label || character.name;
              return {
                id: character.name,
                label: `${label} (${character.name})`,
                icon: <CharacterTypeIcon type={character.type} />,
                extra: sprite ? (
                  <div className="item-info character-tree-sprite">
                    <Sprite sprite={sprite} displaySize={24} />
                  </div>
                ) : undefined,
              };
            })}
            allItemIds={characters.map((character) => character.name)}
            selectedId={
              editCharacterIndex >= 0
                ? characters[editCharacterIndex]?.name
                : null
            }
            onSelectItem={handleCharacterClick}
            onClone={handleClone}
            onDelete={handleDelete}
            filterActive={
              searchTerm.trim() !== '' || !showEnemies || !showTownspersons
            }
            emptyMessage="No characters found"
            compareItems={(a, b) => {
              const typeA = characterTypeById.get(a.id) ?? '';
              const typeB = characterTypeById.get(b.id) ?? '';
              const rankA = isEnemyType(typeA) ? 0 : 1;
              const rankB = isEnemyType(typeB) ? 0 : 1;
              return rankA - rankB || a.label.localeCompare(b.label) || a.id.localeCompare(b.id);
            }}
          />
        </EditorSidebar>

        <div className="editor-main">
          <div id="character-form">
            <CharacterTemplateForm
              character={currentCharacter}
              updateCharacter={updateCharacter}
            />
          </div>
        </div>
        </div>
      </div>

      <AbilityDeleteConfirmModal
        isOpen={characterDeleteConfirm !== null}
        title="Delete character?"
        description={
          characterDeleteConfirm ? (
            <>
              Deleting{' '}
              <strong style={{ color: '#e0e0e0' }}>
                {characterDeleteConfirm.characterLabel}
              </strong>{' '}
              <span style={{ color: '#858585' }}>
                ({characterDeleteConfirm.characterName})
              </span>{' '}
              will also update{' '}
              {characterDeleteConfirm.impacts.length === 1
                ? '1 map'
                : `${characterDeleteConfirm.impacts.length} maps`}
              :
            </>
          ) : null
        }
        impacts={characterDeleteConfirm?.impacts ?? []}
        confirmLabel="Delete character & update maps"
        onConfirm={() => {
          if (characterDeleteConfirm) {
            confirmDeleteCharacter(characterDeleteConfirm.actualIndex);
          }
        }}
        onCancel={() => setCharacterDeleteConfirm(null)}
      />

      {/* Notifications */}
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

