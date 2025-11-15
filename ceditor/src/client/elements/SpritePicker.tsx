import { useState, useMemo } from 'react';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { Sprite } from './Sprite';
import { OptionSelect } from './OptionSelect';
import { Button } from './Button';

interface SpritePickerProps {
  value: string;
  onChange: (spriteName: string) => void;
  scale?: number;
  className?: string;
}

export function SpritePicker({
  value,
  onChange,
  scale = 2,
  className = '',
}: SpritePickerProps) {
  const { sprites } = useSDL2WAssets();
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [selectedSpritesheet, setSelectedSpritesheet] = useState<string>('');
  const [selectedSpriteName, setSelectedSpriteName] = useState<string>(value);

  // Find the current sprite
  const currentSprite = useMemo(() => {
    return sprites.find((s) => s.name === value);
  }, [sprites, value]);

  // Group sprites by spritesheet (pictureAlias)
  const spritesBySheet = useMemo(() => {
    const grouped: Record<string, typeof sprites> = {};
    sprites.forEach((sprite) => {
      if (!grouped[sprite.pictureAlias]) {
        grouped[sprite.pictureAlias] = [];
      }
      grouped[sprite.pictureAlias].push(sprite);
    });
    return grouped;
  }, [sprites]);

  // Get spritesheet options
  const spritesheetOptions = useMemo(() => {
    return Object.keys(spritesBySheet).map((alias) => ({
      value: alias,
      label: alias,
    }));
  }, [spritesBySheet]);

  // Get sprites for the selected spritesheet
  const sheetSprites = useMemo(() => {
    if (!selectedSpritesheet) return [];
    return spritesBySheet[selectedSpritesheet] || [];
  }, [selectedSpritesheet, spritesBySheet]);

  // Initialize modal state when opened
  const handleOpenModal = () => {
    if (currentSprite) {
      setSelectedSpritesheet(currentSprite.pictureAlias);
      setSelectedSpriteName(value);
    } else if (spritesheetOptions.length > 0) {
      setSelectedSpritesheet(spritesheetOptions[0].value);
      setSelectedSpriteName('');
    }
    setIsModalOpen(true);
  };

  const handleCloseModal = () => {
    setIsModalOpen(false);
  };

  const handleOk = () => {
    if (selectedSpriteName) {
      onChange(selectedSpriteName);
    }
    setIsModalOpen(false);
  };

  const handleCancel = () => {
    setIsModalOpen(false);
  };

  // When spritesheet changes, reset sprite selection
  const handleSpritesheetChange = (newSpritesheet: string) => {
    setSelectedSpritesheet(newSpritesheet);
    const firstSprite = spritesBySheet[newSpritesheet]?.[0];
    setSelectedSpriteName(firstSprite?.name || '');
  };

  // Get the selected sprite for preview
  const previewSprite = useMemo(() => {
    return sprites.find((s) => s.name === selectedSpriteName);
  }, [sprites, selectedSpriteName]);

  if (!currentSprite) {
    return (
      <div
        className={className}
        style={{
          padding: '10px',
          backgroundColor: '#2d2d30',
          border: '1px solid #3e3e42',
          borderRadius: '4px',
          cursor: 'pointer',
          display: 'inline-block',
        }}
        onClick={handleOpenModal}
      >
        <div style={{ color: '#858585', fontSize: '12px' }}>
          {value || 'No sprite selected'}
        </div>
        <div style={{ color: '#858585', fontSize: '10px', marginTop: '4px' }}>
          Click to select
        </div>
      </div>
    );
  }

  return (
    <>
      <div
        className={className}
        style={{
          margin: '16px',
          display: 'inline-block',
          cursor: 'pointer',
        }}
        onClick={handleOpenModal}
      >
        <Sprite sprite={currentSprite} scale={scale} />
      </div>

      {isModalOpen && (
        <div
          style={{
            position: 'fixed',
            top: 0,
            left: 0,
            right: 0,
            bottom: 0,
            backgroundColor: 'rgba(0, 0, 0, 0.7)',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            zIndex: 1000,
          }}
          onClick={handleCloseModal}
        >
          <div
            style={{
              backgroundColor: '#252526',
              border: '1px solid #3e3e42',
              borderRadius: '8px',
              padding: '30px',
              maxWidth: '600px',
              width: '90%',
              maxHeight: '80vh',
              overflow: 'auto',
            }}
            onClick={(e) => e.stopPropagation()}
          >
            <h2
              style={{ color: '#4ec9b0', marginBottom: '20px', marginTop: 0 }}
            >
              Select Sprite
            </h2>

            <OptionSelect
              label="Spritesheet"
              value={selectedSpritesheet}
              onChange={handleSpritesheetChange}
              options={spritesheetOptions}
            />

            {previewSprite && selectedSpriteName && (
              <div
                style={{
                  marginTop: '20px',
                  padding: '20px',
                  backgroundColor: '#1e1e1e',
                  borderRadius: '4px',
                  textAlign: 'center',
                }}
              >
                <div style={{ marginBottom: '10px', color: '#d4d4d4' }}>
                  Preview:
                </div>
                <div
                  style={{
                    display: 'inline-block',
                    padding: '10px',
                    backgroundColor: '#2d2d30',
                    borderRadius: '4px',
                  }}
                >
                  <Sprite sprite={previewSprite} scale={4} />
                </div>
                <div
                  style={{
                    marginTop: '10px',
                    color: '#858585',
                    fontSize: '12px',
                  }}
                >
                  {previewSprite.name}
                </div>
              </div>
            )}

            {selectedSpritesheet && (
              <div style={{ marginTop: '20px' }}>
                <label
                  style={{
                    display: 'block',
                    marginBottom: '8px',
                    color: '#d4d4d4',
                    fontSize: '14px',
                    fontWeight: 500,
                  }}
                >
                  Sprite
                </label>
                {sheetSprites.length > 0 ? (
                  <div
                    style={{
                      display: 'grid',
                      gridTemplateColumns:
                        'repeat(auto-fill, minmax(80px, 1fr))',
                      gap: '12px',
                      padding: '15px',
                      backgroundColor: '#1e1e1e',
                      borderRadius: '4px',
                      maxHeight: '400px',
                      overflowY: 'auto',
                    }}
                  >
                    {sheetSprites.map((sprite) => {
                      const isSelected = sprite.name === selectedSpriteName;
                      return (
                        <div
                          key={sprite.name}
                          onClick={() => setSelectedSpriteName(sprite.name)}
                          style={{
                            cursor: 'pointer',
                            padding: '8px',
                            backgroundColor: isSelected ? '#2d2d30' : '#252526',
                            border: isSelected
                              ? '2px solid #4ec9b0'
                              : '2px solid #3e3e42',
                            borderRadius: '4px',
                            transition: 'all 0.2s',
                            textAlign: 'center',
                          }}
                          onMouseEnter={(e) => {
                            if (!isSelected) {
                              e.currentTarget.style.borderColor = '#4ec9b0';
                              e.currentTarget.style.backgroundColor = '#2d2d30';
                            }
                          }}
                          onMouseLeave={(e) => {
                            if (!isSelected) {
                              e.currentTarget.style.borderColor = '#3e3e42';
                              e.currentTarget.style.backgroundColor = '#252526';
                            }
                          }}
                        >
                          <div
                            style={{
                              display: 'flex',
                              justifyContent: 'center',
                              marginBottom: '6px',
                            }}
                          >
                            <Sprite sprite={sprite} scale={2} />
                          </div>
                          <div
                            style={{
                              fontSize: '10px',
                              color: '#858585',
                              wordBreak: 'break-word',
                              overflow: 'hidden',
                              textOverflow: 'ellipsis',
                              maxHeight: '30px',
                            }}
                            title={sprite.name}
                          >
                            {sprite.name}
                          </div>
                        </div>
                      );
                    })}
                  </div>
                ) : (
                  <div
                    style={{
                      padding: '20px',
                      textAlign: 'center',
                      color: '#858585',
                      backgroundColor: '#1e1e1e',
                      borderRadius: '4px',
                    }}
                  >
                    No sprites found in this spritesheet
                  </div>
                )}
              </div>
            )}

            <div
              style={{
                display: 'flex',
                gap: '10px',
                justifyContent: 'flex-end',
                marginTop: '30px',
              }}
            >
              <Button variant="secondary" onClick={handleCancel}>
                Cancel
              </Button>
              <Button
                variant="primary"
                onClick={handleOk}
                disabled={!selectedSpriteName}
              >
                OK
              </Button>
            </div>
          </div>
        </div>
      )}
    </>
  );
}
