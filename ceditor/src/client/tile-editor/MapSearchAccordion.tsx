import { useMemo, useState, type ReactNode } from 'react';
import { CarcerMapTemplate } from '../types/assets';
import { Button } from '../elements/Button';
import { useAssets } from '../contexts/AssetsContext';
import { CharacterSearchInput } from './SelectedTileInfo/CharacterSearchInput';
import { MarkerSearchInput } from './SelectedTileInfo/MarkerSearchInput';
import {
  collectCharacterNamesOnMap,
  collectMarkerNamesOnMap,
  findCharacterOnMap,
  findMarkerOnMap,
  locateOnCurrentMap,
} from './mapLocate';

interface MapSearchAccordionProps {
  map: CarcerMapTemplate;
}

type SearchSectionId = 'markers' | 'characters';

function LocateSection({
  id,
  title,
  map,
  isOpen,
  onToggle,
  searchInput,
}: {
  id: SearchSectionId;
  title: string;
  map: CarcerMapTemplate;
  isOpen: boolean;
  onToggle: () => void;
  searchInput: (props: {
    inputKey: number;
    onSelect: (name: string) => void;
    onEnter: (name: string) => void;
    onSearchTermChange: (term: string) => void;
  }) => ReactNode;
}) {
  const [query, setQuery] = useState('');
  const [message, setMessage] = useState('');
  const [inputKey, setInputKey] = useState(0);

  const handleLocate = (name: string) => {
    const trimmed = name.trim();
    if (!trimmed) {
      setMessage('Enter a name to search.');
      return;
    }

    const location =
      id === 'markers'
        ? findMarkerOnMap(map, trimmed)
        : findCharacterOnMap(map, trimmed);

    if (!location) {
      setMessage(
        `No ${id === 'markers' ? 'marker' : 'character'} "${trimmed}" on this map.`
      );
      return;
    }

    locateOnCurrentMap(map, location);
    setMessage('');
    setQuery('');
    setInputKey((k) => k + 1);
  };

  const searchProps = {
    onSelect: handleLocate,
    onEnter: handleLocate,
    onSearchTermChange: (term: string) => {
      setQuery(term);
      if (message) {
        setMessage('');
      }
    },
  };

  return (
    <div className="tile-editor-search-section">
      <button
        type="button"
        className="tile-editor-search-section-header"
        onClick={onToggle}
        aria-expanded={isOpen}
      >
        <span>{title}</span>
        <span className="tile-editor-search-section-chevron" aria-hidden>
          {isOpen ? '▾' : '▸'}
        </span>
      </button>
      {isOpen && (
        <div className="tile-editor-search-section-body">
          {searchInput({ ...searchProps, inputKey })}
          <Button
            variant="small"
            type="button"
            onClick={() => handleLocate(query)}
          >
            Locate
          </Button>
          {message ? (
            <div className="tile-editor-search-message">{message}</div>
          ) : null}
        </div>
      )}
    </div>
  );
}

export function MapSearchAccordion({ map }: MapSearchAccordionProps) {
  const { characters } = useAssets();
  const markerNames = useMemo(() => collectMarkerNamesOnMap(map), [map]);
  const charactersOnMap = useMemo(() => {
    const namesOnMap = new Set(collectCharacterNamesOnMap(map));
    return characters.filter((c) => namesOnMap.has(c.name));
  }, [map, characters]);
  const [openSection, setOpenSection] = useState<SearchSectionId | null>(null);

  const toggle = (id: SearchSectionId) => {
    setOpenSection((prev) => (prev === id ? null : id));
  };

  return (
    <div className="tile-editor-map-search">
      <div className="tile-editor-map-search-label">Find on map</div>
      <LocateSection
        id="markers"
        title="Markers"
        map={map}
        isOpen={openSection === 'markers'}
        onToggle={() => toggle('markers')}
        searchInput={({ inputKey, onSelect, onEnter, onSearchTermChange }) => (
          <MarkerSearchInput
            key={inputKey}
            markerNames={markerNames}
            placeholder="Search markers on map..."
            dropdownPlacement="auto"
            onSelect={onSelect}
            onEnter={onEnter}
            onSearchTermChange={onSearchTermChange}
          />
        )}
      />
      <LocateSection
        id="characters"
        title="Characters"
        map={map}
        isOpen={openSection === 'characters'}
        onToggle={() => toggle('characters')}
        searchInput={({ inputKey, onSelect, onEnter, onSearchTermChange }) => (
          <CharacterSearchInput
            key={inputKey}
            characters={charactersOnMap}
            placeholder="Search characters on map..."
            dropdownPlacement="auto"
            onSelect={onSelect}
            onEnter={onEnter}
            onSearchTermChange={onSearchTermChange}
          />
        )}
      />
    </div>
  );
}
