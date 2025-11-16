import { useState, useMemo } from 'react';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { Button } from './Button';
import { loadImage } from '../utils/spriteUtils';

interface PicturePickerProps {
  value: string;
  onChange: (pictureName: string, width: number, height: number) => void;
  className?: string;
}

export function PicturePicker({
  value,
  onChange,
  className = '',
}: PicturePickerProps) {
  const { pictures } = useSDL2WAssets();
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [selectedPicture, setSelectedPicture] = useState<string>(value);
  const [searchTerm, setSearchTerm] = useState('');
  const [imageDimensions, setImageDimensions] = useState<{
    width: number;
    height: number;
  } | null>(null);
  // Get picture options
  const pictureOptions = useMemo(() => {
    return Object.keys(pictures).map((alias) => ({
      value: alias,
      label: alias,
    }));
  }, [pictures]);

  // Filter pictures based on search term
  const filteredPictures = useMemo(() => {
    if (!searchTerm) return pictureOptions;
    const term = searchTerm.toLowerCase();
    return pictureOptions.filter(
      (opt) =>
        opt.value.toLowerCase().includes(term) ||
        opt.label.toLowerCase().includes(term)
    );
  }, [pictureOptions, searchTerm]);

  // Initialize modal state when opened
  const handleOpenModal = () => {
    setSelectedPicture(value);
    setSearchTerm('');
    setIsModalOpen(true);
  };

  const handleCloseModal = () => {
    setIsModalOpen(false);
  };

  const handleOk = () => {
    if (selectedPicture) {
      onChange(
        selectedPicture,
        imageDimensions?.width || 0,
        imageDimensions?.height || 0
      );
    }
    setIsModalOpen(false);
  };

  const handleCancel = () => {
    setIsModalOpen(false);
  };

  // Get the picture path for preview
  const picturePath = useMemo(() => {
    if (!selectedPicture || !pictures[selectedPicture]) return null;
    return `/api/${pictures[selectedPicture]}`;
  }, [selectedPicture, pictures]);

  return (
    <>
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
          {value || 'No picture selected'}
        </div>
        <div style={{ color: '#858585', fontSize: '10px', marginTop: '4px' }}>
          Click to select
        </div>
      </div>

      {isModalOpen && (
        <div
          style={{
            position: 'absolute',
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
              position: 'relative',
              maxWidth: '800px',
              width: '90%',
              height: '90%',
              maxHeight: '80vh',
            }}
          >
            <div
              style={{
                backgroundColor: '#252526',
                border: '1px solid #3e3e42',
                borderRadius: '8px',
                padding: '30px',
                overflow: 'auto',
                height: 'calc(100% - 75px)',
              }}
              onClick={(e) => e.stopPropagation()}
            >
              <h2
                style={{ color: '#4ec9b0', marginBottom: '20px', marginTop: 0 }}
              >
                Select Picture
              </h2>

              <div style={{ marginBottom: '20px' }}>
                <input
                  type="text"
                  value={searchTerm}
                  onChange={(e) => setSearchTerm(e.target.value)}
                  placeholder="Search pictures..."
                  style={{
                    width: '100%',
                    padding: '10px',
                    background: '#1e1e1e',
                    border: '1px solid #3e3e42',
                    borderRadius: '4px',
                    color: '#d4d4d4',
                    fontSize: '14px',
                  }}
                />
              </div>

              {picturePath && selectedPicture && (
                <div
                  style={{
                    marginBottom: '20px',
                    padding: '20px',
                    backgroundColor: '#1e1e1e',
                    borderRadius: '4px',
                    textAlign: 'center',
                  }}
                >
                  <div style={{ marginBottom: '10px', color: '#d4d4d4' }}>
                    Preview:
                  </div>
                  <img
                    src={picturePath}
                    alt={selectedPicture}
                    style={{
                      maxWidth: '100%',
                      maxHeight: '300px',
                      border: '1px solid #3e3e42',
                      borderRadius: '4px',
                      imageRendering: 'pixelated',
                    }}
                    onLoad={(e) => {
                      const img = e.currentTarget;
                      console.log(
                        'Picture loaded:',
                        selectedPicture,
                        'Dimensions:',
                        img.naturalWidth,
                        'x',
                        img.naturalHeight
                      );
                      setImageDimensions({
                        width: img.naturalWidth,
                        height: img.naturalHeight,
                      });
                    }}
                  />
                  <div
                    style={{
                      marginTop: '10px',
                      color: '#858585',
                      fontSize: '12px',
                    }}
                  >
                    {selectedPicture}
                  </div>
                </div>
              )}

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
                  Picture
                </label>
                {filteredPictures.length > 0 ? (
                  <div
                    style={{
                      display: 'grid',
                      gridTemplateColumns:
                        'repeat(auto-fill, minmax(150px, 1fr))',
                      gap: '12px',
                      padding: '15px',
                      backgroundColor: '#1e1e1e',
                      borderRadius: '4px',
                      maxHeight: '400px',
                      overflowY: 'auto',
                    }}
                  >
                    {filteredPictures.map((option) => {
                      const isSelected = option.value === selectedPicture;
                      const path = pictures[option.value];
                      return (
                        <div
                          key={option.value}
                          onClick={() => setSelectedPicture(option.value)}
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
                          {path && (
                            <img
                              src={`/api/${path}`}
                              alt={option.value}
                              style={{
                                width: '100%',
                                height: 'auto',
                                maxHeight: '100px',
                                objectFit: 'contain',
                                marginBottom: '6px',
                                imageRendering: 'pixelated',
                              }}
                            />
                          )}
                          <div
                            style={{
                              fontSize: '10px',
                              color: '#858585',
                              wordBreak: 'break-word',
                              overflow: 'hidden',
                              textOverflow: 'ellipsis',
                            }}
                            title={option.value}
                          >
                            {option.value}
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
                    No pictures found
                  </div>
                )}
              </div>

              <div
                style={{
                  position: 'absolute',
                  bottom: '8px',
                  right: '8px',
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
                  disabled={!selectedPicture}
                >
                  OK
                </Button>
              </div>
            </div>
          </div>
        </div>
      )}
    </>
  );
}
