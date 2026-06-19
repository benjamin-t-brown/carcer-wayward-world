import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { OptionSelect } from '../elements/OptionSelect';
import { TextArea } from '../elements/TextArea';
import { Button } from '../elements/Button';
import { SpritePicker } from '../elements/SpritePicker';
import {
  ItemTemplate,
  ItemType,
  ItemUsability,
  ItemStatusEffectRef,
  ITEM_TYPES,
  ITEM_USABILITIES,
  isItemUsable,
  isWeaponItemType,
} from '../types/assets';
import { useAssets } from '../contexts/AssetsContext';
import { ItemWeaponFields } from './ItemWeaponFields';
import { ItemUseAbilityFields } from './ItemUseAbilityFields';
import { ItemUseSpecialEventFields } from './ItemUseSpecialEventFields';
import { EditorEmptyState } from './EditorEmptyState';

// Re-export for backward compatibility
export type { ItemTemplate };

function getStatusEffectName(ref: ItemStatusEffectRef): string {
  return typeof ref === 'string' ? ref : ref.name;
}

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
    indestructable: false,
    itemUsability: 'NOT_USABLE',
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
    if (!item) {
      return;
    }
    setFormData({ ...item, [field]: value });
  };

  const { statusEffects: statusEffectTemplates } = useAssets();

  const addStatusEffect = () => {
    const first = statusEffectTemplates[0]?.name || '';
    setFormData({
      ...formData,
      statusEffects: [...(formData.statusEffects || []), first],
    });
  };

  const removeStatusEffect = (index: number) => {
    setFormData({
      ...formData,
      statusEffects:
        formData.statusEffects?.filter((_, i) => i !== index) || [],
    });
  };

  const updateStatusEffect = (index: number, value: string) => {
    setFormData({
      ...formData,
      statusEffects:
        formData.statusEffects?.map((effect, i) =>
          i === index ? value : effect,
        ) || [],
    });
  };

  if (!item) {
    return <EditorEmptyState message="Select an item to edit" />;
  }

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
            onChange={(value) => {
              const itemType = value as ItemType;
              const next = { ...formData, itemType };
              if (
                isWeaponItemType(itemType) &&
                !isWeaponItemType(formData.itemType) &&
                !next.weapon
              ) {
                next.weapon = { abilityName: '' };
              }
              if (!isWeaponItemType(itemType)) {
                delete next.weapon;
              }
              setFormData(next);
            }}
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

          <div
            style={{
              display: 'flex',
              alignItems: 'center',
              gap: '10px',
              margin: '8px 0',
            }}
          >
            <input
              id="item-indestructable"
              name="indestructable"
              type="checkbox"
              checked={formData.indestructable === true}
              onChange={(e) => updateField('indestructable', e.target.checked)}
              style={{ cursor: 'pointer', transform: 'scale(1.5)translateX(2px)' }}
            />
            <label htmlFor="item-indestructable" style={{ marginBottom: 0 }}>
              Indestructable
            </label>
          </div>

          <div className="form-subsection">
            <h4>Usability</h4>
            <div className="form-fields-inline" style={{ marginBottom: '10px' }}>
              <OptionSelect
                id="item-usability"
                name="itemUsability"
                label="Where Usable"
                value={formData.itemUsability || 'NOT_USABLE'}
                onChange={(value) => {
                  const itemUsability = value as ItemUsability;
                  const next = { ...formData, itemUsability };
                  if (!isItemUsable(itemUsability)) {
                    delete next.useAbility;
                    delete next.useSpecialEvent;
                  } else if (!next.useAbility && !next.useSpecialEvent) {
                    next.useAbility = { abilityName: '' };
                  }
                  setFormData(next);
                }}
                options={ITEM_USABILITIES.map((type) => ({
                  value: type,
                  label: type,
                }))}
              />
            </div>
            {isItemUsable(formData.itemUsability) ? (
              <>
                <ItemUseAbilityFields
                  useAbility={formData.useAbility}
                  onChange={(useAbility) => {
                    const next = { ...formData, useAbility };
                    if (useAbility?.abilityName) {
                      delete next.useSpecialEvent;
                    }
                    setFormData(next);
                  }}
                  idPrefix={`item-${formData.name || 'new'}-use`}
                />
                <ItemUseSpecialEventFields
                  useSpecialEvent={formData.useSpecialEvent}
                  onChange={(useSpecialEvent) => {
                    const next = { ...formData };
                    if (useSpecialEvent) {
                      next.useSpecialEvent = useSpecialEvent;
                      delete next.useAbility;
                    } else {
                      delete next.useSpecialEvent;
                    }
                    setFormData(next);
                  }}
                  idPrefix={`item-${formData.name || 'new'}-use`}
                />
              </>
            ) : null}
          </div>

          {isWeaponItemType(formData.itemType) ? (
            <ItemWeaponFields
              weapon={formData.weapon}
              onChange={(weapon) => updateField('weapon', weapon)}
              idPrefix={`item-${formData.name || 'new'}-weapon`}
            />
          ) : null}

          <div className="form-subsection">
            <h4>Status Effects (while equipped)</h4>
            <div id="status-effects-list">
              {formData.statusEffects?.map((effectRef, index) => (
                <div key={index} className="status-effect-item">
                  <div className="status-effect-header">
                    <OptionSelect
                      value={getStatusEffectName(effectRef)}
                      onChange={(value) => updateStatusEffect(index, value)}
                      options={statusEffectTemplates.map((template) => ({
                        value: template.name,
                        label: template.name,
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
