import {
  createTilesForLayer,
  EditorState,
  setCurrentPaintAction,
  updateEditorState,
} from './editorState';
import { PaintActionType } from './paintTools';
import { CarcerMapTemplate } from '../types/assets';
import { SelectedTileInfo } from './SelectedTileInfo';
import { OpenMapAndSelectTileArgs } from './TileEditor';
import { Button } from '../elements/Button';
import { useState } from 'react';
import { DeleteModal } from '../elements/DeleteModal';

interface LayersPanelProps {
  editorState: EditorState;
  map: CarcerMapTemplate;
  onMapUpdate: (map: CarcerMapTemplate) => void;
}

const LayerButton = (props: {
  onClick: () => void;
  disabled?: boolean;
  color: string;
  children: React.ReactNode;
}) => {
  return (
    <button
      onClick={props.onClick}
      disabled={props.disabled}
      style={{
        color: 'white',
        padding: '2px',
        fontWeight: 'bold',
        fontSize: '16px',
        background: props.disabled ? '#3e3e42' : props.color,
      }}
    >
      {props.children}
    </button>
  );
};

export function LayersPanel(props: LayersPanelProps) {
  const [showDeleteConfirm, setShowDeleteConfirm] = useState(false);

  const handleAddNewLayer = (where: 'above' | 'below') => {
    const highestLayer = Math.max(...Object.keys(props.map.levels).map(Number));
    const lowestLayer = Math.min(...Object.keys(props.map.levels).map(Number));
    const newLayerKey = where === 'above' ? highestLayer + 1 : lowestLayer - 1;
    props.onMapUpdate({
      ...props.map,
      levels: {
        ...props.map.levels,
        [newLayerKey]: createTilesForLayer(props.map),
      },
    });
  };

  const handleSelectLayer = (layer: string) => {
    updateEditorState({ currentLevel: parseInt(layer) });
  };

  const handleDeleteLayer = () => {
    const newLevels = { ...props.map.levels };
    delete newLevels[props.editorState.currentLevel];
    handleSelectLayer('0');
    props.onMapUpdate({
      ...props.map,
      levels: newLevels,
    });
  };

  const sortedEntries = Object.entries(props.map.levels).sort(
    (a, b) => parseInt(b[0]) - parseInt(a[0])
  );

  return (
    <div
      style={{
        padding: '15px 2px',
      }}
    >
      <div
        style={{
          color: '#858585',
          fontSize: '11px',
          textTransform: 'uppercase',
          fontWeight: 'bold',
          marginBottom: '5px',
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
        }}
      >
        <div>Current Layer</div>
        <div
          style={{
            margin: '4px',
            display: 'flex',
            gap: '8px',
            justifyContent: 'flex-end',
          }}
        >
          <LayerButton
            color="#1e7e1a"
            onClick={() => {
              handleAddNewLayer('above');
            }}
          >
            +↑
          </LayerButton>
          <LayerButton
            color="#1e7e1a"
            onClick={() => {
              handleAddNewLayer('below');
            }}
          >
            +↓
          </LayerButton>

          <LayerButton
            disabled={props.editorState.currentLevel === 0}
            color="#7e1a1a"
            onClick={() => {
              setShowDeleteConfirm(true);
            }}
          >
            <span style={{ filter: 'grayscale(100%) sepia(100%)' }}>❌</span>
          </LayerButton>
        </div>
      </div>
      <select
        value={props.editorState.currentLevel}
        onChange={(e) => handleSelectLayer(e.target.value)}
        size={sortedEntries.length}
        style={{
          width: '100%',
          padding: '10px',
          background: '#1e1e1e',
          border: '1px solid #3e3e42',
          borderRadius: '4px',
          color: '#d4d4d4',
          fontSize: '14px',
          fontFamily: 'inherit',
        }}
      >
        {sortedEntries.map(([level, layer]) => {
          return (
            <option key={level} value={level}>
              Layer {level}
            </option>
          );
        })}
      </select>
      <div
        style={{
          display: 'flex',
          justifyContent: 'flex-end',
          alignItems: 'center',
        }}
      >
        <div
          style={{
            color: '#858585',
            fontSize: '11px',
            textTransform: 'uppercase',
            fontWeight: 'bold',
          }}
        ></div>
      </div>
      <DeleteModal
        isOpen={showDeleteConfirm}
        message="Are you sure you want to delete this layer (no undo)?"
        onConfirm={() => {
          handleDeleteLayer();
          setShowDeleteConfirm(false);
        }}
        onCancel={() => setShowDeleteConfirm(false)}
      />
    </div>
  );
}
