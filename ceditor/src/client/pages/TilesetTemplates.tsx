import { useState, useRef } from 'react';
import { AssetTree, scrollAssetTreeItemIntoView } from '../components/AssetTree';
import { EditorSidebar } from '../components/EditorSidebar';
import { TilesetTemplate } from '../types/assets';
import {
  TilesetTemplateForm,
  createDefaultTileset,
} from '../components/TilesetTemplateForm';
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

interface TilesetTemplatesProps {
  routeParams?: URLSearchParams;
}

export function TilesetTemplates({ routeParams }: TilesetTemplatesProps = {}) {
  const { tilesets, setTilesets, saveTilesets } = useAssets();
  const [editTilesetIndex, setEditTilesetIndex] = useState<number>(-1);
  const [searchTerm, setSearchTerm] = useState('');
  const [notifications, setNotifications] = useState<NotificationState[]>([]);
  const notificationIdRef = useRef(0);
  const [showForm, setShowForm] = useState(true);
  const org = useAssetOrganization('tilesetTemplates');

  const showNotification = (message: string, type: 'success' | 'error') => {
    const id = notificationIdRef.current++;
    setNotifications((prev) => [...prev, { message, type, id }]);
  };

  const removeNotification = (id: number) => {
    setNotifications((prev) => prev.filter((n) => n.id !== id));
  };

  // console.log('render tileset templates', tilesets, editTilesetIndex);

  // Filter tilesets based on search term
  const filteredTilesets = tilesets.filter(
    (tileset) =>
      tileset.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
      tileset.spriteBase.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const scrollToTopOfForm = () => {
    setTimeout(() => {
      document
        .getElementById('tileset-form')
        ?.scrollIntoView({ behavior: 'smooth' });
    }, 100);
  };

  const handleTilesetClick = (tilesetId: string) => {
    const actualIndex = tilesets.findIndex((tileset) => tileset.name === tilesetId);
    if (actualIndex < 0) {
      return;
    }
    setEditTilesetIndex(actualIndex);
    scrollToTopOfForm();
    setShowForm(false);
    setTimeout(() => {
      setShowForm(true);
    }, 100);
  };

  const handleClone = (tilesetId: string) => {
    const actualIndex = tilesets.findIndex((tileset) => tileset.name === tilesetId);
    if (actualIndex < 0) {
      return;
    }
    const originalTileset = tilesets[actualIndex];
    const clonedTileset: TilesetTemplate = JSON.parse(
      JSON.stringify(originalTileset)
    );
    clonedTileset.name = clonedTileset.name + '_copy';
    org.copyPlacement(tilesetId, clonedTileset.name);
    const newTilesets = tilesets.slice();
    const clonedIndex = actualIndex + 1;
    newTilesets.splice(clonedIndex, 0, clonedTileset);
    setTilesets(newTilesets);
    setEditTilesetIndex(clonedIndex);
    showNotification('Tileset cloned!', 'success');
    scrollToTopOfForm();
    scrollAssetTreeItemIntoView(clonedTileset.name);
  };

  const handleDelete = (tilesetId: string) => {
    const actualIndex = tilesets.findIndex((tileset) => tileset.name === tilesetId);
    if (actualIndex < 0) {
      return;
    }
    if (confirm('Are you sure you want to delete this tileset?')) {
      org.removeItem(tilesetId);
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
    const selectedId =
      editTilesetIndex >= 0 ? tilesets[editTilesetIndex]?.name ?? null : null;
    org.fileNewItem(newTilesetTemplate.name, selectedId);
    const newTilesets = [...tilesets, newTilesetTemplate];
    setTilesets(newTilesets);
    const actualIndex = newTilesets.length - 1;
    setEditTilesetIndex(actualIndex);
    scrollToTopOfForm();
    setSearchTerm('');
    scrollAssetTreeItemIntoView(newTilesetTemplate.name);
  };

  usePersistedEditorSelection({
    editorKey: 'tilesetTemplates',
    items: tilesets,
    getId: (tileset) => tileset.name,
    selectedIndex: editTilesetIndex,
    setSelectedIndex: setEditTilesetIndex,
    routeParams,
    onRestored: () => scrollToTopOfForm(),
  });

  const updateTileset = (tileset: TilesetTemplate) => {
    if (editTilesetIndex < 0) {
      return;
    }
    org.rekeyItem(tilesets[editTilesetIndex].name, tileset.name);
    const updatedTilesets = [...tilesets];
    updatedTilesets[editTilesetIndex] = tileset;
    setTilesets(updatedTilesets);
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
      if (
        tileset.tileWidth === undefined ||
        tileset.tileWidth === null ||
        tileset.tileWidth <= 0
      ) {
        missingFields.push('tileWidth');
      }
      if (
        tileset.tileHeight === undefined ||
        tileset.tileHeight === null ||
        tileset.tileHeight <= 0
      ) {
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
      errors.push(
        `Duplicate tileset names found: ${duplicateNames.join(', ')}`
      );
    }

    if (tilesetsWithMissingFields.length > 0) {
      errors.push(
        `Tilesets with missing required fields:\n${tilesetsWithMissingFields.join(
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
    const validation = validateTilesets();
    if (!validation.isValid) {
      showNotification(validation.error || 'Validation failed', 'error');
      return;
    }

    const currentTilesetName =
      editTilesetIndex >= 0 ? tilesets[editTilesetIndex]?.name : undefined;

    const trimmedTilesets = trimStrings(tilesets);

    const sortedTilesets = trimmedTilesets.sort((a, b) => {
      return a.name.localeCompare(b.name);
    });

    try {
      const savedTilesets = await saveTilesets(sortedTilesets);
      showNotification('Tilesets saved successfully!', 'success');
      if (currentTilesetName) {
        const nextTilesetIndex = savedTilesets.findIndex(
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

  useSaveHotkey(handleSaveAll);

  const currentTileset = tilesets[editTilesetIndex];

  return (
    <div className="container editor-page">
      <EditorHeader
        title="Tileset Templates Editor"
        onSave={handleSaveAll}
        preserveAsset="tilesetTemplates"
      />

      <div className="editor-page-body">
        <div className="editor-content">
        <EditorSidebar
          searchTerm={searchTerm}
          onSearchChange={setSearchTerm}
          searchPlaceholder="Search tilesets..."
          createLabel="+ New Tileset"
          onCreate={handleCreateNew}
        >
          <AssetTree
            assetId="tilesetTemplates"
            items={filteredTilesets.map((tileset) => ({
              id: tileset.name,
              label: tileset.name,
              extra: (
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
              ),
            }))}
            allItemIds={tilesets.map((tileset) => tileset.name)}
            selectedId={
              editTilesetIndex >= 0 ? tilesets[editTilesetIndex]?.name : null
            }
            onSelectItem={handleTilesetClick}
            onClone={handleClone}
            onDelete={handleDelete}
            filterActive={searchTerm.trim() !== ''}
            emptyMessage="No tilesets found"
          />
        </EditorSidebar>

        <div className="editor-main">
          <div id="tileset-form">
            {showForm && (
              <TilesetTemplateForm
                tileset={currentTileset}
                updateTileset={updateTileset}
              />
            )}
          </div>
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
