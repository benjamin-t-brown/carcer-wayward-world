import { useEffect, useState } from 'react';
import {
  AbilityTemplate,
  createDefaultAbilityTemplate,
  sanitizeAbilityDepiction,
  ABILITY_TYPES,
  AbilityAttack,
  AbilityStatus,
  AbilityRestore,
  AbilityDamage,
  createDefaultAbilityDamage,
} from '../types/ability';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import {
  AbilityBasicsSection,
  AbilityEffectsSections,
  AbilityTargetingSection,
} from './ability/AbilityTemplateSections';
import { useAssets } from '../contexts/AssetsContext';
import {
  applyWeaponAttackDeleteToItems,
  planWeaponAttackDeleteImpacts,
  AbilityDeleteImpact,
} from '../types/assets';
import { AbilityAttackDeleteModal } from './AbilityAttackDeleteModal';
import { EditorEmptyState } from './EditorEmptyState';
import './AbilityTemplateForm.css';

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
      soundMap,
    );
    if (
      depiction.dmgAnim !== ability.depiction.dmgAnim ||
      depiction.projectileType !== ability.depiction.projectileType ||
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
  const updateField = <K extends keyof AbilityTemplate>(
    field: K,
    value: AbilityTemplate[K],
  ) => {
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
      damageType: 'DAMAGE_TYPE_EDGED',
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
      ability.statuses?.filter((_, i) => i !== index) || [],
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
      ability.restores?.filter((_, i) => i !== index) || [],
    );
  };

  const updateDamage = (index: number, damage: AbilityDamage) => {
    const damages = [...(ability.damages || [])];
    damages[index] = damage;
    updateField('damages', damages);
  };

  const addDamage = () => {
    updateField('damages', [
      ...(ability.damages || []),
      createDefaultAbilityDamage(),
    ]);
  };

  const removeDamage = (index: number) => {
    updateField(
      'damages',
      ability.damages?.filter((_, i) => i !== index) || [],
    );
  };

  return (
    <div className="item-form ability-template-form">
      <h2>Edit Ability</h2>
      <form>
        <AbilityBasicsSection ability={ability} updateField={updateField} />

        <div className="form-section">
          <AbilityTargetingSection
            ability={ability}
            updateField={updateField}
          />
          <AbilityEffectsSections
            ability={ability}
            statusEffectOptions={statusEffectOptions}
            onUpdateAttack={updateAttack}
            onAddAttack={addAttack}
            onRemoveAttack={removeAttack}
            onUpdateStatus={updateStatus}
            onAddStatus={addStatus}
            onRemoveStatus={removeStatus}
            onUpdateRestore={updateRestore}
            onAddRestore={addRestore}
            onRemoveRestore={removeRestore}
            onUpdateDamage={updateDamage}
            onAddDamage={addDamage}
            onRemoveDamage={removeDamage}
          />
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
