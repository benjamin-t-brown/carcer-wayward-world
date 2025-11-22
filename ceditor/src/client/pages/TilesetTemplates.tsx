import { useState, useEffect, useRef } from 'react';
import { CardList } from '../components/CardList';
import { TilesetTemplate } from '../types/assets';
import {
  TilesetTemplateForm,
  createDefaultTileset,
} from '../components/TilesetTemplateForm';
import { Button } from '../elements/Button';
import { Notification } from '../elements/Notification';
import { useAssets } from '../contexts/AssetsContext';
import { trimStrings } from '../utils/jsonUtils';

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
}

interface TilesetTemplatesProps {
  routeParams?: URLSearchParams;
}

export function TilesetTemplates({ routeParams }: TilesetTemplatesProps = {}) {
  const { tilesets, setTilesets, saveTilesets } = useAssets();
  const [editTilesetIndex, setEditTilesetIndex] = useState<number>(-1);
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

  // Filter tilesets based on search term
  const filteredTilesets = tilesets.filter(
    (tileset) =>
      tileset.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
      tileset.spriteBase.toLowerCase().includes(searchTerm.toLowerCase())
  );

  // Get the actual index in the full tilesets array for filtered tilesets
  const getActualIndex = (filteredIndex: number): number => {
    const filteredTileset = filteredTilesets[filteredIndex];
    return tilesets.indexOf(filteredTileset);
  };

  const scrollToTopOfForm = () => {
    setTimeout(() => {
      document
        .getElementById('tileset-form')
        ?.scrollIntoView({ behavior: 'smooth' });
    }, 100);
  };

  const handleTilesetClick = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    setEditTilesetIndex(actualIndex);
    scrollToTopOfForm();
  };

  const handleClone = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    const originalTileset = tilesets[actualIndex];
    const clonedTileset: TilesetTemplate = JSON.parse(
      JSON.stringify(originalTileset)
    );
    clonedTileset.name = clonedTileset.name + '_copy';
    const newTilesets = tilesets.slice();
    const clonedIndex = actualIndex + 1;
    newTilesets.splice(clonedIndex, 0, clonedTileset);
    setTilesets(newTilesets);
    setEditTilesetIndex(clonedIndex);
    showNotification('Tileset cloned!', 'success');
    scrollToTopOfForm();
    setTimeout(() => {
      const tilesetCard = document.getElementById(`tileset-card-${clonedIndex}`);
      if (tilesetCard) {
        tilesetCard.scrollIntoView({ behavior: 'smooth' });
      }
    }, 100);
  };

  const handleDelete = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    if (confirm('Are you sure you want to delete this tileset?')) {
      const newTilesets = tilesets.filter((_, index) => index !== actualIndex);
      setTilesets(newTilesets);
      if (editTilesetIndex === actualIndex) {
        setEditTilesetIndex(-1);
      } else if (editTilesetIndex > actualIndex) {
        setEditTilesetIndex(editTilesetIndex - 1);
      }
    }
  };

  const handleCreateNew = () => {
    const newTilesetTemplate = createDefaultTileset();
    const newTilesets = [...tilesets, newTilesetTemplate];
    setTilesets(newTilesets);
    const actualIndex = newTilesets.length - 1;
    setEditTilesetIndex(actualIndex);
    scrollToTopOfForm();
    setSearchTerm('');
    setTimeout(() => {
      const tilesetCard = document.getElementById(`tileset-card-${actualIndex}`);
      if (tilesetCard) {
        tilesetCard.scrollIntoView({ behavior: 'smooth' });
      }
    }, 100);
  };

  // Check for tileset query parameter on mount
  useEffect(() => {
    if (routeParams) {
      const tilesetName = routeParams.get('tileset');
      if (tilesetName) {
        const index = tilesets.findIndex((t) => t.name === tilesetName);
        if (index >= 0) {
          setEditTilesetIndex(index);
          scrollToTopOfForm();
        }
      }
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [tilesets, routeParams]);

  const updateTileset = (tileset: TilesetTemplate) => {
    if (editTilesetIndex >= 0) {
      const currentTilesetIndex = getActualIndex(editTilesetIndex);
      const currentTileset = tilesets[currentTilesetIndex];
      if (currentTileset) {
        // Update existing tileset in real-time
        const updatedTilesets = [...tilesets];
        updatedTilesets[currentTilesetIndex] = tileset;
        setTilesets(updatedTilesets);
      }
    }
  };

  const validateTilesets = (): { isValid: boolean; error?: string } => {
    const errors: string[] = [];
    const nameCounts = new Map<string, number>();
    const tilesetsWithMissingFields: string[] = [];

    tilesets.forEach((tileset, index) => {
      const missingFields: string[] = [];

      // Check required string fields
      if (!tileset.name || tileset.name.trim() === '') {
        missingFields.push('name');
      }
      if (!tileset.spriteBase || tileset.spriteBase.trim() === '') {
        missingFields.push('spriteBase');
      }

      // Check required number fields
      if (tileset.tileWidth === undefined || tileset.tileWidth === null || tileset.tileWidth <= 0) {
        missingFields.push('tileWidth');
      }
      if (tileset.tileHeight === undefined || tileset.tileHeight === null || tileset.tileHeight <= 0) {
        missingFields.push('tileHeight');
      }

      if (missingFields.length > 0) {
        const tilesetIdentifier = tileset.name || `Tileset at index ${index}`;
        tilesetsWithMissingFields.push(
          `${tilesetIdentifier}: missing ${missingFields.join(', ')}`
        );
      }

      // Track names for duplicate checking
      if (tileset.name && tileset.name.trim()) {
        const count = nameCounts.get(tileset.name) || 0;
        nameCounts.set(tileset.name, count + 1);
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
      errors.push(`Duplicate tileset names found: ${duplicateNames.join(', ')}`);
    }

    if (tilesetsWithMissingFields.length > 0) {
      errors.push(
        `Tilesets with missing required fields:\n${tilesetsWithMissingFields.join('\n')}`
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
    const validation = validateTilesets();
    if (!validation.isValid) {
      showNotification(validation.error || 'Validation failed', 'error');
      return;
    }

    const currentTilesetIndex = editTilesetIndex >= 0 ? getActualIndex(editTilesetIndex) : -1;
    const currentTilesetName = currentTilesetIndex >= 0 ? tilesets[currentTilesetIndex]?.name : undefined;

    const trimmedTilesets = trimStrings(tilesets);

    const sortedTilesets = trimmedTilesets.sort((a, b) => {
      return a.name.localeCompare(b.name);
    });

    try {
      await saveTilesets(sortedTilesets);
      showNotification('Tilesets saved successfully!', 'success');
      if (currentTilesetName) {
        const nextTilesetIndex = sortedTilesets.findIndex(
          (tileset) => tileset.name === currentTilesetName.trim()
        );
        setEditTilesetIndex(nextTilesetIndex);
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
  }, [tilesets]); // Include dependencies

  const currentTileset = tilesets[editTilesetIndex];

  return (
    <div className="container">
      <div className="editor-header">
        <Button variant="back" onClick={() => window.history.back()}>
          ← Back
        </Button>
        <h1>Tileset Templates Editor</h1>
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
              placeholder="Search tilesets..."
            />
          </div>
          <div className="item-actions">
            <Button variant="primary" onClick={handleCreateNew}>
              + New Tileset
            </Button>
          </div>
          <CardList
              items={filteredTilesets}
              onItemClick={handleTilesetClick}
              onClone={handleClone}
              onDelete={handleDelete}
              selectedIndex={
                editTilesetIndex !== -1
                  ? (() => {
                      const index = filteredTilesets.findIndex(
                        (tileset) => tilesets.indexOf(tileset) === editTilesetIndex
                      );
                      return index >= 0 ? index : null;
                    })()
                  : null
              }
              renderAdditionalInfo={(tileset) => {
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
                      <span className="item-type">
                        {tileset.tileWidth}×{tileset.tileHeight}
                      </span>
                    </div>
                    <div
                      style={{
                        fontSize: '12px',
                        color: '#858585',
                        marginTop: '4px',
                      }}
                    >
                      {tileset.spriteBase}
                    </div>
                    {tileset.tiles.length > 0 && (
                      <div
                        style={{
                          fontSize: '12px',
                          color: '#858585',
                          marginTop: '4px',
                        }}
                      >
                        {tileset.tiles.length} tiles
                      </div>
                    )}
                  </>
                );
              }}
              emptyMessage="No tilesets found"
            />
        </div>

        <div className="editor-main">
          <div id="tileset-form">
            <TilesetTemplateForm
              tileset={currentTileset}
              updateTileset={updateTileset}
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

