import { useState, useMemo } from 'react';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { Sprite } from './Sprite';
import { OptionSelect } from './OptionSelect';
import { ModalRowLayout } from './ModalRowLayout';
import { Sprite as SpriteType } from '../utils/assetLoader';
import { GenericModal } from './GenericModal';

interface SpritePickerProps {
  value: string;
  onChange: (spriteName: string) => void;
  scale?: number;
  className?: string;
  maxHeight?: string;
}

const calculateScale = (sprite: SpriteType) => {
  if (sprite.width >= 64) {
    return 1;
  }
  if (sprite.width >= 50) {
    return 2;
  }
  return 3;
};

export function SpritePicker({
  value,
  onChange,
  scale = 2,
  className = '',
  maxHeight = '400px',
}: SpritePickerProps) {
  const { sprites, spriteMap } = useSDL2WAssets();
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [selectedSpritesheet, setSelectedSpritesheet] = useState<string>('');
  const [selectedSpriteName, setSelectedSpriteName] = useState<string>(value);

  // Find the current sprite
  const currentSprite = useMemo(() => {
    return spriteMap[value];
  }, [spriteMap, value]);

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
    return spriteMap[selectedSpriteName];
  }, [spriteMap, selectedSpriteName]);

  if (!currentSprite && !isModalOpen) {
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

  const spriteCanvases: React.ReactNode[] = [];
  for (let i = 0; i < sheetSprites.length; i++) {
    const sprite = sheetSprites[i];
    const isSelected = sprite.name === selectedSpriteName;
    spriteCanvases.push(
      <div
        key={`${selectedSpritesheet}-${sprite.name}-${i}`}
        onClick={() => setSelectedSpriteName(sprite.name)}
        style={{
          cursor: 'pointer',
          padding: '8px',
          backgroundColor: isSelected ? '#2d2d30' : '#252526',
          border: isSelected ? '2px solid #4ec9b0' : '2px solid #3e3e42',
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
          <Sprite sprite={sprite} scale={2} maxWidth={50} />
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
    if (i > 255) {
      break;
    }
    // break;
  }

  return (
    <>
      {currentSprite && (
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
      )}

      {isModalOpen && (
        <GenericModal
          title="Select Sprite"
          onConfirm={handleOk}
          onCancel={handleCancel}
          body={() => (
            <>
              <ModalRowLayout
                fixedHeight={400}
                fixedContent={
                  <div onClick={(e) => e.stopPropagation()}>
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
                          <Sprite
                            sprite={previewSprite}
                            scale={calculateScale(previewSprite)}
                          />
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
                  </div>
                }
                resizableContent={
                  <div
                    style={{
                      width: '100%',
                      height: '100%',
                      marginBottom: '20px',
                    }}
                  >
                    {selectedSpritesheet && (
                      <>
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
                              height: 'calc(100% - 60px)',
                              overflowY: 'auto',
                            }}
                          >
                            {spriteCanvases}
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
                      </>
                    )}
                  </div>
                }
              />
            </>
          )}
        ></GenericModal>
      )}
    </>
  );
}
