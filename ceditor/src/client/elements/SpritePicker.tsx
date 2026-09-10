import { useMemo, useState } from 'react';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { useIncrementalMedia } from '../hooks/useIncrementalMedia';
import { filterMediaChoices } from '../utils/mediaPicker';
import { MediaPickerModal } from './MediaPickerModal';
import { OptionSelect } from './OptionSelect';
import { Sprite } from './Sprite';

interface SpritePickerProps {
  value: string;
  onChange: (spriteName: string) => void;
  scale?: number;
  className?: string;
  /** Spritesheet pre-selected when opening the picker with no current value. */
  defaultSpritesheet?: string;
}

const SPRITE_PICKER_PREVIEW_SIZE = 128;
const SPRITE_PICKER_CELL_SIZE = 64;

export function SpritePicker({
  value,
  onChange,
  scale = 2,
  className = '',
  defaultSpritesheet,
}: SpritePickerProps) {
  const { sprites, spriteMap } = useSDL2WAssets();
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [selectedSpritesheet, setSelectedSpritesheet] = useState('');
  const [selectedSpriteName, setSelectedSpriteName] = useState(value);
  const [searchTerm, setSearchTerm] = useState('');
  const currentSprite = spriteMap[value];

  const spritesBySheet = useMemo(() => {
    const grouped: Record<string, typeof sprites> = {};
    for (const sprite of sprites) {
      (grouped[sprite.pictureAlias] ??= []).push(sprite);
    }
    return grouped;
  }, [sprites]);
  const spritesheetOptions = useMemo(
    () =>
      Object.keys(spritesBySheet)
        .sort((left, right) => left.localeCompare(right))
        .map((alias) => ({ value: alias, label: alias })),
    [spritesBySheet],
  );
  const filteredSprites = useMemo(
    () =>
      filterMediaChoices(
        selectedSpritesheet ? (spritesBySheet[selectedSpritesheet] ?? []) : [],
        searchTerm,
        (sprite) => [sprite.name, sprite.pictureAlias, sprite.picturePath],
      ),
    [searchTerm, selectedSpritesheet, spritesBySheet],
  );
  const { visibleChoices, hasMore, showMore } = useIncrementalMedia(
    filteredSprites,
    `${selectedSpritesheet}\u0000${searchTerm}`,
  );
  const previewSprite = spriteMap[selectedSpriteName];
  const legacyValue =
    selectedSpriteName && !previewSprite ? selectedSpriteName : undefined;

  const resolveInitialSpritesheet = () => {
    if (defaultSpritesheet && spritesBySheet[defaultSpritesheet]?.length) {
      return defaultSpritesheet;
    }
    return spritesheetOptions[0]?.value ?? '';
  };

  const handleOpenModal = () => {
    setSearchTerm('');
    setSelectedSpriteName(value);
    if (currentSprite) {
      setSelectedSpritesheet(currentSprite.pictureAlias);
    } else {
      const sheet = resolveInitialSpritesheet();
      setSelectedSpritesheet(sheet);
      if (!value) {
        setSelectedSpriteName(spritesBySheet[sheet]?.[0]?.name ?? '');
      }
    }
    setIsModalOpen(true);
  };

  const handleSpritesheetChange = (spritesheet: string) => {
    setSelectedSpritesheet(spritesheet);
    setSearchTerm('');
    setSelectedSpriteName(spritesBySheet[spritesheet]?.[0]?.name ?? '');
  };

  const confirm = () => {
    if (selectedSpriteName) {
      onChange(selectedSpriteName);
    }
    setIsModalOpen(false);
  };

  return (
    <>
      <button
        type="button"
        className={`media-picker-trigger ${className}`.trim()}
        onClick={handleOpenModal}
      >
        {currentSprite ? (
          <Sprite sprite={currentSprite} scale={scale} />
        ) : (
          <>
            <span>{value || 'No sprite selected'}</span>
            <small>Click to select</small>
          </>
        )}
      </button>

      {isModalOpen ? (
        <MediaPickerModal
          title="Select Sprite"
          searchTerm={searchTerm}
          searchPlaceholder="Search sprites..."
          onSearchTermChange={setSearchTerm}
          selectedName={selectedSpriteName}
          legacyValue={legacyValue}
          resultCount={filteredSprites.length}
          visibleCount={visibleChoices.length}
          hasMore={hasMore}
          onShowMore={showMore}
          onConfirm={confirm}
          onCancel={() => setIsModalOpen(false)}
          sidebar={
            <>
              <OptionSelect
                label="Spritesheet"
                value={selectedSpritesheet}
                onChange={handleSpritesheetChange}
                options={spritesheetOptions}
              />
              {previewSprite ? (
                <div className="media-picker-selected-preview">
                  <span>Preview</span>
                  <Sprite
                    sprite={previewSprite}
                    displaySize={SPRITE_PICKER_PREVIEW_SIZE}
                  />
                  <small>{previewSprite.name}</small>
                </div>
              ) : null}
            </>
          }
        >
          {visibleChoices.map((sprite) => (
            <button
              type="button"
              className="media-picker-card"
              aria-selected={sprite.name === selectedSpriteName}
              key={`${sprite.pictureAlias}-${sprite.name}`}
              title={sprite.name}
              onClick={() => setSelectedSpriteName(sprite.name)}
              onDoubleClick={() => {
                onChange(sprite.name);
                setIsModalOpen(false);
              }}
            >
              <span className="media-picker-card-visual">
                <Sprite sprite={sprite} displaySize={SPRITE_PICKER_CELL_SIZE} />
              </span>
              <small>{sprite.name}</small>
            </button>
          ))}
          {filteredSprites.length === 0 ? (
            <p className="media-picker-empty">No sprites found</p>
          ) : null}
        </MediaPickerModal>
      ) : null}
    </>
  );
}
