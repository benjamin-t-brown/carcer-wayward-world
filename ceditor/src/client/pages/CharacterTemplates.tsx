import { useState, useEffect, useRef } from 'react';
import { CardList } from '../components/CardList';
import {
  CharacterTemplateForm,
  CharacterTemplate,
  createDefaultCharacter,
} from '../components/CharacterTemplateForm';
import { Button } from '../elements/Button';
import { Notification } from '../elements/Notification';
import { useAssets } from '../contexts/AssetsContext';
import { trimStrings } from '../utils/jsonUtils';

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
}

export function CharacterTemplates() {
  const { characters, setCharacters, saveCharacters } = useAssets();
  const [editCharacterIndex, setEditCharacterIndex] = useState<number>(-1);
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

  // Filter characters based on search term
  const filteredCharacters = characters.filter(
    (character) =>
      character.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
      character.label.toLowerCase().includes(searchTerm.toLowerCase()) ||
      character.type.toLowerCase().includes(searchTerm.toLowerCase())
  );

  // Get the actual index in the full characters array for filtered characters
  const getActualIndex = (filteredIndex: number): number => {
    const filteredCharacter = filteredCharacters[filteredIndex];
    return characters.indexOf(filteredCharacter);
  };

  const scrollToTopOfForm = () => {
    setTimeout(() => {
      document
        .getElementById('character-form')
        ?.scrollIntoView({ behavior: 'smooth' });
    }, 100);
  };

  const handleCharacterClick = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    setEditCharacterIndex(actualIndex);
    scrollToTopOfForm();
  };

  const handleClone = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    const originalCharacter = characters[actualIndex];
    const clonedCharacter: CharacterTemplate = JSON.parse(
      JSON.stringify(originalCharacter)
    );
    clonedCharacter.name = clonedCharacter.name + '_copy';
    const newCharacters = characters.slice();
    const clonedIndex = actualIndex + 1;
    newCharacters.splice(clonedIndex, 0, clonedCharacter);
    setCharacters(newCharacters);
    setEditCharacterIndex(clonedIndex);
    showNotification('Character cloned!', 'success');
    scrollToTopOfForm();
    setTimeout(() => {
      const characterCard = document.getElementById(
        `character-card-${clonedIndex}`
      );
      if (characterCard) {
        characterCard.scrollIntoView({ behavior: 'smooth' });
      }
    }, 100);
  };

  const handleDelete = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    if (confirm('Are you sure you want to delete this character?')) {
      const newCharacters = characters.filter(
        (_, index) => index !== actualIndex
      );
      setCharacters(newCharacters);
      if (editCharacterIndex === actualIndex) {
        setEditCharacterIndex(-1);
      } else if (editCharacterIndex > actualIndex) {
        setEditCharacterIndex(editCharacterIndex - 1);
      }
    }
  };

  const handleCreateNew = () => {
    const newCharacterTemplate = createDefaultCharacter();
    const newCharacters = [...characters, newCharacterTemplate];
    setCharacters(newCharacters);
    const actualIndex = newCharacters.length - 1;
    setEditCharacterIndex(actualIndex);
    scrollToTopOfForm();
    setSearchTerm('');
    setTimeout(() => {
      const characterCard = document.getElementById(
        `character-card-${actualIndex}`
      );
      if (characterCard) {
        characterCard.scrollIntoView({ behavior: 'smooth' });
      }
    }, 100);
  };

  const updateCharacter = (character: CharacterTemplate) => {
    const currentCharacterIndex = getActualIndex(editCharacterIndex);
    const currentCharacter = characters[currentCharacterIndex];
    if (currentCharacter) {
      // Update existing character in real-time
      const updatedCharacters = [...characters];
      updatedCharacters[currentCharacterIndex] = character;
      setCharacters(updatedCharacters);
    }
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
      if (!character.spritesheetName || character.spritesheetName.trim() === '') {
        missingFields.push('spritesheetName');
      }
      if (!character.spriteOffset || character.spriteOffset.trim() === '') {
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

    const currentCharacterIndex = editCharacterIndex >= 0 ? getActualIndex(editCharacterIndex) : -1;
    const currentCharacterName = currentCharacterIndex >= 0 ? characters[currentCharacterIndex]?.name : undefined;

    const trimmedCharacters = trimStrings(characters);

    const sortedCharacters = trimmedCharacters.sort((a, b) => {
      const cmp = a.name.localeCompare(b.name);
      return cmp === 0 ? a.label.localeCompare(b.label) : cmp;
    });

    try {
      await saveCharacters(sortedCharacters);
      setCharacters(sortedCharacters);
      showNotification('Characters saved successfully!', 'success');
      if (currentCharacterName) {
        const nextCharacterIndex = sortedCharacters.findIndex(
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

  // Global hotkey: Ctrl+S to save
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      // Check for Ctrl+S (Windows/Linux) or Cmd+S (Mac)
      if ((e.ctrlKey || e.metaKey) && e.key === 's') {
        e.preventDefault();
        handleSaveAll();
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => {
      window.removeEventListener('keydown', handleKeyDown);
    };
  }, [characters]); // Include dependencies

  const currentCharacter = characters[editCharacterIndex];

  return (
    <div className="container">
      <div className="editor-header">
        <Button variant="back" onClick={() => window.history.back()}>
          ← Back
        </Button>
        <h1>Character Templates Editor</h1>
        <Button variant="primary" onClick={handleSaveAll}>
          Save All
        </Button>
      </div>

      <div className="editor-content">
        <div className="editor-sidebar">
          <div className="search-box">
            <input
              type="text"
              value={searchTerm}
              onChange={(e) => setSearchTerm(e.target.value)}
              placeholder="Search characters..."
            />
          </div>
          <div className="item-actions">
            <Button variant="primary" onClick={handleCreateNew}>
              + New Character
            </Button>
          </div>
          <CardList
              items={filteredCharacters}
              onItemClick={handleCharacterClick}
              onClone={handleClone}
              onDelete={handleDelete}
              selectedIndex={
                editCharacterIndex !== -1
                  ? (() => {
                      const index = filteredCharacters.findIndex(
                        (character) =>
                          characters.indexOf(character) === editCharacterIndex
                      );
                      return index >= 0 ? index : null;
                    })()
                  : null
              }
              renderAdditionalInfo={(character) => {
                return (
                  <>
                    <div
                      className="item-info"
                      style={{
                        display: 'flex',
                        alignItems: 'center',
                        gap: '8px',
                      }}
                    >
                      <span className="item-type">{character.type}</span>
                    </div>
                    <div
                      style={{
                        fontSize: '12px',
                        color: '#858585',
                        marginTop: '4px',
                      }}
                    >
                      {character.spritesheetName} @ {character.spriteOffset}
                    </div>
                  </>
                );
              }}
              emptyMessage="No characters found"
            />
          )
        </div>

        <div className="editor-main">
          <div id="character-form">
            <CharacterTemplateForm
              character={currentCharacter}
              updateCharacter={updateCharacter}
            />
          </div>
        </div>
      </div>

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

