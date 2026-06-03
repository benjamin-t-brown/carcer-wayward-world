import { useState, useRef, useEffect } from 'react';
import { CardList } from '../components/CardList';
import { EditorSidebar } from '../components/EditorSidebar';
import { FeatTemplate } from '../types/assets';
import {
  FeatTemplateForm,
  createDefaultFeatTemplate,
} from '../components/FeatTemplateForm';
import { EditorHeader } from '../components/EditorHeader';
import { Notification } from '../elements/Notification';
import { useAssets } from '../contexts/AssetsContext';
import { trimStrings } from '../utils/jsonUtils';

interface NotificationState {
  message: string;
  type: 'success' | 'error';
  id: number;
}

interface FeatListItem {
  name: string;
  label?: string;
  implementation?: string;
}

export function FeatTemplates() {
  const { feats, setFeats, saveFeats } = useAssets();
  const [editFeatIndex, setEditFeatIndex] = useState<number>(-1);
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

  const filteredFeats = feats.filter(
    (feat) =>
      feat.id.toLowerCase().includes(searchTerm.toLowerCase()) ||
      feat.label.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const filteredListItems: FeatListItem[] = filteredFeats.map((feat) => ({
    name: feat.id,
    label: feat.label || feat.id,
    implementation: feat.implementation || 'DATA',
  }));

  const getActualIndex = (filteredIndex: number): number => {
    const filteredFeat = filteredFeats[filteredIndex];
    return feats.indexOf(filteredFeat);
  };

  const handleFeatClick = (filteredIndex: number) => {
    setEditFeatIndex(getActualIndex(filteredIndex));
  };

  const handleClone = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    const clonedFeat: FeatTemplate = JSON.parse(JSON.stringify(feats[actualIndex]));
    clonedFeat.id = clonedFeat.id + '_copy';
    const newFeats = feats.slice();
    const clonedIndex = actualIndex + 1;
    newFeats.splice(clonedIndex, 0, clonedFeat);
    setFeats(newFeats);
    setEditFeatIndex(clonedIndex);
    showNotification('Feat cloned!', 'success');
    setTimeout(() => {
      const featCard = document.getElementById(`item-card-${clonedIndex}`);
      if (featCard) {
        featCard.scrollIntoView({ behavior: 'smooth' });
      }
    }, 100);
  };

  const handleDelete = (filteredIndex: number) => {
    const actualIndex = getActualIndex(filteredIndex);
    if (confirm('Are you sure you want to delete this feat?')) {
      const newFeats = feats.filter((_, index) => index !== actualIndex);
      setFeats(newFeats);
      if (editFeatIndex === actualIndex) {
        setEditFeatIndex(-1);
      } else if (editFeatIndex > actualIndex) {
        setEditFeatIndex(editFeatIndex - 1);
      }
    }
  };

  const handleCreateNew = () => {
    const newFeat = createDefaultFeatTemplate();
    const newFeats = [...feats, newFeat];
    setFeats(newFeats);
    const actualIndex = newFeats.length - 1;
    setEditFeatIndex(actualIndex);
    setSearchTerm('');
    setTimeout(() => {
      const featCard = document.getElementById(`item-card-${actualIndex}`);
      if (featCard) {
        featCard.scrollIntoView({ behavior: 'smooth' });
      }
    }, 100);
  };

  const updateFeat = (feat: FeatTemplate) => {
    if (editFeatIndex < 0) {
      return;
    }
    const updatedFeats = [...feats];
    updatedFeats[editFeatIndex] = feat;
    setFeats(updatedFeats);
  };

  const handleSaveAll = async () => {
    const currentFeatId = editFeatIndex >= 0 ? feats[editFeatIndex]?.id : undefined;

    const trimmedFeats = trimStrings(feats);
    const sortedFeats = trimmedFeats.sort((a, b) => {
      const cmp = a.id.localeCompare(b.id);
      return cmp === 0 ? a.label.localeCompare(b.label) : cmp;
    });

    try {
      await saveFeats(sortedFeats);
      setFeats(sortedFeats);
      showNotification('Feats saved successfully!', 'success');
      if (currentFeatId) {
        const nextIndex = sortedFeats.findIndex((feat) => feat.id === currentFeatId.trim());
        setEditFeatIndex(nextIndex);
      }
    } catch (error) {
      showNotification(
        error instanceof Error ? error.message : 'Failed to save feats',
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
    return () => {
      window.removeEventListener('keydown', handleKeyDown);
    };
  }, [feats]);

  const currentFeat = editFeatIndex >= 0 ? feats[editFeatIndex] : undefined;

  return (
    <div className="container editor-page">
      <EditorHeader title="Feat Templates Editor" onSave={handleSaveAll} />

      <div className="editor-page-body">
        <div className="editor-content">
        <EditorSidebar
          searchTerm={searchTerm}
          onSearchChange={setSearchTerm}
          searchPlaceholder="Search feats..."
          createLabel="+ New Feat"
          onCreate={handleCreateNew}
        >
          <CardList
            items={filteredListItems}
            onItemClick={handleFeatClick}
            onClone={handleClone}
            onDelete={handleDelete}
            selectedIndex={
              editFeatIndex !== -1
                ? (() => {
                    const index = filteredFeats.findIndex(
                      (feat) => feats.indexOf(feat) === editFeatIndex
                    );
                    return index >= 0 ? index : null;
                  })()
                : null
            }
            renderAdditionalInfo={(item) => (
              <div className="item-info">
                <span className="item-type">{item.implementation || 'DATA'}</span>
              </div>
            )}
            emptyMessage="No feats found"
          />
        </EditorSidebar>

        <div className="editor-main">
          <div id="feat-form">
            <FeatTemplateForm feat={currentFeat} updateFeat={updateFeat} />
          </div>
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
