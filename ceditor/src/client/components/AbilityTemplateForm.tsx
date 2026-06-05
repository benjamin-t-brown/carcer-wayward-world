import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { OptionSelect } from '../elements/OptionSelect';
import { TextArea } from '../elements/TextArea';
import { Button } from '../elements/Button';
import { useEffect, useState } from 'react';
import {
  AbilityTemplate,
  createDefaultAbilityTemplate,
  sanitizeAbilityDepiction,
  ABILITY_TYPES,
  AbilityAttack,
  AbilityStatus,
  AbilityRestore,
} from '../types/combat';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import {
  TargetSelectFields,
  AbilityDepictionFields,
  AbilityAttackEditor,
  AbilityStatusEditor,
  AbilityRestoreEditor,
  ABILITY_COST_TYPES,
  enumOptions,
} from './combat/CombatFormFields';
import { useAssets } from '../contexts/AssetsContext';
import {
  applyWeaponAttackDeleteToItems,
  planWeaponAttackDeleteImpacts,
  AbilityDeleteImpact,
} from '../types/assets';
import { AbilityAttackDeleteModal } from './AbilityAttackDeleteModal';
import { EditorEmptyState } from './EditorEmptyState';

export type { AbilityTemplate };
export { createDefaultAbilityTemplate };

interface AbilityTemplateFormProps {
  ability?: AbilityTemplate;
  updateAbility: (ability: AbilityTemplate) => void;
}

export function AbilityTemplateForm(props: AbilityTemplateFormProps) {
  const { statusEffects, items, setItems } = useAssets();
  const [attackDeleteConfirm, setAttackDeleteConfirm] = useState<{
    attackIndex: number;
    impacts: AbilityDeleteImpact[];
  } | null>(null);
  const { animationMap, soundMap } = useSDL2WAssets();
  const ability = props.ability;

  useEffect(() => {
    if (!ability) {
      return;
    }
    const depiction = sanitizeAbilityDepiction(
      ability.depiction,
      animationMap,
      soundMap
    );
    if (
      depiction.dmgAnim !== ability.depiction.dmgAnim ||
      depiction.projectileAnim !== ability.depiction.projectileAnim ||
      depiction.startSound !== ability.depiction.startSound ||
      depiction.dmgSound !== ability.depiction.dmgSound
    ) {
      props.updateAbility({ ...ability, depiction });
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps -- only when selecting a different ability
  }, [ability?.name, animationMap, soundMap]);

  if (!ability) {
    return <EditorEmptyState message="Select an ability to edit" />;
  }

  const setFormData = (data: AbilityTemplate) => props.updateAbility(data);
  const updateField = <K extends keyof AbilityTemplate>(field: K, value: AbilityTemplate[K]) => {
    setFormData({ ...ability, [field]: value });
  };

  const statusEffectOptions = statusEffects.map((s) => ({
    value: s.name,
    label: s.name,
  }));

  const updateAttack = (index: number, attack: AbilityAttack) => {
    const attacks = [...(ability.attacks || [])];
    attacks[index] = attack;
    updateField('attacks', attacks);
  };

  const addAttack = () => {
    const newAttack: AbilityAttack = {
      attackClass: 'ATTACK_CLASS_MELEE',
      dmg: {
        dmgDice: ['D6'],
        dmgBonus: 0,
        dmgStat: 'STAT_STR',
        dmgStatMult: 1,
        attackBonus: 0,
      },
    };
    updateField('attacks', [...(ability.attacks || []), newAttack]);
  };

  const removeAttack = (index: number) => {
    const attacks = ability.attacks ?? [];
    const newAttacks = attacks.filter((_, i) => i !== index);

    if (!ability.name.trim()) {
      updateField('attacks', newAttacks);
      return;
    }

    const impacts = planWeaponAttackDeleteImpacts(
      ability.name,
      index,
      attacks,
      items,
      ability.restores ?? [],
    );

    if (impacts.length === 0) {
      updateField('attacks', newAttacks);
      return;
    }

    setAttackDeleteConfirm({ attackIndex: index, impacts });
  };

  const confirmRemoveAttack = () => {
    if (!attackDeleteConfirm) {
      return;
    }
    const { attackIndex } = attackDeleteConfirm;
    const attacks = ability.attacks ?? [];
    const newAttacks = attacks.filter((_, i) => i !== attackIndex);

    updateField('attacks', newAttacks);
    setItems(
      applyWeaponAttackDeleteToItems(
        ability.name,
        attackIndex,
        newAttacks,
        items,
        ability.restores ?? [],
      ),
    );
    setAttackDeleteConfirm(null);
  };

  const updateStatus = (index: number, status: AbilityStatus) => {
    const statuses = [...(ability.statuses || [])];
    statuses[index] = status;
    updateField('statuses', statuses);
  };

  const addStatus = () => {
    updateField('statuses', [
      ...(ability.statuses || []),
      { statusEffect: statusEffectOptions[0]?.value || '' },
    ]);
  };

  const removeStatus = (index: number) => {
    updateField(
      'statuses',
      ability.statuses?.filter((_, i) => i !== index) || []
    );
  };

  const updateRestore = (index: number, restore: AbilityRestore) => {
    const restores = [...(ability.restores || [])];
    restores[index] = restore;
    updateField('restores', restores);
  };

  const addRestore = () => {
    updateField('restores', [
      ...(ability.restores || []),
      {
        restoreWhich: 'CURRENT_STAT_HP',
        restoreDice: ['D6'],
        restoreBonus: 0,
        restoreStat: 'STAT_STR',
        restoreStatMult: 1,
      },
    ]);
  };

  const removeRestore = (index: number) => {
    updateField(
      'restores',
      ability.restores?.filter((_, i) => i !== index) || []
    );
  };

  return (
    <div className="item-form ability-template-form">
      <h2>Edit Ability</h2>
      <form>
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
              onChange={(value) => updateField('type', value as AbilityTemplate['type'])}
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
              onChange={(value) => updateField('costType', value as AbilityTemplate['costType'])}
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

        <div className="form-section">
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

          <div className="form-subsection">
            <h4>Attacks</h4>
            {ability.attacks?.map((attack, index) => (
              <AbilityAttackEditor
                key={index}
                attack={attack}
                index={index}
                onChange={(updated) => updateAttack(index, updated)}
                onRemove={() => removeAttack(index)}
              />
            ))}
            <Button type="button" variant="secondary" onClick={addAttack}>
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
                onChange={(updated) => updateStatus(index, updated)}
                onRemove={() => removeStatus(index)}
              />
            ))}
            <Button type="button" variant="secondary" onClick={addStatus}>
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
                onChange={(updated) => updateRestore(index, updated)}
                onRemove={() => removeRestore(index)}
              />
            ))}
            <Button type="button" variant="secondary" onClick={addRestore}>
              + Add Restore
            </Button>
          </div>
        </div>
      </form>

      <AbilityAttackDeleteModal
        isOpen={attackDeleteConfirm !== null}
        attackNumber={(attackDeleteConfirm?.attackIndex ?? 0) + 1}
        abilityName={ability.name}
        impacts={attackDeleteConfirm?.impacts ?? []}
        onConfirm={confirmRemoveAttack}
        onCancel={() => setAttackDeleteConfirm(null)}
      />
    </div>
  );
}
