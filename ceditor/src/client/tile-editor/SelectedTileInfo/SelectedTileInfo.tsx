import { CarcerMapTemplate, CarcerMapTileTemplate } from '../../types/assets';
import { useSDL2WAssets } from '../../contexts/SDL2WAssetsContext';
import { useAssets } from '../../contexts/AssetsContext';
import { getSpriteNameFromTile } from '../draw';
import { Sprite } from '../../elements/Sprite';
import { EditorState, getEditorStateMap } from '../editorState';
import { ItemSearchInput } from './ItemSearchInput';
import { CharacterSearchInput } from './CharacterSearchInput';
import { MarkersSection } from './MarkersSection';
import { ItemsList } from './ItemsList';
import { CharactersList } from './CharactersList';
import { TravelTriggerSection } from './TravelTriggerSection';
import { EventTriggerSection } from './EventTriggerSection';
import { TileOverridesSection } from './TileOverridesSection';

interface SelectedTileInfoProps {
  editorState: EditorState;
  map: CarcerMapTemplate;
  onMapUpdate: (map: CarcerMapTemplate) => void;
  onOpenMapAndSelectTile?: (
    mapName: string,
    markerName?: string,
    x?: number,
    y?: number
  ) => void;
}

export function SelectedTileInfo({
  editorState,
  map,
  onMapUpdate,
  onOpenMapAndSelectTile,
}: SelectedTileInfoProps) {
  const { spriteMap } = useSDL2WAssets();
  const { characters, items, gameEvents, maps } = useAssets();
  const selectedTileInd = getEditorStateMap(editorState.selectedMapName)?.selectedTileInd ?? -1;

  const updateTile = (updater: (tile: CarcerMapTileTemplate) => void) => {
    if (selectedTileInd < 0 || selectedTileInd >= map.tiles.length) {
      return;
    }

    const updatedTiles = [...map.tiles];
    const updatedTile = { ...updatedTiles[selectedTileInd] };
    updater(updatedTile);
    updatedTiles[selectedTileInd] = updatedTile;

    onMapUpdate({
      ...map,
      tiles: updatedTiles,
    });
  };

  if (selectedTileInd < 0 || selectedTileInd >= map.tiles.length) {
    return (
      <div
        style={{
          marginTop: '20px',
          padding: '10px',
          backgroundColor: '#2d2d30',
          borderRadius: '4px',
          border: '1px solid #3e3e42',
        }}
      >
        <div
          style={{
            color: '#858585',
            fontSize: '11px',
            textTransform: 'uppercase',
            fontWeight: 'bold',
            marginBottom: '8px',
          }}
        >
          Selected Tile
        </div>
        <div style={{ color: '#858585', fontSize: '12px' }}>
          No tile selected
        </div>
      </div>
    );
  }

  const selectedTile = map.tiles[selectedTileInd];
  const x = selectedTileInd % map.width;
  const y = Math.floor(selectedTileInd / map.width);
  const spriteName = getSpriteNameFromTile(selectedTile);
  const sprite = spriteMap[spriteName];

  return (
    <div
      style={{
        marginTop: '6px',
        padding: '10px',
        backgroundColor: '#2d2d30',
        borderRadius: '4px',
        border: '1px solid #3e3e42',
      }}
    >
      <div
        style={{
          color: '#858585',
          fontSize: '11px',
          textTransform: 'uppercase',
          fontWeight: 'bold',
          marginBottom: '8px',
        }}
      >
        Selected Tile
      </div>
      <div
        style={{
          display: 'flex',
          flexDirection: 'column',
          gap: '6px',
          fontSize: '12px',
        }}
      >
        <div style={{ display: 'flex', justifyContent: 'space-between' }}>
          <span style={{ color: '#858585' }}>Index:</span>
          <span style={{ color: '#ffffff' }}>{selectedTileInd}</span>
        </div>
        <div style={{ display: 'flex', justifyContent: 'space-between' }}>
          <span style={{ color: '#858585' }}>Coordinates:</span>
          <span style={{ color: '#ffffff' }}>
            ({x}, {y})
          </span>
        </div>
        <div style={{ display: 'flex', justifyContent: 'space-between' }}>
          <span style={{ color: '#858585' }}>Sprite:</span>
          <span
            style={{
              color: '#ffffff',
              maxWidth: '150px',
              overflow: 'hidden',
              textOverflow: 'ellipsis',
              whiteSpace: 'nowrap',
            }}
            title={spriteName}
          >
            {spriteName || 'None'}
          </span>
        </div>
        {sprite && (
          <div
            style={{
              marginTop: '8px',
              display: 'flex',
              justifyContent: 'center',
              alignItems: 'center',
              padding: '8px',
              backgroundColor: '#1e1e1e',
              borderRadius: '4px',
            }}
          >
            <Sprite sprite={sprite} scale={2} maxWidth={64} />
          </div>
        )}
      </div>

      <TileOverridesSection
        selectedTile={selectedTile}
        updateTile={updateTile}
      />

      <TravelTriggerSection
        selectedTile={selectedTile}
        maps={maps}
        updateTile={updateTile}
        onOpenMapAndSelectTile={onOpenMapAndSelectTile}
      />

      <EventTriggerSection
        selectedTile={selectedTile}
        gameEvents={gameEvents}
        updateTile={updateTile}
      />

      {/* Characters Section */}
      <div
        style={{
          marginTop: '15px',
          paddingTop: '15px',
          borderTop: '1px solid #3e3e42',
        }}
      >
        <div
          style={{
            color: '#858585',
            fontSize: '11px',
            textTransform: 'uppercase',
            fontWeight: 'bold',
            marginBottom: '10px',
          }}
        >
          Characters
        </div>

        {/* Character Search Input */}
        <CharacterSearchInput
          characters={characters}
          onSelect={(characterName) => {
            updateTile((tile) => {
              if (!tile.characters.includes(characterName)) {
                tile.characters = [...tile.characters, characterName];
              }
            });
          }}
        />

        <CharactersList
          selectedTile={selectedTile}
          characters={characters}
          updateTile={updateTile}
        />
      </div>

      {/* Items Section */}
      <div
        style={{
          marginTop: '15px',
          paddingTop: '15px',
          borderTop: '1px solid #3e3e42',
        }}
      >
        <div
          style={{
            color: '#858585',
            fontSize: '11px',
            textTransform: 'uppercase',
            fontWeight: 'bold',
            marginBottom: '10px',
          }}
        >
          Items
        </div>

        {/* Item Search Input */}
        <ItemSearchInput
          items={items}
          onSelect={(itemName) => {
            updateTile((tile) => {
              if (!tile.items.includes(itemName)) {
                tile.items = [...tile.items, itemName];
              }
            });
          }}
        />

        <ItemsList
          selectedTile={selectedTile}
          items={items}
          updateTile={updateTile}
        />
      </div>

      <MarkersSection selectedTile={selectedTile} updateTile={updateTile} />
    </div>
  );
}
