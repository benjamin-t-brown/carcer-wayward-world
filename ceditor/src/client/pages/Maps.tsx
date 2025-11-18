import { useState, useRef, useEffect } from 'react';
import { CarcerMapTemplate } from '../components/MapTemplateForm';
import { Button } from '../elements/Button';
import { Notification } from '../elements/Notification';
import { OptionSelect } from '../elements/OptionSelect';
import { useAssets } from '../contexts/AssetsContext';
import { trimStrings } from '../utils/jsonUtils';
import { DeleteModal } from '../elements/DeleteModal';
import { CreateMapModal } from '../components/CreateMapModal';
import { EditMapModal } from '../components/EditMapModal';
import { TileEditor } from '../tile-editor/TileEditor';

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
}

interface OpenTab {
  mapIndex: number;
  map: CarcerMapTemplate;
}

export function Maps() {
  const { maps, setMaps, saveMaps } = useAssets();
  const [openTabs, setOpenTabs] = useState<OpenTab[]>([]);
  const [activeTabIndex, setActiveTabIndex] = useState<number | null>(null);
  const [selectedMapIndex, setSelectedMapIndex] = useState<string>('');
  const [notifications, setNotifications] = useState<NotificationState[]>([]);
  const notificationIdRef = useRef(0);
  const [deleteConfirm, setDeleteConfirm] = useState<{
    isOpen: boolean;
    mapIndex: number | null;
  }>({ isOpen: false, mapIndex: null });
  const [createModalOpen, setCreateModalOpen] = useState(false);
  const [editModalOpen, setEditModalOpen] = useState(false);

  const showNotification = (message: string, type: 'success' | 'error') => {
    const id = notificationIdRef.current++;
    setNotifications((prev) => [...prev, { message, type, id }]);
  };

  const removeNotification = (id: number) => {
    setNotifications((prev) => prev.filter((n) => n.id !== id));
  };

  // Map options for OptionSelect
  const mapOptions = [
    { value: '', label: '-- Select a map --' },
    ...maps.map((map, index) => ({
      value: index.toString(),
      label: `${map.label} (${map.name})`,
    })),
  ];

  const handleMapSelect = (value: string) => {
    setSelectedMapIndex(value);
    if (value === '') {
      return;
    }

    const mapIndex = parseInt(value, 10);
    if (isNaN(mapIndex) || mapIndex < 0 || mapIndex >= maps.length) {
      return;
    }

    // Check if this map is already open in a tab
    const existingTabIndex = openTabs.findIndex(
      (tab) => tab.mapIndex === mapIndex
    );

    if (existingTabIndex >= 0) {
      // Switch to existing tab
      setActiveTabIndex(existingTabIndex);
    } else {
      // Open new tab
      const newTab: OpenTab = {
        mapIndex,
        map: maps[mapIndex],
      };
      const newTabs = [...openTabs, newTab];
      setOpenTabs(newTabs);
      setActiveTabIndex(newTabs.length - 1);
    }
  };

  const handleCreateMap = (newMap: CarcerMapTemplate) => {
    // Generate tiles for all positions in the map if not already generated
    if (!newMap.tiles || newMap.tiles.length === 0) {
      const tiles: any[] = [];
      for (let y = 0; y < newMap.height; y++) {
        for (let x = 0; x < newMap.width; x++) {
          tiles.push({
            tilesetName: '',
            tileId: 0,
            x: x,
            y: y,
            characters: [],
            items: [],
          });
        }
      }
      newMap.tiles = tiles;
    }

    const newMaps = [...maps, newMap];
    setMaps(newMaps);
    const newMapIndex = newMaps.length - 1;

    // Open the new map in a tab
    const newTab: OpenTab = {
      mapIndex: newMapIndex,
      map: newMap,
    };
    const newTabs = [...openTabs, newTab];
    setOpenTabs(newTabs);
    setActiveTabIndex(newTabs.length - 1);
    setCreateModalOpen(false);
    showNotification('Map created!', 'success');
  };

  const handleCloseTab = (tabIndex: number) => {
    const newTabs = openTabs.filter((_, index) => index !== tabIndex);
    setOpenTabs(newTabs);

    if (activeTabIndex === tabIndex) {
      // If we closed the active tab, switch to another one
      if (newTabs.length > 0) {
        // Switch to the tab that was after the closed one, or the last tab
        const newActiveIndex = Math.min(tabIndex, newTabs.length - 1);
        setActiveTabIndex(newActiveIndex);
      } else {
        setActiveTabIndex(null);
      }
    } else if (activeTabIndex !== null && activeTabIndex > tabIndex) {
      // Adjust active tab index if it was after the closed tab
      setActiveTabIndex(activeTabIndex - 1);
    }
  };


  const confirmDelete = () => {
    if (deleteConfirm.mapIndex !== null) {
      const mapIndex = deleteConfirm.mapIndex;
      const newMaps = maps.filter((_, index) => index !== mapIndex);
      setMaps(newMaps);

      // Close any tabs that reference this map
      const newTabs = openTabs
        .map((tab) => {
          if (tab.mapIndex === mapIndex) {
            return null; // Mark for removal
          }
          // Adjust mapIndex if it was after the deleted map
          if (tab.mapIndex > mapIndex) {
            return { ...tab, mapIndex: tab.mapIndex - 1 };
          }
          return tab;
        })
        .filter((tab): tab is OpenTab => tab !== null);

      setOpenTabs(newTabs);

      // Adjust active tab index
      if (activeTabIndex !== null) {
        if (newTabs.length === 0) {
          setActiveTabIndex(null);
        } else {
          setActiveTabIndex(Math.min(activeTabIndex, newTabs.length - 1));
        }
      }

      // Reset selection if it was the deleted map
      if (selectedMapIndex === mapIndex.toString()) {
        setSelectedMapIndex('');
      }
    }
    setDeleteConfirm({ isOpen: false, mapIndex: null });
  };

  const updateMapInTabs = (updatedMap: CarcerMapTemplate) => {
    if (activeTab) {
      const mapIndex = activeTab.mapIndex;
      const updatedMaps = [...maps];
      updatedMaps[mapIndex] = updatedMap;
      setMaps(updatedMaps);

      // Update the tab's map reference
      const tabIndex = openTabs.findIndex((tab) => tab.mapIndex === mapIndex);
      if (tabIndex >= 0) {
        const newTabs = [...openTabs];
        newTabs[tabIndex] = { ...newTabs[tabIndex], map: updatedMap };
        setOpenTabs(newTabs);
      }
    }
  };

  const handleEditMap = (updatedMap: CarcerMapTemplate) => {
    updateMapInTabs(updatedMap);
    setEditModalOpen(false);
  };

  const validateMaps = (): { isValid: boolean; error?: string } => {
    const errors: string[] = [];
    const nameCounts = new Map<string, number>();
    const mapsWithMissingFields: string[] = [];

    maps.forEach((map, index) => {
      const missingFields: string[] = [];

      // Check required string fields
      if (!map.name || map.name.trim() === '') {
        missingFields.push('name');
      }
      if (!map.label || map.label.trim() === '') {
        missingFields.push('label');
      }

      // Check required number fields
      if (map.width === undefined || map.width === null || map.width <= 0) {
        missingFields.push('width');
      }
      if (map.height === undefined || map.height === null || map.height <= 0) {
        missingFields.push('height');
      }

      if (missingFields.length > 0) {
        const mapIdentifier = map.name || `Map at index ${index}`;
        mapsWithMissingFields.push(
          `${mapIdentifier}: missing ${missingFields.join(', ')}`
        );
      }

      // Track names for duplicate checking
      if (map.name && map.name.trim()) {
        const count = nameCounts.get(map.name) || 0;
        nameCounts.set(map.name, count + 1);
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
      errors.push(`Duplicate map names found: ${duplicateNames.join(', ')}`);
    }

    if (mapsWithMissingFields.length > 0) {
      errors.push(
        `Maps with missing required fields:\n${mapsWithMissingFields.join(
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
    const validation = validateMaps();
    if (!validation.isValid) {
      showNotification(validation.error || 'Validation failed', 'error');
      return;
    }

    const trimmedMaps = trimStrings(maps);
    const sortedMaps = trimmedMaps.sort((a, b) => {
      return a.name.localeCompare(b.name);
    });

    try {
      await saveMaps(sortedMaps);
      setMaps(sortedMaps);
      showNotification('Maps saved successfully!', 'success');

      // Update tab references after sorting
      const newTabs = openTabs.map((tab) => {
        const sortedIndex = sortedMaps.findIndex(
          (map) => map.name === tab.map.name
        );
        if (sortedIndex >= 0) {
          return {
            mapIndex: sortedIndex,
            map: sortedMaps[sortedIndex],
          };
        }
        return tab;
      });
      setOpenTabs(newTabs);
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
  }, [maps]);

  const activeTab = activeTabIndex !== null ? openTabs[activeTabIndex] : null;
  const activeMap = activeTab ? maps[activeTab.mapIndex] : null;

  return (
    <div
      className="container"
      style={{ display: 'flex', flexDirection: 'column' }}
    >
      <div className="editor-header">
        <Button variant="back" onClick={() => window.history.back()}>
          ← Back
        </Button>
        <h1>Maps Editor</h1>
        <div style={{ display: 'flex', gap: '10px', alignItems: 'center' }}>
          <OptionSelect
            style={{ marginBottom: 'unset' }}
            id="map-select"
            name="mapSelect"
            label=""
            value={selectedMapIndex}
            onChange={handleMapSelect}
            options={mapOptions}
          />
          <Button variant="primary" onClick={() => setCreateModalOpen(true)}>
            + New Map
          </Button>
          <Button variant="primary" onClick={handleSaveAll}>
            Save All
          </Button>
        </div>
      </div>

      {openTabs.length > 0 && (
        <div
          style={{
            display: 'flex',
            borderBottom: '1px solid #3e3e42',
            backgroundColor: '#1e1e1e',
            overflowX: 'auto',
          }}
        >
          {openTabs.map((tab, index) => (
            <div
              key={index}
              style={{
                display: 'flex',
                alignItems: 'center',
                padding: '8px 16px',
                borderRight: '1px solid #3e3e42',
                backgroundColor:
                  activeTabIndex === index ? '#252526' : '#1e1e1e',
                cursor: 'pointer',
                minWidth: '150px',
                maxWidth: '250px',
              }}
              onClick={() => setActiveTabIndex(index)}
            >
              <span
                style={{
                  color: activeTabIndex === index ? '#4ec9b0' : '#858585',
                  flex: 1,
                  overflow: 'hidden',
                  textOverflow: 'ellipsis',
                  whiteSpace: 'nowrap',
                  marginRight: '8px',
                }}
                title={tab.map.label}
              >
                {tab.map.label}
              </span>
              <button
                onClick={(e) => {
                  e.stopPropagation();
                  setActiveTabIndex(index);
                  setEditModalOpen(true);
                }}
                style={{
                  background: 'none',
                  border: 'none',
                  color: '#858585',
                  cursor: 'pointer',
                  fontSize: '14px',
                  padding: '0 4px',
                  lineHeight: '1',
                  marginRight: '4px',
                }}
                title="Edit Map Properties"
                onMouseEnter={(e) => {
                  e.currentTarget.style.color = '#4ec9b0';
                }}
                onMouseLeave={(e) => {
                  e.currentTarget.style.color = '#858585';
                }}
              >
                ✎
              </button>
              <button
                onClick={(e) => {
                  e.stopPropagation();
                  handleCloseTab(index);
                }}
                style={{
                  background: 'none',
                  border: 'none',
                  color: '#858585',
                  cursor: 'pointer',
                  fontSize: '18px',
                  padding: '0 4px',
                  lineHeight: '1',
                }}
                title="Close Tab"
                onMouseEnter={(e) => {
                  e.currentTarget.style.color = '#f48771';
                }}
                onMouseLeave={(e) => {
                  e.currentTarget.style.color = '#858585';
                }}
              >
                ×
              </button>
            </div>
          ))}
        </div>
      )}

      <div
        style={{
          // flex: 1,
          overflow: 'hidden',
          height: 'calc(100vh - 152px)',
        }}
      >
        {activeMap ? (
          <TileEditor
            map={activeMap}
            onMapUpdate={updateMapInTabs}
          />
        ) : (
          <div
            style={{
              color: '#858585',
              fontSize: '14px',
              textAlign: 'center',
              marginTop: '50px',
            }}
          >
            {openTabs.length === 0
              ? 'Select a map from the dropdown or create a new one to get started.'
              : 'Select a tab to view the map.'}
          </div>
        )}
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

      <DeleteModal
        isOpen={deleteConfirm.isOpen}
        message="Are you sure you want to delete this map?"
        onConfirm={confirmDelete}
        onCancel={() => setDeleteConfirm({ isOpen: false, mapIndex: null })}
      />

      <CreateMapModal
        isOpen={createModalOpen}
        onConfirm={handleCreateMap}
        onCancel={() => setCreateModalOpen(false)}
      />

      <EditMapModal
        isOpen={editModalOpen}
        map={activeMap}
        onConfirm={handleEditMap}
        onCancel={() => setEditModalOpen(false)}
        onDelete={() => {
          if (activeTab) {
            const mapIndex = activeTab.mapIndex;
            setDeleteConfirm({ isOpen: true, mapIndex });
          }
        }}
      />
    </div>
  );
}
