import { useState, useEffect, useRef } from 'react';
import { CardList } from '../components/CardList';
import {
  ItemTemplateForm,
  ItemTemplate,
  createDefaultItem,
} from '../components/ItemTemplateForm';
import { Button } from '../elements/Button';
import { Notification } from '../elements/Notification';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { Sprite } from '../elements/Sprite';
import { trimStrings } from '../utils/jsonUtils';

async function loadItems(): Promise<ItemTemplate[]> {
  const response = await fetch('/api/assets/itemTemplates');
  if (!response.ok) {
    throw new Error('Failed to load items');
  }
  return response.json();
}

async function saveItems(items: ItemTemplate[]): Promise<void> {
  console.log('save items', items);
  const response = await fetch('/api/assets/itemTemplates', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify(items),
  });

  if (!response.ok) {
    throw new Error('Failed to save items');
  }
}

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
}

export function ItemTemplates() {
  const {
    sprites,
    animations: _animations,
    pictures: _pictures,
  } = useSDL2WAssets();
  const [items, setItems] = useState<ItemTemplate[]>([]);
  const [editItemIndex, setEditItemIndex] = useState<number>(-1);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<string | null>(null);
  const [searchTerm, setSearchTerm] = useState('');
  const [notifications, setNotifications] = useState<NotificationState[]>([]);
  const notificationIdRef = useRef(0);

  // Load items on mount
  useEffect(() => {
    loadItems()
      .then((loadedItems) => {
        setItems(
          loadedItems.sort((a, b) => {
            const cmp = a.name.localeCompare(b.name);
            return cmp === 0 ? a.label.localeCompare(b.label) : cmp;
          })
        );
        setLoading(false);
      })
      .catch((err) => {
        setError(err instanceof Error ? err.message : 'Unknown error');
        setLoading(false);
      });
  }, []);

  const showNotification = (message: string, type: 'success' | 'error') => {
    const id = notificationIdRef.current++;
    setNotifications((prev) => [...prev, { message, type, id }]);
  };

  const removeNotification = (id: number) => {
    setNotifications((prev) => prev.filter((n) => n.id !== id));
  };

  // Filter items based on search term
  const filteredItems = items.filter(
    (item) =>
      item.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
      item.label.toLowerCase().includes(searchTerm.toLowerCase()) ||
      item.itemType.toLowerCase().includes(searchTerm.toLowerCase())
  );

  // Get the actual index in the full items array for filtered items
  const getActualIndex = (filteredIndex: number): number => {
    const filteredItem = filteredItems[filteredIndex];
    return items.indexOf(filteredItem);
  };

  const scrollToTopOfForm = () => {
    setTimeout(() => {
      document
        .getElementById('item-form')
        ?.scrollIntoView({ behavior: 'smooth' });
    }, 100);
  };

  const handleItemClick = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    setEditItemIndex(actualIndex);
    scrollToTopOfForm();
  };

  const handleClone = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    const originalItem = items[actualIndex];
    const clonedItem: ItemTemplate = JSON.parse(JSON.stringify(originalItem));
    clonedItem.name = clonedItem.name + '_copy';
    const newItems = items.slice();
    const clonedIndex = actualIndex + 1;
    newItems.splice(clonedIndex, 0, clonedItem);
    setItems(newItems);
    setEditItemIndex(clonedIndex);
    showNotification('Item cloned!', 'success');
    scrollToTopOfForm();
    setTimeout(() => {
      const itemCard = document.getElementById(`item-card-${clonedIndex}`);
      console.log('scrollItemCardIntoView', clonedIndex, itemCard);
      if (itemCard) {
        itemCard.scrollIntoView({ behavior: 'smooth' });
      }
    }, 100);
  };

  const handleDelete = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    if (confirm('Are you sure you want to delete this item?')) {
      const newItems = items.filter((_, index) => index !== actualIndex);
      setItems(newItems);
      if (editItemIndex === actualIndex) {
        setEditItemIndex(-1);
      } else if (editItemIndex > actualIndex) {
        setEditItemIndex(editItemIndex - 1);
        // scrollItemCardIntoView(filteredIndex);
      }
    }
  };

  const handleCreateNew = () => {
    const newItemTemplate = createDefaultItem();
    const newItems = [...items, newItemTemplate];
    setItems(newItems);
    const actualIndex = newItems.length - 1;
    setEditItemIndex(actualIndex);
    scrollToTopOfForm();
    setSearchTerm('');
    setTimeout(() => {
      const itemCard = document.getElementById(`item-card-${actualIndex}`);
      console.log('scrollItemCardIntoView', actualIndex, itemCard);
      if (itemCard) {
        itemCard.scrollIntoView({ behavior: 'smooth' });
      }
    }, 100);
  };

  const updateItem = (item: ItemTemplate) => {
    const currentItemIndex = getActualIndex(editItemIndex);
    const currentItem = items[currentItemIndex];
    if (currentItem) {
      // Update existing item in real-time
      const updatedItems = [...items];
      updatedItems[currentItemIndex] = item;
      setItems(updatedItems);
    }
  };

  const validateItems = (): { isValid: boolean; error?: string } => {
    const errors: string[] = [];
    const nameCounts = new Map<string, number>();
    const itemsWithMissingFields: string[] = [];

    items.forEach((item, index) => {
      const missingFields: string[] = [];

      // Check required string fields
      if (!item.itemType || item.itemType.trim() === '') {
        missingFields.push('itemType');
      }
      if (!item.name || item.name.trim() === '') {
        missingFields.push('name');
      }
      if (!item.label || item.label.trim() === '') {
        missingFields.push('label');
      }
      if (!item.icon || item.icon.trim() === '') {
        missingFields.push('icon');
      }
      if (!item.description || item.description.trim() === '') {
        missingFields.push('description');
      }

      // Check required number fields
      if (item.weight === undefined || item.weight === null || isNaN(item.weight)) {
        missingFields.push('weight');
      }
      if (item.value === undefined || item.value === null || isNaN(item.value)) {
        missingFields.push('value');
      }

      if (missingFields.length > 0) {
        const itemIdentifier = item.name || `Item at index ${index}`;
        itemsWithMissingFields.push(
          `${itemIdentifier}: missing ${missingFields.join(', ')}`
        );
      }

      // Track names for duplicate checking
      if (item.name && item.name.trim()) {
        const count = nameCounts.get(item.name) || 0;
        nameCounts.set(item.name, count + 1);
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
      errors.push(`Duplicate item names found: ${duplicateNames.join(', ')}`);
    }

    if (itemsWithMissingFields.length > 0) {
      errors.push(`Items with missing required fields:\n${itemsWithMissingFields.join('\n')}`);
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
    const validation = validateItems();
    if (!validation.isValid) {
      showNotification(validation.error || 'Validation failed', 'error');
      return;
    }

    const currentItemIndex = getActualIndex(editItemIndex);
    const currentItemName = items[currentItemIndex]?.name;
    
    const trimmedItems = trimStrings(items);
    
    const sortedItems = trimmedItems.sort((a, b) => {
      const cmp = a.name.localeCompare(b.name);
      return cmp === 0 ? a.label.localeCompare(b.label) : cmp;
    });
    
    try {
      await saveItems(sortedItems);
      showNotification('Items saved successfully!', 'success');
      if (currentItemName) {
        const nextItemIndex = sortedItems.findIndex(
          (item) => item.name === currentItemName.trim()
        );
        setEditItemIndex(nextItemIndex);
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
  }, [items, showNotification]); // Include dependencies

  const currentItem = items[editItemIndex];

  return (
    <div className="container">
      <div className="editor-header">
        <Button variant="back" onClick={() => window.history.back()}>
          ← Back
        </Button>
        <h1>Item Templates Editor</h1>
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
              placeholder="Search items..."
            />
          </div>
          <div className="item-actions">
            <Button variant="primary" onClick={handleCreateNew}>
              + New Item
            </Button>
          </div>
          {loading ? (
            <div className="loading">Loading items...</div>
          ) : error ? (
            <div className="error">Error loading items: {error}</div>
          ) : (
            <CardList
              items={filteredItems}
              onItemClick={handleItemClick}
              onClone={handleClone}
              onDelete={handleDelete}
              selectedIndex={
                editItemIndex !== -1
                  ? (() => {
                      const index = filteredItems.findIndex(
                        (item) => items.indexOf(item) === editItemIndex
                      );
                      return index >= 0 ? index : null;
                    })()
                  : null
              }
              renderAdditionalInfo={(item) => {
                const sprite = sprites.find((s) => s.name === item.icon);
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
                      {sprite && (
                        <div style={{ display: 'inline-block' }}>
                          <Sprite sprite={sprite} scale={1.5} />
                        </div>
                      )}
                      <span className="item-type">{item.itemType}</span>
                    </div>
                  </>
                );
              }}
              emptyMessage="No items found"
            />
          )}
        </div>

        <div className="editor-main">
          <div id="item-form">
            <ItemTemplateForm item={currentItem} updateItem={updateItem} />
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
