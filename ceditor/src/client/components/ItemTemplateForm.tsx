import { useState, useEffect } from 'react';
import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { OptionSelect } from '../elements/OptionSelect';
import { TextArea } from '../elements/TextArea';
import { Button } from '../elements/Button';
import { SpritePicker } from '../elements/SpritePicker';

export interface ItemTemplate {
  itemType: string;
  name: string;
  label: string;
  icon: string;
  description: string;
  weight: number;
  value: number;
  // Optional fields
  itemUsability?: string;
  itemUsabilityArgs?: {
    itemUsabilityType: string;
    intArgs?: number[];
    stringArgs?: string[];
  };
  statusEffects?: Array<{
    statusEffectType: string;
    name?: string;
    label?: string;
    description?: string;
  }>;
  weapon?: {
    dmgMin?: number;
    dmgMax?: number;
  };
}

const ITEM_TYPES = [
  'WEAPON_MELEE',
  'WEAPON_RANGED',
  'WEAPON_AMMO',
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

const ITEM_USABILITY_ARG_TYPES = ['ITEM_USE_NOT_USABLE', 'ITEM_USE_CAST_SPELL'];

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
    itemUsability: 'NOT_USABLE',
    itemUsabilityArgs: {
      itemUsabilityType: 'ITEM_USE_NOT_USABLE',
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
  // const [formData, _setFormData] = useState<ItemTemplate>(() => {
  //   if (item) {
  //     return { ...item };
  //   }
  //   return createDefaultItem();
  // });

  // const setFormData = (data: ItemTemplate) => {
  //   _setFormData(data);
  //   props.updateItem(data);
  // };

  // useEffect(() => {
  //   if (item && item?.name !== formData.name) {
  //     _setFormData({ ...item });
  //   } else {
  //     _setFormData(createDefaultItem());
  //   }
  // }, [item]);

  // const buildItemFromFormData = (data: ItemTemplate): ItemTemplate => {
  //   const item: ItemTemplate = {
  //     itemType: data.itemType,
  //     name: data.name,
  //     label: data.label,
  //     icon: data.icon,
  //     description: data.description,
  //     weight: data.weight,
  //     value: data.value,
  //   };

  //   // Optional: Usability
  //   if (data.itemUsability && data.itemUsability !== 'NOT_USABLE') {
  //     item.itemUsability = data.itemUsability;

  //     const usabilityType =
  //       data.itemUsabilityArgs?.itemUsabilityType || 'ITEM_USE_NOT_USABLE';
  //     const intArgs = data.itemUsabilityArgs?.intArgs || [];
  //     const stringArgs = data.itemUsabilityArgs?.stringArgs || [];

  //     if (
  //       usabilityType !== 'ITEM_USE_NOT_USABLE' ||
  //       intArgs.length > 0 ||
  //       stringArgs.length > 0
  //     ) {
  //       item.itemUsabilityArgs = {
  //         itemUsabilityType: usabilityType,
  //       };
  //       if (intArgs.length > 0) {
  //         item.itemUsabilityArgs.intArgs = intArgs;
  //       }
  //       if (stringArgs.length > 0) {
  //         item.itemUsabilityArgs.stringArgs = stringArgs;
  //       }
  //     }
  //   }

  //   // Optional: Weapon
  //   if (data.weapon?.dmgMin || data.weapon?.dmgMax) {
  //     item.weapon = {};
  //     if (data.weapon.dmgMin !== undefined) {
  //       item.weapon.dmgMin = data.weapon.dmgMin;
  //     }
  //     if (data.weapon.dmgMax !== undefined) {
  //       item.weapon.dmgMax = data.weapon.dmgMax;
  //     }
  //   }

  //   // Optional: Status Effects
  //   if (data.statusEffects && data.statusEffects.length > 0) {
  //     item.statusEffects = data.statusEffects;
  //   }

  //   return item;
  // };

  const updateField = <K extends keyof ItemTemplate>(
    field: K,
    value: ItemTemplate[K]
  ) => {
    setFormData({ ...formData, [field]: value });
  };

  const updateUsabilityArgs = (
    field: 'itemUsabilityType' | 'intArgs' | 'stringArgs',
    value: string | number[] | string[] | undefined
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
    value: number | undefined
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
    value: string
  ) => {
    setFormData({
      ...formData,
      statusEffects:
        formData.statusEffects?.map((effect, i) =>
          i === index ? { ...effect, [field]: value } : effect
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
    <div className={`item-form`}>
      <h2>{'Edit Item'}</h2>
      <form>
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

        <div className="form-group">
          <label htmlFor="item-icon">Icon *</label>
          <div style={{ marginTop: '8px' }}>
            <SpritePicker
              value={formData.icon}
              onChange={(value) => updateField('icon', value)}
              scale={2}
            />
          </div>
          {formData.icon && (
            <div
              style={{ marginTop: '8px', fontSize: '12px', color: '#858585' }}
            >
              Selected: {formData.icon}
            </div>
          )}
        </div>

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

        <div className="form-row">
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

        <div className="form-section">
          <h3>Optional Properties</h3>

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
              'ITEM_USE_NOT_USABLE'
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
            label="Usability Int Args (comma-separated)"
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
            label="Usability String Args (comma-separated)"
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

          <div className="form-row">
            <NumberInput
              id="weapon-dmg-min"
              name="weaponDmgMin"
              label="Weapon Damage Min"
              value={formData.weapon?.dmgMin || 0}
              onChange={(value) => updateWeapon('dmgMin', value || undefined)}
              min={0}
            />

            <NumberInput
              id="weapon-dmg-max"
              name="weaponDmgMax"
              label="Weapon Damage Max"
              value={formData.weapon?.dmgMax || 0}
              onChange={(value) => updateWeapon('dmgMax', value || undefined)}
              min={0}
            />
          </div>

          <div className="form-group">
            <label>Status Effects</label>
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
            <div style={{ marginTop: '10px' }}>
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
