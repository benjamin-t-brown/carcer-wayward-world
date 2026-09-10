import { Button } from '../../elements/Button';
import { NumberInput } from '../../elements/NumberInput';
import { OptionSelect } from '../../elements/OptionSelect';
import { TextArea } from '../../elements/TextArea';
import { TextInput } from '../../elements/TextInput';
import {
  ABILITY_TYPES,
  AbilityAttack,
  AbilityDamage,
  AbilityRestore,
  AbilityStatus,
  AbilityTemplate,
} from '../../types/ability';
import {
  ABILITY_COST_TYPES,
  AbilityAttackEditor,
  AbilityDamageEditor,
  AbilityDepictionFields,
  AbilityRestoreEditor,
  AbilityStatusEditor,
  TargetSelectFields,
  enumOptions,
} from './AbilityFormFields';

export type UpdateAbilityField = <K extends keyof AbilityTemplate>(
  field: K,
  value: AbilityTemplate[K],
) => void;

interface AbilityBasicsSectionProps {
  ability: AbilityTemplate;
  updateField: UpdateAbilityField;
}

export function AbilityBasicsSection({
  ability,
  updateField,
}: AbilityBasicsSectionProps) {
  return (
    <>
      <div className="form-subsection">
        <h4>Identity</h4>
        <div className="form-fields-inline">
          <TextInput
            id="ability-name"
            name="name"
            label="Name (ID)"
            value={ability.name}
            onChange={(value) => updateField('name', value)}
            required
          />
          <TextInput
            id="ability-label"
            name="label"
            label="Label"
            value={ability.label}
            onChange={(value) => updateField('label', value)}
            required
          />
          <TextInput
            id="ability-icon"
            name="icon"
            label="Icon"
            value={ability.icon}
            onChange={(value) => updateField('icon', value)}
          />
          <OptionSelect
            id="ability-type"
            name="type"
            label="Type"
            value={ability.type}
            onChange={(value) =>
              updateField('type', value as AbilityTemplate['type'])
            }
            options={enumOptions(ABILITY_TYPES)}
          />
        </div>
      </div>

      <div className="form-subsection">
        <h4>Cost</h4>
        <div className="form-fields-inline">
          <NumberInput
            id="ability-ap-cost"
            name="apCost"
            label="AP Cost"
            value={ability.apCost}
            onChange={(value) => updateField('apCost', value ?? 0)}
            min={0}
          />
          <OptionSelect
            id="ability-cost-type"
            name="costType"
            label="Aux Cost Type"
            value={ability.costType}
            onChange={(value) =>
              updateField('costType', value as AbilityTemplate['costType'])
            }
            options={enumOptions(ABILITY_COST_TYPES)}
          />
          <NumberInput
            id="ability-cost-value"
            name="costValue"
            label="Aux Cost Value"
            value={ability.costValue}
            onChange={(value) => updateField('costValue', value ?? 0)}
            min={0}
          />
        </div>
      </div>

      <div className="form-group form-block">
        <TextArea
          id="ability-description"
          name="description"
          label="Description"
          value={ability.description}
          onChange={(value) => updateField('description', value)}
          rows={3}
        />
      </div>
    </>
  );
}

export function AbilityTargetingSection({
  ability,
  updateField,
}: AbilityBasicsSectionProps) {
  return (
    <>
      <div className="form-subsection">
        <h4>Target Selection</h4>
        <TargetSelectFields
          value={ability.targetSelect}
          onChange={(targetSelect) => updateField('targetSelect', targetSelect)}
          idPrefix="ability-target"
        />
      </div>

      <div className="form-subsection">
        <h4>Depiction</h4>
        <AbilityDepictionFields
          value={ability.depiction}
          onChange={(depiction) => updateField('depiction', depiction)}
          idPrefix="ability-depiction"
          compact
        />
      </div>
    </>
  );
}

interface AbilityEffectsSectionProps {
  ability: AbilityTemplate;
  statusEffectOptions: Array<{ value: string; label: string }>;
  onUpdateAttack: (index: number, attack: AbilityAttack) => void;
  onAddAttack: () => void;
  onRemoveAttack: (index: number) => void;
  onUpdateStatus: (index: number, status: AbilityStatus) => void;
  onAddStatus: () => void;
  onRemoveStatus: (index: number) => void;
  onUpdateRestore: (index: number, restore: AbilityRestore) => void;
  onAddRestore: () => void;
  onRemoveRestore: (index: number) => void;
  onUpdateDamage: (index: number, damage: AbilityDamage) => void;
  onAddDamage: () => void;
  onRemoveDamage: (index: number) => void;
}

export function AbilityEffectsSections({
  ability,
  statusEffectOptions,
  onUpdateAttack,
  onAddAttack,
  onRemoveAttack,
  onUpdateStatus,
  onAddStatus,
  onRemoveStatus,
  onUpdateRestore,
  onAddRestore,
  onRemoveRestore,
  onUpdateDamage,
  onAddDamage,
  onRemoveDamage,
}: AbilityEffectsSectionProps) {
  return (
    <>
      <div className="form-subsection">
        <h4>Attacks</h4>
        {ability.attacks?.map((attack, index) => (
          <AbilityAttackEditor
            key={index}
            attack={attack}
            index={index}
            onChange={(updated) => onUpdateAttack(index, updated)}
            onRemove={() => onRemoveAttack(index)}
          />
        ))}
        <Button type="button" variant="secondary" onClick={onAddAttack}>
          + Add Attack
        </Button>
      </div>

      <div className="form-subsection">
        <h4>Apply Status Effects</h4>
        {ability.statuses?.map((status, index) => (
          <AbilityStatusEditor
            key={index}
            status={status}
            index={index}
            statusEffectOptions={statusEffectOptions}
            onChange={(updated) => onUpdateStatus(index, updated)}
            onRemove={() => onRemoveStatus(index)}
          />
        ))}
        <Button type="button" variant="secondary" onClick={onAddStatus}>
          + Add Status Apply
        </Button>
      </div>

      <div className="form-subsection">
        <h4>Restores</h4>
        {ability.restores?.map((restore, index) => (
          <AbilityRestoreEditor
            key={index}
            restore={restore}
            index={index}
            onChange={(updated) => onUpdateRestore(index, updated)}
            onRemove={() => onRemoveRestore(index)}
          />
        ))}
        <Button type="button" variant="secondary" onClick={onAddRestore}>
          + Add Restore
        </Button>
      </div>

      <div className="form-subsection">
        <h4>Damages</h4>
        {ability.damages?.map((damage, index) => (
          <AbilityDamageEditor
            key={index}
            damage={damage}
            index={index}
            onChange={(updated) => onUpdateDamage(index, updated)}
            onRemove={() => onRemoveDamage(index)}
          />
        ))}
        <Button type="button" variant="secondary" onClick={onAddDamage}>
          + Add Damage
        </Button>
      </div>
    </>
  );
}
