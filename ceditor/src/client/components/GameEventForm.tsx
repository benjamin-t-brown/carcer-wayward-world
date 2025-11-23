import { useState } from 'react';
import { TextInput } from '../elements/TextInput';
import { OptionSelect } from '../elements/OptionSelect';
import { SpritePicker } from '../elements/SpritePicker';
import { Button } from '../elements/Button';
// import {
//   createDefaultChild,
//   GameEventChild,
//   GameEventChildForm,
// } from './GameEventChildForm';
import { DeleteModal } from '../elements/DeleteModal';
import { GameEvent } from '../types/assets';

// Re-export for backward compatibility
export type { GameEvent };

const GAME_EVENT_TYPES = ['MODAL', 'TALK', 'TRAVEL'];

interface GameEventFormProps {
  gameEvent?: GameEvent;
  updateGameEvent: (gameEvent: GameEvent) => void;
}

export function createDefaultGameEvent(): GameEvent {
  return {
    id: '',
    title: '',
    eventType: 'MODAL',
    icon: 'special_event_icons_0',
    vars: {},
    children: [],
  };
}

export function GameEventForm(props: GameEventFormProps) {
  const gameEvent = props.gameEvent;
  const [expandedChildren, setExpandedChildren] = useState<Set<number>>(
    new Set()
  );
  const [deleteConfirm, setDeleteConfirm] = useState<{
    isOpen: boolean;
    index: number | null;
  }>({ isOpen: false, index: null });

  const formData = gameEvent as GameEvent;
  const setFormData = (data: GameEvent) => {
    props.updateGameEvent(data);
  };

  const updateField = <K extends keyof GameEvent>(
    field: K,
    value: GameEvent[K]
  ) => {
    setFormData({ ...formData, [field]: value });
  };

  // const handleAddChild = () => {
  //   const newChildren = [...(formData.children || [])];
  //   const newChild = createDefaultChild();
  //   newChildren.push(newChild);
  //   updateField('children', newChildren);
  //   // Expand the newly added child
  //   setExpandedChildren(new Set([...expandedChildren, newChildren.length - 1]));
  // };

  // const handleDeleteChild = (index: number) => {
  //   setDeleteConfirm({ isOpen: true, index });
  // };

  const confirmDeleteChild = () => {
    if (deleteConfirm.index !== null) {
      const index = deleteConfirm.index;
      const newChildren = [...(formData.children || [])];
      newChildren.splice(index, 1);
      updateField('children', newChildren);
      // Remove from expanded set and adjust indices
      const newExpanded = new Set<number>();
      expandedChildren.forEach((idx) => {
        if (idx < index) {
          newExpanded.add(idx);
        } else if (idx > index) {
          newExpanded.add(idx - 1);
        }
      });
      setExpandedChildren(newExpanded);
    }
    setDeleteConfirm({ isOpen: false, index: null });
  };

  const handleCloneChild = (index: number) => {
    const newChildren = [...(formData.children || [])];
    const childToClone = newChildren[index];
    const clonedChild = JSON.parse(JSON.stringify(childToClone));
    clonedChild.id = `${clonedChild.id}_copy`;
    newChildren.splice(index + 1, 0, clonedChild);
    updateField('children', newChildren);
    // Expand the cloned child
    setExpandedChildren(new Set([...expandedChildren, index + 1]));
  };

  // const handleUpdateChild = (index: number, child: GameEventChild) => {
  //   const newChildren = [...(formData.children || [])];
  //   newChildren[index] = child;
  //   updateField('children', newChildren);
  // };

  const toggleChildExpanded = (index: number) => {
    const newExpanded = new Set(expandedChildren);
    if (newExpanded.has(index)) {
      newExpanded.delete(index);
    } else {
      newExpanded.add(index);
    }
    setExpandedChildren(newExpanded);
  };

  if (!gameEvent) {
    return <div>Select a game event to edit</div>;
  }

  return (
    <div className="item-form">
      <h2>Edit Game Event</h2>
      <form>
        <TextInput
          id="game-event-id"
          name="id"
          label="ID"
          value={formData.id}
          onChange={(value) => updateField('id', value)}
          placeholder="e.g., exampleEvent"
          required
        />

        <TextInput
          id="game-event-title"
          name="title"
          label="Title"
          value={formData.title}
          onChange={(value) => updateField('title', value)}
          placeholder="e.g., Example Event"
          required
        />

        <OptionSelect
          id="game-event-type"
          name="eventType"
          label="Event Type"
          value={formData.eventType}
          onChange={(value) =>
            updateField('eventType', value as 'MODAL' | 'TALK')
          }
          options={GAME_EVENT_TYPES.map((type) => ({
            value: type,
            label: type,
          }))}
          required
        />

        <div className="form-group">
          <label htmlFor="game-event-icon">Icon *</label>
          <div style={{ marginTop: '8px' }}>
            <SpritePicker
              value={formData.icon}
              onChange={(value) => updateField('icon', value)}
              scale={2}
              maxHeight="calc(100ch - 675px)"
            />
          </div>
          {formData.icon && (
            <div
              style={{ marginTop: '8px', fontSize: '12px', color: '#858585' }}
            >
              Selected: {formData.icon}
            </div>
          )}
        </div>

        {/* <div className="form-section" style={{ marginTop: '30px' }}>
          <div
            style={{
              display: 'flex',
              justifyContent: 'space-between',
              alignItems: 'center',
              marginBottom: '15px',
            }}
          >
            <h3>Children</h3>
            <Button variant="primary" onClick={handleAddChild}>
              + Add Child
            </Button>
          </div>

          {formData.children && formData.children.length > 0 ? (
            <div>
              {formData.children.map((child: GameEventChild, index: number) => (
                <GameEventChildForm
                  key={index}
                  child={child}
                  index={index}
                  isExpanded={expandedChildren.has(index)}
                  onToggle={() => toggleChildExpanded(index)}
                  onUpdate={(updatedChild) =>
                    handleUpdateChild(index, updatedChild)
                  }
                  onDelete={() => handleDeleteChild(index)}
                  onClone={() => handleCloneChild(index)}
                />
              ))}
            </div>
          ) : (
            <div style={{ color: '#858585', fontSize: '14px' }}>
              No children added yet. Click "Add Child" to add one.
            </div>
          )}
        </div> */}
      </form>

      <DeleteModal
        isOpen={deleteConfirm.isOpen}
        message="Are you sure you want to delete this child?"
        onConfirm={confirmDeleteChild}
        onCancel={() => setDeleteConfirm({ isOpen: false, index: null })}
      />
    </div>
  );
}
