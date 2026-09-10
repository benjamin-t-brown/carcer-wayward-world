import { useMemo, useState } from 'react';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { useAnimationClock } from '../hooks/useAnimationClock';
import { useIncrementalMedia } from '../hooks/useIncrementalMedia';
import { filterMediaChoices } from '../utils/mediaPicker';
import { AnimationPreview } from './AnimationPreview';
import { MediaPickerModal } from './MediaPickerModal';

interface AnimationPickerProps {
  id?: string;
  name?: string;
  label?: string;
  value: string;
  onChange: (animationName: string) => void;
  className?: string;
}

const ANIMATION_CARD_SIZE = 64;
const ANIMATION_PREVIEW_SIZE = 128;

export function AnimationPicker({
  id,
  name,
  label,
  value,
  onChange,
  className = '',
}: AnimationPickerProps) {
  const { animations, animationMap } = useSDL2WAssets();
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [selectedName, setSelectedName] = useState(value);
  const [searchTerm, setSearchTerm] = useState('');
  const clockMs = useAnimationClock(isModalOpen);
  const filteredAnimations = useMemo(
    () =>
      filterMediaChoices(animations, searchTerm, (animation) => [
        animation.name,
        ...animation.frames.map((frame) => frame.spriteName),
      ]),
    [animations, searchTerm],
  );
  const { visibleChoices, hasMore, showMore } = useIncrementalMedia(
    filteredAnimations,
    searchTerm,
  );
  const selectedAnimation = animationMap[selectedName];
  const legacyValue =
    selectedName && !selectedAnimation ? selectedName : undefined;

  const open = () => {
    setSelectedName(value || animations[0]?.name || '');
    setSearchTerm('');
    setIsModalOpen(true);
  };
  const confirm = () => {
    if (selectedName) {
      onChange(selectedName);
    }
    setIsModalOpen(false);
  };

  return (
    <div className={`form-group media-picker-field ${className}`.trim()}>
      {label ? <label htmlFor={id}>{label}</label> : null}
      {name ? <input type="hidden" name={name} value={value} /> : null}
      <button
        id={id}
        type="button"
        className="media-picker-trigger media-picker-animation-trigger"
        onClick={open}
      >
        <AnimationPreview animationName={value} />
        <span>{value || 'No animation selected'}</span>
      </button>

      {isModalOpen ? (
        <MediaPickerModal
          title="Select Animation"
          searchTerm={searchTerm}
          searchPlaceholder="Search animations..."
          onSearchTermChange={setSearchTerm}
          selectedName={selectedName}
          legacyValue={legacyValue}
          resultCount={filteredAnimations.length}
          visibleCount={visibleChoices.length}
          hasMore={hasMore}
          onShowMore={showMore}
          onConfirm={confirm}
          onCancel={() => setIsModalOpen(false)}
          sidebar={
            selectedAnimation ? (
              <div className="media-picker-selected-preview">
                <span>Preview</span>
                <AnimationPreview
                  animationName={selectedName}
                  displaySize={ANIMATION_PREVIEW_SIZE}
                  clockMs={clockMs}
                />
                <small>{selectedName}</small>
              </div>
            ) : null
          }
        >
          {visibleChoices.map((animation) => (
            <button
              type="button"
              className="media-picker-card"
              aria-selected={animation.name === selectedName}
              key={animation.name}
              title={animation.name}
              onClick={() => setSelectedName(animation.name)}
              onDoubleClick={() => {
                onChange(animation.name);
                setIsModalOpen(false);
              }}
            >
              <span className="media-picker-card-visual">
                <AnimationPreview
                  animationName={animation.name}
                  displaySize={ANIMATION_CARD_SIZE}
                  clockMs={clockMs}
                />
              </span>
              <small>{animation.name}</small>
            </button>
          ))}
          {filteredAnimations.length === 0 ? (
            <p className="media-picker-empty">No animations found</p>
          ) : null}
        </MediaPickerModal>
      ) : null}
    </div>
  );
}
