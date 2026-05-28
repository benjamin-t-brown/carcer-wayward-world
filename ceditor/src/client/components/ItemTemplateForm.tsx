import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { OptionSelect } from '../elements/OptionSelect';
import { TextArea } from '../elements/TextArea';
import { Button } from '../elements/Button';
import { SpritePicker } from '../elements/SpritePicker';
import { ItemTemplate } from '../types/assets';

// Re-export for backward compatibility
export type { ItemTemplate };

const ITEM_TYPES = [
  'WEAPON_MELEE',
  'WEAPON_MELEE_2H',
  'WEAPON_RANGED',
  'WEAPON_AMMO',
  'SHIELD',
  'GARB',
  'PANTS',
  'GLOVES',
  'HAT',
  'SHOES',
  'NECKLACE',
  'POTION',
  'UTILITY',
];

const ITEM_USABILITY_TYPES = [
  'NOT_USABLE',
  'USABLE_EVERYWHERE',
  'USABLE_TOWN_ONLY',
  'USABLE_COMBAT_ONLY',
  'USABLE_OUTSIDE_ONLY',
  'USABLE_TOWN_AND_COMBAT',
];

const ITEM_USABILITY_ARG_TYPES = ['ITEM_USE_DEFAULT', 'ITEM_USE_CAST_SPELL'];

const STATUS_EFFECT_TYPES = ['MODIFY_RESISTANCE_FIRE_PLUS_1'];

interface ItemTemplateFormProps {
  item?: ItemTemplate;
  updateItem: (item: ItemTemplate) => void;
}

export function createDefaultItem(): ItemTemplate {
  return {
    itemType: 'UTILITY',
    name: '',
    label: '',
    icon: 'ui_item_icons_0',
    description: 'This item has no description.',
    weight: 1,
    value: 1,
    stackable: false,
    itemUsability: 'NOT_USABLE',
    itemUsabilityArgs: {
      itemUsabilityType: 'ITEM_USE_DEFAULT',
    },
    statusEffects: [],
  };
}

export function ItemTemplateForm(props: ItemTemplateFormProps) {
  const item = props.item;

  const formData = item as ItemTemplate;
  const setFormData = (data: ItemTemplate) => {
    props.updateItem(data);
  };

  const updateField = <K extends keyof ItemTemplate>(
    field: K,
    value: ItemTemplate[K],
  ) => {
    setFormData({ ...formData, [field]: value });
  };

  const updateUsabilityArgs = (
    field: 'itemUsabilityType' | 'intArgs' | 'stringArgs',
    value: string | number[] | string[] | undefined,
  ) => {
    setFormData({
      ...formData,
      itemUsabilityArgs: {
        ...formData.itemUsabilityArgs,
        [field]: value,
      } as ItemTemplate['itemUsabilityArgs'],
    });
  };

  const updateWeapon = (
    field: 'dmgMin' | 'dmgMax',
    value: number | undefined,
  ) => {
    setFormData({
      ...formData,
      weapon: { ...formData.weapon, [field]: value },
    });
  };

  const addStatusEffect = () => {
    setFormData({
      ...formData,
      statusEffects: [
        ...(formData.statusEffects || []),
        { statusEffectType: 'MODIFY_RESISTANCE_FIRE_PLUS_1' },
      ],
    });
  };

  const removeStatusEffect = (index: number) => {
    setFormData({
      ...formData,
      statusEffects:
        formData.statusEffects?.filter((_, i) => i !== index) || [],
    });
  };

  const updateStatusEffect = (
    index: number,
    field: 'statusEffectType' | 'name' | 'label' | 'description',
    value: string,
  ) => {
    setFormData({
      ...formData,
      statusEffects:
        formData.statusEffects?.map((effect, i) =>
          i === index ? { ...effect, [field]: value } : effect,
        ) || [],
    });
  };

  if (!item) {
    return <div>Select an item to edit</div>;
  }

  const intArgsString = formData.itemUsabilityArgs?.intArgs?.join(',') || '';
  const stringArgsString =
    formData.itemUsabilityArgs?.stringArgs?.join(',') || '';

  return (
    <div className="item-form item-template-form">
      <h2>Edit Item</h2>
      <form>
        <div className="form-fields-inline">
          <OptionSelect
            id="item-type"
            name="itemType"
            label="Item Type"
            value={formData.itemType}
            onChange={(value) => updateField('itemType', value)}
            options={ITEM_TYPES.map((type) => ({ value: type, label: type }))}
            required
          />

          <TextInput
            id="item-name"
            name="name"
            label="Name"
            value={formData.name}
            onChange={(value) => updateField('name', value)}
            placeholder="e.g., PotionHealing"
            required
          />

          <TextInput
            id="item-label"
            name="label"
            label="Label"
            value={formData.label}
            onChange={(value) => updateField('label', value)}
            placeholder="e.g., Potion: Healing"
            required
          />

          <NumberInput
            id="item-weight"
            name="weight"
            label="Weight"
            value={formData.weight}
            onChange={(value) => updateField('weight', value)}
            min={0}
            required
          />

          <NumberInput
            id="item-value"
            name="value"
            label="Value"
            value={formData.value}
            onChange={(value) => updateField('value', value)}
            min={0}
            required
          />
        </div>

        <div className="form-group form-block">
          <label htmlFor="item-icon">Icon *</label>
          <div style={{ marginTop: '4px' }}>
            <SpritePicker
              value={formData.icon}
              onChange={(value) => updateField('icon', value)}
              scale={2}
            />
          </div>
          {formData.icon && (
            <div
              style={{ marginTop: '4px', fontSize: '11px', color: '#858585' }}
            >
              Selected: {formData.icon}
            </div>
          )}
        </div>

        <div className="form-group form-block">
          <TextArea
            id="item-description"
            name="description"
            label="Description"
            value={formData.description}
            onChange={(value) => updateField('description', value)}
            placeholder="Item description"
            rows={3}
            required
          />
        </div>

        <div className="form-section">
          <h3>Optional Properties</h3>

          <div
            style={{
              display: 'flex',
              alignItems: 'center',
              gap: '10px',
              margin: '8px 0',
            }}
          >
            <input
              id="item-stackable"
              name="stackable"
              type="checkbox"
              checked={formData.stackable === true}
              onChange={(e) => updateField('stackable', e.target.checked)}
              style={{ cursor: 'pointer', transform: 'scale(1.5)translateX(2px)' }}
            />
            <label htmlFor="item-stackable" style={{ marginBottom: 0 }}>
              Stackable
            </label>
          </div>

          <div className="form-subsection">
            <h4>Usability</h4>
            <div className="form-fields-inline">
              <OptionSelect
                id="item-usability"
                name="itemUsability"
                label="Usability"
                value={formData.itemUsability || 'NOT_USABLE'}
                onChange={(value) => updateField('itemUsability', value)}
                options={ITEM_USABILITY_TYPES.map((type) => ({
                  value: type,
                  label: type,
                }))}
              />

              <OptionSelect
                id="item-usability-type"
                name="itemUsabilityType"
                label="Usability Type"
                value={
                  formData.itemUsabilityArgs?.itemUsabilityType ||
                  'ITEM_USE_DEFAULT'
                }
                onChange={(value) =>
                  updateUsabilityArgs('itemUsabilityType', value)
                }
                options={ITEM_USABILITY_ARG_TYPES.map((type) => ({
                  value: type,
                  label: type,
                }))}
              />

              <TextInput
                id="item-usability-int-args"
                name="itemUsabilityIntArgs"
                label="Int Args"
                value={intArgsString}
                onChange={(value) => {
                  const intArgs = value
                    .split(',')
                    .map((s) => parseInt(s.trim()))
                    .filter((n) => !isNaN(n));
                  updateUsabilityArgs('intArgs', intArgs);
                }}
                placeholder="e.g., 10, 20, 30"
              />

              <TextInput
                id="item-usability-string-args"
                name="itemUsabilityStringArgs"
                label="String Args"
                value={stringArgsString}
                onChange={(value) => {
                  const stringArgs = value
                    .split(',')
                    .map((s) => s.trim())
                    .filter((s) => s.length > 0);
                  updateUsabilityArgs('stringArgs', stringArgs);
                }}
                placeholder="e.g., arg1, arg2"
              />
            </div>
          </div>

          <div className="form-subsection">
            <h4>Weapon</h4>
            <div className="form-fields-inline">
              <NumberInput
                id="weapon-dmg-min"
                name="weaponDmgMin"
                label="Damage Min"
                value={formData.weapon?.dmgMin || 0}
                onChange={(value) => updateWeapon('dmgMin', value || undefined)}
                min={0}
              />

              <NumberInput
                id="weapon-dmg-max"
                name="weaponDmgMax"
                label="Damage Max"
                value={formData.weapon?.dmgMax || 0}
                onChange={(value) => updateWeapon('dmgMax', value || undefined)}
                min={0}
              />
            </div>
          </div>

          <div className="form-subsection">
            <h4>Status Effects</h4>
            <div id="status-effects-list">
              {formData.statusEffects?.map((effect, index) => (
                <div key={index} className="status-effect-item">
                  <div className="status-effect-header">
                    <OptionSelect
                      value={effect.statusEffectType}
                      onChange={(value) =>
                        updateStatusEffect(index, 'statusEffectType', value)
                      }
                      options={STATUS_EFFECT_TYPES.map((type) => ({
                        value: type,
                        label: type,
                      }))}
                    />
                    <Button
                      variant="small"
                      className="btn-danger"
                      onClick={() => removeStatusEffect(index)}
                    >
                      Remove
                    </Button>
                  </div>
                  <div className="form-fields-inline">
                    <TextInput
                      label="Name"
                      value={effect.name || ''}
                      onChange={(value) =>
                        updateStatusEffect(index, 'name', value)
                      }
                      placeholder="Status effect name"
                    />
                    <TextInput
                      label="Label"
                      value={effect.label || ''}
                      onChange={(value) =>
                        updateStatusEffect(index, 'label', value)
                      }
                      placeholder="Display label"
                    />
                  </div>
                  <TextArea
                    label="Description"
                    value={effect.description || ''}
                    onChange={(value) =>
                      updateStatusEffect(index, 'description', value)
                    }
                    placeholder="Description"
                    rows={2}
                  />
                </div>
              ))}
            </div>
            <div style={{ marginTop: '6px' }}>
              <Button
                type="button"
                variant="secondary"
                onClick={addStatusEffect}
              >
                + Add Status Effect
              </Button>
            </div>
          </div>
        </div>

        {/* {isNew ? (
          <div className="form-actions">
            <Button type="submit" variant="primary">
              Save Item
            </Button>
          </div>
        ) : null} */}
      </form>
    </div>
  );
}
