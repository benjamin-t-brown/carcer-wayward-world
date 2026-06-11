import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { OptionSelect } from '../elements/OptionSelect';
import {
  CarcerMapTemplate,
  CarcerMapTileTemplate,
  MAP_TYPES,
  MapType,
} from '../types/assets';
import { createDefaultMapTemplate } from '../utils/mapIndex';

interface MapTemplateFormProps {
  map?: CarcerMapTemplate;
  updateMap: (map: CarcerMapTemplate) => void;
}

export function createDefaultMap(): CarcerMapTemplate {
  return createDefaultMapTemplate();
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
      <OptionSelect
        id="map-type"
        name="type"
        label="Type"
        value={formData.type}
        onChange={(value) => updateField('type', value as MapType)}
        options={MAP_TYPES.map((type) => ({ value: type, label: type }))}
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

