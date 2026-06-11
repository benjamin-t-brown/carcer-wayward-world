import { useEffect, useMemo, useRef, useState } from 'react';
import { CarcerMapTemplate, MapType } from '../types/assets';
import { useAssets } from '../contexts/AssetsContext';
import { GenericModal } from './GenericModal';
import { ModalRowLayout } from './ModalRowLayout';
import { MapPreview } from './MapPreview';
import { MAP_PREVIEW_DISPLAY_SIZE } from '../utils/mapPreview';
import {
  CreateMapModal,
  CreateMapConstraints,
} from '../components/CreateMapModal';
import { prepareNewMapForEditor } from '../utils/mapIndex';
import { openMapEditorInNewTab } from '../utils/mapTabsStorage';
import { Button } from './Button';

export interface MapPickerProps {
  value: string;
  onChange: (mapName: string) => void;
  /** Only list maps with this tile width. */
  requiredWidth?: number;
  /** Only list maps with this tile height. */
  requiredHeight?: number;
  /** Controlled modal open state. */
  isOpen?: boolean;
  onOpenChange?: (open: boolean) => void;
  /** Hide inline trigger; use with controlled isOpen. */
  hideTrigger?: boolean;
  className?: string;
}

type MapTypeFilter = 'all' | MapType;

const MAP_TYPE_FILTER_OPTIONS: { value: MapTypeFilter; label: string }[] = [
  { value: 'all', label: 'All' },
  { value: 'TOWN', label: 'Town' },
  { value: 'OUTDOOR', label: 'Outdoor' },
];

function mapMatchesDimensions(
  map: CarcerMapTemplate,
  requiredWidth?: number,
  requiredHeight?: number
): boolean {
  if (requiredWidth !== undefined && map.width !== requiredWidth) {
    return false;
  }
  if (requiredHeight !== undefined && map.height !== requiredHeight) {
    return false;
  }
  return true;
}

export function MapPicker({
  value,
  onChange,
  requiredWidth,
  requiredHeight,
  isOpen: controlledIsOpen,
  onOpenChange,
  hideTrigger = false,
  className = '',
}: MapPickerProps) {
  const { maps, setMaps, saveMaps } = useAssets();
  const [internalIsOpen, setInternalIsOpen] = useState(false);
  const [searchTerm, setSearchTerm] = useState('');
  const [mapTypeFilter, setMapTypeFilter] = useState<MapTypeFilter>('all');
  const [selectedMapName, setSelectedMapName] = useState(value);
  const [createModalOpen, setCreateModalOpen] = useState(false);
  const [createError, setCreateError] = useState<string | null>(null);
  const [isCreating, setIsCreating] = useState(false);

  const canCreate =
    requiredWidth !== undefined && requiredHeight !== undefined;

  const isOpen = controlledIsOpen ?? internalIsOpen;
  const setIsOpen = (open: boolean) => {
    if (onOpenChange) {
      onOpenChange(open);
    } else {
      setInternalIsOpen(open);
    }
  };

  const eligibleMaps = useMemo(
    () =>
      maps.filter((map) =>
        mapMatchesDimensions(map, requiredWidth, requiredHeight)
      ),
    [maps, requiredWidth, requiredHeight]
  );

  const typeFilteredMaps = useMemo(() => {
    if (mapTypeFilter === 'all') {
      return eligibleMaps;
    }
    return eligibleMaps.filter((map) => map.type === mapTypeFilter);
  }, [eligibleMaps, mapTypeFilter]);

  const filteredMaps = useMemo(() => {
    const term = searchTerm.trim().toLowerCase();
    if (!term) {
      return typeFilteredMaps;
    }
    return typeFilteredMaps.filter(
      (map) =>
        map.name.toLowerCase().includes(term) ||
        map.label.toLowerCase().includes(term)
    );
  }, [typeFilteredMaps, searchTerm]);

  const currentMap = useMemo(
    () => maps.find((map) => map.name === value),
    [maps, value]
  );

  const selectedMap = useMemo(
    () => maps.find((map) => map.name === selectedMapName),
    [maps, selectedMapName]
  );

  const wasOpenRef = useRef(false);
  useEffect(() => {
    if (isOpen && !wasOpenRef.current) {
      setSelectedMapName(value);
      setSearchTerm('');
      setMapTypeFilter('all');
    }
    wasOpenRef.current = isOpen;
  }, [isOpen, value]);

  const handleOpenModal = () => {
    setIsOpen(true);
  };

  const handleConfirm = () => {
    if (selectedMapName) {
      onChange(selectedMapName);
    }
    setIsOpen(false);
  };

  const handleCancel = () => {
    setIsOpen(false);
  };

  const createConstraints = useMemo((): CreateMapConstraints | undefined => {
    if (!canCreate) {
      return undefined;
    }
    const mapType: MapType | undefined =
      mapTypeFilter === 'all' ? undefined : mapTypeFilter;
    return {
      width: requiredWidth,
      height: requiredHeight,
      mapType,
      lockDimensions: true,
      lockType: mapTypeFilter !== 'all',
    };
  }, [canCreate, requiredWidth, requiredHeight, mapTypeFilter]);

  const existingMapNames = useMemo(
    () => maps.map((map) => map.name),
    [maps],
  );

  const handleCreateMap = async (draft: CarcerMapTemplate) => {
    setIsCreating(true);
    setCreateError(null);
    try {
      const newMap = prepareNewMapForEditor(draft);
      const newMaps = [...maps, newMap];
      setMaps(newMaps);
      await saveMaps(newMaps);
      openMapEditorInNewTab(newMap.name);
      onChange(newMap.name);
      setCreateModalOpen(false);
      setIsOpen(false);
    } catch (error) {
      setCreateError(
        error instanceof Error ? error.message : 'Failed to create map',
      );
    } finally {
      setIsCreating(false);
    }
  };

  const dimensionHint =
    requiredWidth !== undefined && requiredHeight !== undefined
      ? `${requiredWidth}×${requiredHeight} tiles`
      : null;

  return (
    <>
      {!hideTrigger && (
        <div
          className={className}
          onClick={handleOpenModal}
          style={{
            display: 'inline-flex',
            flexDirection: 'column',
            alignItems: 'center',
            gap: '6px',
            padding: '10px',
            backgroundColor: '#2d2d30',
            border: '1px solid #3e3e42',
            borderRadius: '4px',
            cursor: 'pointer',
            minWidth: MAP_PREVIEW_DISPLAY_SIZE + 20,
          }}
        >
          {currentMap ? (
            <>
              <MapPreview map={currentMap} />
              <div
                style={{
                  fontSize: '11px',
                  color: '#d4d4d4',
                  textAlign: 'center',
                  wordBreak: 'break-word',
                  maxWidth: MAP_PREVIEW_DISPLAY_SIZE + 20,
                }}
                title={currentMap.name}
              >
                {currentMap.label || currentMap.name}
              </div>
            </>
          ) : (
            <>
              <div
                style={{
                  width: MAP_PREVIEW_DISPLAY_SIZE,
                  height: MAP_PREVIEW_DISPLAY_SIZE,
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                  backgroundColor: '#1e1e1e',
                  borderRadius: '4px',
                  color: '#858585',
                  fontSize: '11px',
                  textAlign: 'center',
                  padding: '8px',
                }}
              >
                No map selected
              </div>
              <div style={{ fontSize: '10px', color: '#858585' }}>
                Click to select
              </div>
            </>
          )}
        </div>
      )}

      {isOpen && (
        <GenericModal
          title="Select Map"
          fillBody
          maxWidth="900px"
          onConfirm={handleConfirm}
          onCancel={handleCancel}
          body={() => (
            <ModalRowLayout
              className="map-picker"
              sidebar={
                <div>
                  <label
                    style={{
                      display: 'block',
                      marginBottom: '6px',
                      color: '#d4d4d4',
                      fontSize: '12px',
                    }}
                  >
                    Search
                  </label>
                  <input
                    type="text"
                    value={searchTerm}
                    onChange={(e) => setSearchTerm(e.target.value)}
                    placeholder="Search maps..."
                    autoFocus
                    style={{
                      width: '100%',
                      boxSizing: 'border-box',
                      padding: '8px 10px',
                      backgroundColor: '#1e1e1e',
                      border: '1px solid #3e3e42',
                      borderRadius: '4px',
                      color: '#d4d4d4',
                      fontSize: '14px',
                    }}
                  />
                  <div style={{ marginTop: '12px' }}>
                    <label
                      style={{
                        display: 'block',
                        marginBottom: '6px',
                        color: '#d4d4d4',
                        fontSize: '12px',
                      }}
                    >
                      Map type
                    </label>
                    <div
                      style={{
                        display: 'flex',
                        gap: '6px',
                        flexWrap: 'wrap',
                      }}
                    >
                      {MAP_TYPE_FILTER_OPTIONS.map((option) => {
                        const isActive = mapTypeFilter === option.value;
                        return (
                          <button
                            key={option.value}
                            type="button"
                            onClick={() => setMapTypeFilter(option.value)}
                            style={{
                              padding: '4px 10px',
                              fontSize: '12px',
                              borderRadius: '4px',
                              border: isActive
                                ? '1px solid #4ec9b0'
                                : '1px solid #3e3e42',
                              backgroundColor: isActive ? '#2d4a45' : '#2d2d30',
                              color: isActive ? '#4ec9b0' : '#d4d4d4',
                              cursor: 'pointer',
                            }}
                          >
                            {option.label}
                          </button>
                        );
                      })}
                    </div>
                  </div>
                  {dimensionHint && (
                    <div
                      style={{
                        marginTop: '10px',
                        fontSize: '11px',
                        color: '#858585',
                      }}
                    >
                      Showing maps sized {dimensionHint}
                    </div>
                  )}
                  <div
                    style={{
                      marginTop: '8px',
                      fontSize: '11px',
                      color: '#858585',
                    }}
                  >
                    {filteredMaps.length} map
                    {filteredMaps.length === 1 ? '' : 's'}
                  </div>
                  {canCreate && (
                    <div style={{ marginTop: '12px' }}>
                      <Button
                        type="button"
                        variant="secondary"
                        onClick={() => {
                          setCreateError(null);
                          setCreateModalOpen(true);
                        }}
                        disabled={isCreating}
                        style={{ width: '100%' }}
                      >
                        + Create new map
                      </Button>
                      {createError && (
                        <div
                          style={{
                            marginTop: '6px',
                            fontSize: '11px',
                            color: '#f48771',
                          }}
                        >
                          {createError}
                        </div>
                      )}
                    </div>
                  )}
                  {selectedMap && (
                    <div
                      style={{
                        marginTop: '12px',
                        padding: '12px',
                        backgroundColor: '#1e1e1e',
                        borderRadius: '4px',
                        textAlign: 'center',
                      }}
                    >
                      <div
                        style={{
                          marginBottom: '8px',
                          color: '#d4d4d4',
                          fontSize: '12px',
                        }}
                      >
                        Preview
                      </div>
                      <div
                        style={{
                          display: 'inline-block',
                          padding: '8px',
                          backgroundColor: '#2d2d30',
                          borderRadius: '4px',
                        }}
                      >
                        <MapPreview map={selectedMap} />
                      </div>
                      <div
                        style={{
                          marginTop: '8px',
                          color: '#858585',
                          fontSize: '11px',
                          wordBreak: 'break-word',
                        }}
                      >
                        {selectedMap.label || selectedMap.name}
                      </div>
                      <div
                        style={{
                          marginTop: '4px',
                          color: '#6a6a6a',
                          fontSize: '10px',
                        }}
                      >
                        {selectedMap.name} · {selectedMap.width}×
                        {selectedMap.height}
                      </div>
                    </div>
                  )}
                </div>
              }
              main={
                filteredMaps.length > 0 ? (
                  <>
                    <label
                      style={{
                        display: 'block',
                        flexShrink: 0,
                        marginBottom: '8px',
                        color: '#d4d4d4',
                        fontSize: '14px',
                        fontWeight: 500,
                      }}
                    >
                      Maps
                    </label>
                    <div
                      style={{
                        flex: 1,
                        minHeight: 0,
                        overflowY: 'auto',
                        display: 'grid',
                        gridTemplateColumns:
                          'repeat(auto-fill, minmax(120px, 1fr))',
                        gap: '12px',
                        padding: '12px',
                        backgroundColor: '#1e1e1e',
                        borderRadius: '4px',
                        alignContent: 'start',
                      }}
                    >
                      {filteredMaps.map((map) => {
                        const isSelected = map.name === selectedMapName;
                        return (
                          <div
                            key={map.name}
                            onClick={() => setSelectedMapName(map.name)}
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
                                e.currentTarget.style.backgroundColor =
                                  '#2d2d30';
                              }
                            }}
                            onMouseLeave={(e) => {
                              if (!isSelected) {
                                e.currentTarget.style.borderColor = '#3e3e42';
                                e.currentTarget.style.backgroundColor =
                                  '#252526';
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
                              <MapPreview map={map} />
                            </div>
                            <div
                              style={{
                                fontSize: '10px',
                                color: '#d4d4d4',
                                wordBreak: 'break-word',
                                overflow: 'hidden',
                                textOverflow: 'ellipsis',
                              }}
                              title={map.name}
                            >
                              {map.label || map.name}
                            </div>
                            <div
                              style={{
                                marginTop: '2px',
                                fontSize: '9px',
                                color: '#6a6a6a',
                              }}
                            >
                              {map.width}×{map.height}
                            </div>
                          </div>
                        );
                      })}
                    </div>
                  </>
                ) : (
                  <div
                    style={{
                      flex: 1,
                      display: 'flex',
                      alignItems: 'center',
                      justifyContent: 'center',
                      padding: '20px',
                      textAlign: 'center',
                      color: '#858585',
                      backgroundColor: '#1e1e1e',
                      borderRadius: '4px',
                    }}
                  >
                    {eligibleMaps.length === 0
                      ? dimensionHint
                        ? `No maps match required size (${dimensionHint})`
                        : 'No maps available'
                      : typeFilteredMaps.length === 0
                        ? `No ${mapTypeFilter === 'TOWN' ? 'town' : 'outdoor'} maps match the required size`
                        : 'No maps match your search'}
                  </div>
                )
              }
            />
          )}
        />
      )}

      <CreateMapModal
        isOpen={createModalOpen}
        onConfirm={handleCreateMap}
        onCancel={() => setCreateModalOpen(false)}
        constraints={createConstraints}
        existingMapNames={existingMapNames}
        isSubmitting={isCreating}
      />
    </>
  );
}
