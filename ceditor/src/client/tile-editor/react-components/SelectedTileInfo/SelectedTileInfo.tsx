import { CarcerMapTemplate, CarcerMapTileTemplate } from '../../../types/assets';
import { useSDL2WAssets } from '../../../contexts/SDL2WAssetsContext';
import { useAssets } from '../../../contexts/AssetsContext';
import { getSpriteNameFromTile } from '../../../utils/draw';
import { Sprite } from '../../../elements/Sprite';
import { EditorState, getEditorState, getEditorStateMap } from '../../editorState';
import { ItemSearchInput } from './ItemSearchInput';
import { CharacterSearchInput } from './CharacterSearchInput';
import { MarkersSection } from './MarkersSection';
import { ItemsList } from './ItemsList';
import { CharactersList } from './CharactersList';
import { TravelTriggerSection } from './TravelTriggerSection';
import { EventTriggerSection } from './EventTriggerSection';
import { TileOverridesSection } from './TileOverridesSection';
import { OpenMapAndSelectTileArgs } from '../../TileEditor';
import { commitCurrentLayer, getTileList } from '../../editorEvents';
import { addMapTileItemEntry } from '../../mapTileItems';

interface SelectedTileInfoProps {
  editorState: EditorState;
  map: CarcerMapTemplate;
  onMapUpdate: (map: CarcerMapTemplate) => void;
  onOpenMapAndSelectTile?: (args: OpenMapAndSelectTileArgs) => void;
}

export function SelectedTileInfo({
  editorState,
  map,
  onMapUpdate,
  onOpenMapAndSelectTile,
}: SelectedTileInfoProps) {
  const { spriteMap } = useSDL2WAssets();
  const { characters, items, gameEvents, maps, tilesets } = useAssets();
  const selectedTileInd =
    getEditorStateMap(editorState.selectedMapName)?.selectedTileInd ?? -1;
  const mapTiles = getTileList(map);

  const updateTile = (updater: (tile: CarcerMapTileTemplate) => void) => {
    if (selectedTileInd < 0 || selectedTileInd >= mapTiles.length) {
      return;
    }

    const updatedTile = { ...mapTiles[selectedTileInd] };
    updater(updatedTile);
    mapTiles[selectedTileInd] = updatedTile;
    commitCurrentLayer(map, getEditorState().currentLevel);
    onMapUpdate({ ...map });
  };

  if (selectedTileInd < 0 || selectedTileInd >= mapTiles.length) {
    return (
      <div
        className="tile-editor-sidebar-panel"
        style={{
          marginTop: '6px',
          padding: '10px',
          backgroundColor: '#2d2d30',
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

  const selectedTile = mapTiles[selectedTileInd];
  const x = selectedTileInd % map.width;
  const y = Math.floor(selectedTileInd / map.width);
  const spriteName = getSpriteNameFromTile(selectedTile);
  const sprite = spriteMap[spriteName];

  return (
    <div
      className="tile-editor-sidebar-panel"
      style={{
        marginTop: '6px',
        padding: '10px',
        backgroundColor: '#2d2d30',
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
        tilesets={tilesets}
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
          dropdownPlacement="auto"
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
          dropdownPlacement="above"
          onSelect={(itemName) => {
            const template = items.find((i) => i.name === itemName);
            updateTile((tile) => {
              tile.items = addMapTileItemEntry(
                tile.items,
                itemName,
                Boolean(template?.stackable)
              );
            });
          }}
        />

        <ItemsList
          selectedTile={selectedTile}
          items={items}
          updateTile={updateTile}
        />
      </div>

      <MarkersSection
        map={map}
        maps={maps}
        selectedTile={selectedTile}
        updateTile={updateTile}
        onOpenMapAndSelectTile={onOpenMapAndSelectTile}
      />
    </div>
  );
}
