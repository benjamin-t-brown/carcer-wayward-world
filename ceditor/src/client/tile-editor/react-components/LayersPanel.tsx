import { EditorState, updateEditorState } from '../editorState';
import { CarcerMapTemplate } from '../../types/assets';
import { useState } from 'react';
import { DeleteModal } from '../../elements/DeleteModal';
import { MapSearchAccordion } from './MapSearchAccordion';
import {
  addMapLayer,
  deleteMapLayer,
  layerKey,
  sortedLayerKeys,
} from '../../utils/mapIndex';

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
    const layers = sortedLayerKeys(props.map);
    const highestLayer = layers.length ? Math.max(...layers) : 0;
    const lowestLayer = layers.length ? Math.min(...layers) : 0;
    const newLayerKey = where === 'above' ? highestLayer + 1 : lowestLayer - 1;
    props.onMapUpdate(addMapLayer(props.map, newLayerKey));
  };

  const handleSelectLayer = (layer: string) => {
    updateEditorState({ currentLevel: parseInt(layer) });
  };

  const handleDeleteLayer = () => {
    const updated = deleteMapLayer(props.map, props.editorState.currentLevel);
    handleSelectLayer(String(updated.layers[0] ?? 0));
    props.onMapUpdate(updated);
  };

  const sortedEntries = sortedLayerKeys(props.map).map((level) => [
    layerKey(level),
    level,
  ]);

  const layerRowHeight = 34;
  const layerListMaxHeight = Math.min(
    sortedEntries.length * layerRowHeight,
    120
  );

  return (
    <div className="tile-editor-sidebar-section">
      <label
        style={{
          display: 'flex',
          alignItems: 'center',
          gap: '8px',
          cursor: 'pointer',
          fontSize: '12px',
          color: '#d4d4d4',
          marginBottom: '8px',
        }}
      >
        <input
          type="checkbox"
          checked={props.editorState.showGrid}
          onChange={(e) => updateEditorState({ showGrid: e.target.checked })}
          style={{
            cursor: 'pointer',
            width: '16px',
            height: '16px',
          }}
        />
        Show grid
      </label>
      <MapSearchAccordion map={props.map} />
      <div
        style={{
          marginTop: '8px',
          marginBottom: '5px',
          display: 'flex',
          gap: '4px',
          justifyContent: 'center',
          flexWrap: 'wrap',
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
      <div
        className="tile-editor-sidebar-scroll"
        style={{ maxHeight: layerListMaxHeight }}
      >
        <div className="tile-editor-sidebar-panel tile-editor-layer-list">
          {sortedEntries.map(([level]) => {
            const levelNum = parseInt(level, 10);
            const isSelected = props.editorState.currentLevel === levelNum;
            return (
              <button
                key={level}
                type="button"
                className={`tile-editor-layer-row${isSelected ? ' selected' : ''}`}
                onClick={() => handleSelectLayer(level)}
              >
                Layer {level}
              </button>
            );
          })}
        </div>
      </div>
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
