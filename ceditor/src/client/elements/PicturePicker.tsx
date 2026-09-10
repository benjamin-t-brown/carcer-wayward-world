import { useMemo, useState } from 'react';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { useIncrementalMedia } from '../hooks/useIncrementalMedia';
import { filterMediaChoices } from '../utils/mediaPicker';
import { loadImage } from '../utils/spriteUtils';
import { MediaPickerModal } from './MediaPickerModal';

interface PicturePickerProps {
  value: string;
  onChange: (pictureName: string, width: number, height: number) => void;
  className?: string;
}

interface PictureChoice {
  name: string;
  path: string;
}

export function PicturePicker({
  value,
  onChange,
  className = '',
}: PicturePickerProps) {
  const { pictures } = useSDL2WAssets();
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [selectedPicture, setSelectedPicture] = useState(value);
  const [searchTerm, setSearchTerm] = useState('');
  const [imageDimensions, setImageDimensions] = useState<{
    width: number;
    height: number;
  } | null>(null);

  const choices = useMemo(
    () =>
      Object.entries(pictures)
        .map(([name, path]) => ({ name, path }))
        .sort((left, right) => left.name.localeCompare(right.name)),
    [pictures],
  );
  const filteredPictures = useMemo(
    () =>
      filterMediaChoices(choices, searchTerm, (choice) => [
        choice.name,
        choice.path,
      ]),
    [choices, searchTerm],
  );
  const { visibleChoices, hasMore, showMore } = useIncrementalMedia(
    filteredPictures,
    searchTerm,
  );
  const selectedPath = selectedPicture ? pictures[selectedPicture] : undefined;
  const legacyValue =
    selectedPicture && !selectedPath ? selectedPicture : undefined;

  const selectPicture = (name: string) => {
    setSelectedPicture(name);
    setImageDimensions(null);
  };

  const choosePicture = async (choice: PictureChoice) => {
    try {
      const image = await loadImage(`/api/${choice.path}`);
      onChange(choice.name, image.naturalWidth, image.naturalHeight);
    } catch {
      onChange(
        choice.name,
        choice.name === selectedPicture ? (imageDimensions?.width ?? 0) : 0,
        choice.name === selectedPicture ? (imageDimensions?.height ?? 0) : 0,
      );
    }
    setIsModalOpen(false);
  };

  const confirm = async () => {
    if (!selectedPicture) {
      return;
    }
    const path = pictures[selectedPicture];
    if (path) {
      await choosePicture({ name: selectedPicture, path });
      return;
    }
    // Confirming an unavailable legacy value leaves the underlying data intact.
    setIsModalOpen(false);
  };

  const preview = selectedPath ? (
    <div className="media-picker-selected-preview">
      <span>Preview</span>
      <img
        src={`/api/${selectedPath}`}
        alt={selectedPicture}
        onLoad={(event) =>
          setImageDimensions({
            width: event.currentTarget.naturalWidth,
            height: event.currentTarget.naturalHeight,
          })
        }
      />
      <small>{selectedPicture}</small>
    </div>
  ) : null;

  return (
    <>
      <button
        type="button"
        className={`media-picker-trigger ${className}`.trim()}
        onClick={() => {
          setSelectedPicture(value);
          setSearchTerm('');
          setImageDimensions(null);
          setIsModalOpen(true);
        }}
      >
        <span>{value || 'No picture selected'}</span>
        <small>Click to select</small>
      </button>

      {isModalOpen ? (
        <MediaPickerModal
          title="Select Picture"
          searchTerm={searchTerm}
          searchPlaceholder="Search pictures..."
          onSearchTermChange={setSearchTerm}
          selectedName={selectedPicture}
          legacyValue={legacyValue}
          resultCount={filteredPictures.length}
          visibleCount={visibleChoices.length}
          hasMore={hasMore}
          onShowMore={showMore}
          onConfirm={() => void confirm()}
          onCancel={() => setIsModalOpen(false)}
          sidebar={preview}
        >
          {visibleChoices.map((choice) => (
            <button
              type="button"
              className="media-picker-card media-picker-picture-card"
              aria-selected={choice.name === selectedPicture}
              key={choice.name}
              title={choice.name}
              onClick={() => selectPicture(choice.name)}
              onDoubleClick={() => void choosePicture(choice)}
            >
              <span className="media-picker-card-visual">
                <img
                  src={`/api/${choice.path}`}
                  alt=""
                  loading="lazy"
                  decoding="async"
                />
              </span>
              <small>{choice.name}</small>
            </button>
          ))}
          {filteredPictures.length === 0 ? (
            <p className="media-picker-empty">No pictures found</p>
          ) : null}
        </MediaPickerModal>
      ) : null}
    </>
  );
}
