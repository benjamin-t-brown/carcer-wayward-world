import { useState, useEffect } from 'react';
import { TextInput } from '../elements/TextInput';
import { OptionSelect } from '../elements/OptionSelect';
import { SpritePicker } from '../elements/SpritePicker';
import { Button } from '../elements/Button';
import { GameEvent } from '../types/assets';
import { GenericModal } from '../elements/GenericModal';

interface EditGameEventModalProps {
  isOpen: boolean;
  gameEvent: GameEvent | null;
  onConfirm: (previousGameEvent: GameEvent, newGameEvent: GameEvent) => void;
  onCancel: () => void;
}

const GAME_EVENT_TYPES = ['MODAL', 'TALK', 'TRAVEL'];

export function EditGameEventModal({
  isOpen,
  gameEvent,
  onConfirm,
  onCancel,
}: EditGameEventModalProps) {
  const [formData, setFormData] = useState<GameEvent | null>(null);

  useEffect(() => {
    if (gameEvent) {
      setFormData({ ...gameEvent });
    }
  }, [gameEvent]);

  if (!isOpen || !gameEvent || !formData) {
    return null;
  }

  const handleConfirm = () => {
    // Basic validation
    if (
      !formData.id ||
      !formData.title ||
      !formData.eventType ||
      !formData.icon
    ) {
      return;
    }
    console.log('formData', formData);
    onConfirm(gameEvent, formData);
  };

  return (
    <GenericModal
      title="Edit Game Event"
      maxWidth="600px"
      onConfirm={handleConfirm}
      onCancel={onCancel}
      body={() => (
        <>
          <div style={{ marginBottom: '20px' }}>
            <TextInput
              id="edit-game-event-id"
              name="id"
              label="ID"
              value={formData.id}
              onChange={(value) => setFormData({ ...formData, id: value })}
              placeholder="e.g., exampleEvent"
              required
            />
            <TextInput
              id="edit-game-event-title"
              name="title"
              label="Title"
              value={formData.title}
              onChange={(value) => setFormData({ ...formData, title: value })}
              placeholder="e.g., Example Event"
              required
            />
            <OptionSelect
              id="edit-game-event-type"
              name="eventType"
              label="Event Type"
              value={formData.eventType}
              onChange={(value) =>
                setFormData({
                  ...formData,
                  eventType: value as 'MODAL' | 'TALK' | 'TRAVEL',
                })
              }
              options={GAME_EVENT_TYPES.map((type) => ({
                value: type,
                label: type,
              }))}
              required
            />
            <div className="form-group" style={{ marginTop: '15px' }}>
              <label htmlFor="edit-game-event-icon">Icon *</label>
              <div style={{ marginTop: '8px' }}>
                <SpritePicker
                  value={formData.icon}
                  onChange={(value) =>
                    setFormData({ ...formData, icon: value })
                  }
                  scale={2}
                  maxHeight="300px"
                />
              </div>
              {formData.icon && (
                <div
                  style={{
                    marginTop: '8px',
                    fontSize: '12px',
                    color: '#858585',
                  }}
                >
                  Selected: {formData.icon}
                </div>
              )}
            </div>
          </div>
        </>
      )}
    ></GenericModal>
  );

  // return (
  //   <div
  //     style={{
  //       position: 'fixed',
  //       top: 0,
  //       left: 0,
  //       right: 0,
  //       bottom: 0,
  //       backgroundColor: 'rgba(0, 0, 0, 0.7)',
  //       display: 'flex',
  //       alignItems: 'center',
  //       justifyContent: 'center',
  //       zIndex: 1000,
  //     }}
  //     onClick={onCancel}
  //   >
  //     <div
  //       style={{
  //         backgroundColor: '#252526',
  //         border: '1px solid #3e3e42',
  //         borderRadius: '8px',
  //         padding: '30px',
  //         maxWidth: '600px',
  //         width: '90%',
  //         maxHeight: '90vh',
  //         overflowY: 'auto',
  //         position: 'relative',
  //       }}
  //       onClick={(e) => e.stopPropagation()}
  //     >
  //       <button
  //         style={{
  //           position: 'absolute',
  //           top: 1,
  //           right: 5,
  //           backgroundColor: 'transparent',
  //           border: 'none',
  //           color: '#d4d4d4',
  //           fontSize: '24px',
  //           fontWeight: 'bold',
  //           cursor: 'pointer',
  //         }}
  //         onClick={onCancel}
  //       >
  //         <span>×</span>
  //       </button>
  //       <h2
  //         style={{
  //           color: '#4ec9b0',
  //           marginBottom: '20px',
  //           marginTop: 0,
  //         }}
  //       >
  //         Edit Game Event
  //       </h2>
  //       <div style={{ marginBottom: '20px' }}>
  //         <TextInput
  //           id="edit-game-event-id"
  //           name="id"
  //           label="ID"
  //           value={formData.id}
  //           onChange={(value) => setFormData({ ...formData, id: value })}
  //           placeholder="e.g., exampleEvent"
  //           required
  //         />

  //         <TextInput
  //           id="edit-game-event-title"
  //           name="title"
  //           label="Title"
  //           value={formData.title}
  //           onChange={(value) => setFormData({ ...formData, title: value })}
  //           placeholder="e.g., Example Event"
  //           required
  //         />

  //         <OptionSelect
  //           id="edit-game-event-type"
  //           name="eventType"
  //           label="Event Type"
  //           value={formData.eventType}
  //           onChange={(value) =>
  //             setFormData({
  //               ...formData,
  //               eventType: value as 'MODAL' | 'TALK' | 'TRAVEL',
  //             })
  //           }
  //           options={GAME_EVENT_TYPES.map((type) => ({
  //             value: type,
  //             label: type,
  //           }))}
  //           required
  //         />

  //         <div className="form-group" style={{ marginTop: '15px' }}>
  //           <label htmlFor="edit-game-event-icon">Icon *</label>
  //           <div style={{ marginTop: '8px' }}>
  //             <SpritePicker
  //               value={formData.icon}
  //               onChange={(value) => setFormData({ ...formData, icon: value })}
  //               scale={2}
  //               maxHeight="300px"
  //             />
  //           </div>
  //           {formData.icon && (
  //             <div
  //               style={{ marginTop: '8px', fontSize: '12px', color: '#858585' }}
  //             >
  //               Selected: {formData.icon}
  //             </div>
  //           )}
  //         </div>
  //       </div>
  //       <div
  //         style={{
  //           display: 'flex',
  //           gap: '10px',
  //           justifyContent: 'flex-end',
  //         }}
  //       >
  //         <Button
  //           variant="primary"
  //           onClick={handleConfirm}
  //           disabled={
  //             !formData.id ||
  //             !formData.title ||
  //             !formData.eventType ||
  //             !formData.icon
  //           }
  //         >
  //           Save
  //         </Button>
  //         <Button variant="secondary" onClick={onCancel}>
  //           Cancel
  //         </Button>
  //       </div>
  //     </div>
  //   </div>
  // );
}
