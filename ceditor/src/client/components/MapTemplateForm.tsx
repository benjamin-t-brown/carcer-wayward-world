import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
} from '../types/assets';

interface MapTemplateFormProps {
  map?: CarcerMapTemplate;
  updateMap: (map: CarcerMapTemplate) => void;
}

export function createDefaultMap(): CarcerMapTemplate {
  const width = 20;
  const height = 20;
  const tiles: any[] = [];
  
  // Generate tiles for all positions in the map
  for (let y = 0; y < height; y++) {
    for (let x = 0; x < width; x++) {
      tiles.push(createDefaultCarcerMapTile());
    }
  }
  
  return {
    name: '',
    label: '',
    width: width,
    height: height,
    spriteWidth: 28,
    spriteHeight: 32,
    tiles: tiles,
  };
}

export function createDefaultCarcerMapTile(): CarcerMapTileTemplate {
  return {
    characters: [],
    items: [],
    markers: [],
    tilesetName: '',
    tileId: 0,
  };
}

export function MapTemplateForm(props: MapTemplateFormProps) {
  const map = props.map;

  const formData = map as CarcerMapTemplate;
  const setFormData = (data: CarcerMapTemplate) => {
    props.updateMap(data);
  };

  const updateField = <K extends keyof CarcerMapTemplate>(
    field: K,
    value: CarcerMapTemplate[K]
  ) => {
    setFormData({ ...formData, [field]: value });
  };

  if (!map) {
    return (
      <div className="item-form">
        <div style={{ color: '#858585', fontSize: '14px' }}>
          Select a map to edit or create a new one.
        </div>
      </div>
    );
  }

  return (
    <div className="item-form">
      <TextInput
        id="map-name"
        name="name"
        label="Name"
        value={formData.name}
        onChange={(value) => updateField('name', value)}
        placeholder="e.g., town_map"
        required
      />
      <TextInput
        id="map-label"
        name="label"
        label="Label"
        value={formData.label}
        onChange={(value) => updateField('label', value)}
        placeholder="e.g., Town Map"
        required
      />
      <NumberInput
        id="map-width"
        name="width"
        label="Width"
        value={formData.width}
        onChange={(value) => updateField('width', value)}
        min={1}
        required
      />
      <NumberInput
        id="map-height"
        name="height"
        label="Height"
        value={formData.height}
        onChange={(value) => updateField('height', value)}
        min={1}
        required
      />
    </div>
  );
}

