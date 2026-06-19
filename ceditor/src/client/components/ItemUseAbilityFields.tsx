import { useEffect } from 'react';
import { SearchSelect } from '../elements/SearchSelect';
import {
  AbilityAttackDmgFields,
  AbilityRestoreFields,
} from './ability/AbilityFormFields';
import { useAssets } from '../contexts/AssetsContext';
import {
  ItemUseAbilityConfig,
  normalizeItemUseAbilityConfig,
  resolveItemUseAbilityRestoreOverrides,
  resolveItemWeaponDmgOverrides,
} from '../types/assets';
import {
  AbilityAttackDmg,
  AbilityRestore,
  mergeAbilityAttackDmg,
  mergeAbilityRestore,
} from '../types/ability';

interface ItemUseAbilityFieldsProps {
  useAbility: ItemUseAbilityConfig | undefined;
  onChange: (useAbility: ItemUseAbilityConfig | undefined) => void;
  idPrefix: string;
}

export function ItemUseAbilityFields({
  useAbility,
  onChange,
  idPrefix,
}: ItemUseAbilityFieldsProps) {
  const { abilities } = useAssets();
  const config: ItemUseAbilityConfig = useAbility ?? { abilityName: '' };

  const selectedAbility = abilities.find((a) => a.name === config.abilityName);
  const attacks = selectedAbility?.attacks ?? [];
  const restores = selectedAbility?.restores ?? [];
  const dmgOverrides = resolveItemWeaponDmgOverrides(config, attacks);
  const restoreOverrides = resolveItemUseAbilityRestoreOverrides(
    config,
    restores,
  );

  useEffect(() => {
    if (!useAbility?.abilityName) {
      return;
    }
    const needsDmgSeed =
      attacks.length > 0 &&
      (!useAbility.dmgOverrides ||
        useAbility.dmgOverrides.length !== attacks.length);
    const needsRestoreSeed =
      restores.length > 0 &&
      (!useAbility.restoreOverrides ||
        useAbility.restoreOverrides.length !== restores.length);
    if (!needsDmgSeed && !needsRestoreSeed) {
      return;
    }
    onChange(
      normalizeItemUseAbilityConfig(useAbility, {
        attacks,
        restores,
      }),
    );
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [useAbility?.abilityName, attacks.length, restores.length]);

  const setConfig = (patch: Partial<ItemUseAbilityConfig>) => {
    const next: ItemUseAbilityConfig = { ...config, ...patch };
    if (patch.dmgOverrides) {
      next.dmgOverrides = patch.dmgOverrides.map((d) => mergeAbilityAttackDmg(d));
    }
    if (patch.restoreOverrides) {
      next.restoreOverrides = patch.restoreOverrides.map((r) =>
        mergeAbilityRestore(r),
      );
    }
    onChange(next);
  };

  const handleAbilityChange = (abilityName: string) => {
    if (!abilityName) {
      onChange(undefined);
      return;
    }
    const ability = abilities.find((a) => a.name === abilityName);
    const baseAttacks = ability?.attacks ?? [];
    const baseRestores = ability?.restores ?? [];
    onChange(
      normalizeItemUseAbilityConfig(
        {
          abilityName,
          dmgOverrides:
            baseAttacks.length > 0
              ? baseAttacks.map((attack) => mergeAbilityAttackDmg(attack.dmg))
              : undefined,
          restoreOverrides:
            baseRestores.length > 0
              ? baseRestores.map((restore) => mergeAbilityRestore(restore))
              : undefined,
        },
        { attacks: baseAttacks, restores: baseRestores },
      ),
    );
  };

  const updateAttackOverride = (index: number, dmg: AbilityAttackDmg) => {
    const next = [...dmgOverrides];
    next[index] = mergeAbilityAttackDmg(dmg);
    setConfig({ dmgOverrides: next });
  };

  const updateRestoreOverride = (index: number, restore: AbilityRestore) => {
    const next = [...restoreOverrides];
    next[index] = mergeAbilityRestore(restore);
    setConfig({ restoreOverrides: next });
  };

  return (
    <div className="form-subsection item-use-ability-fields">
      <div className="weapon-ability-row">
        <SearchSelect
          id={`${idPrefix}-ability`}
          name="useAbilityName"
          label="Use Ability"
          value={config.abilityName}
          onChange={handleAbilityChange}
          items={abilities}
          getItemKey={(a) => a.name}
          getItemLabel={(a) => a.label?.trim() || a.name}
          searchFields={(a) => [a.name, a.label ?? '', a.type ?? '']}
          placeholder="Search abilities..."
          emptyLabel="(select ability)"
          renderItem={(a) => (
            <>
              <div style={{ fontWeight: 600 }}>
                {a.label?.trim() || a.name}
              </div>
              <div
                style={{ fontSize: '10px', color: '#858585', marginTop: '2px' }}
              >
                {a.name}
                {a.type ? ` • ${a.type}` : ''}
              </div>
            </>
          )}
        />
        {config.abilityName ? (
          <a
            className="template-edit-link"
            href={`${window.location.origin}${window.location.pathname}#/editor/abilityTemplates?ability=${encodeURIComponent(config.abilityName)}`}
            target="_blank"
            rel="noopener noreferrer"
          >
            Edit ability
          </a>
        ) : null}
      </div>
      {config.abilityName &&
      attacks.length === 0 &&
      restores.length === 0 ? (
        <p style={{ fontSize: '12px', color: '#858585', margin: '6px 0 0' }}>
          Selected ability has no attacks or restores to override.
        </p>
      ) : null}
      {config.abilityName && attacks.length > 0 ? (
        <div className="weapon-attack-overrides">
          <p className="weapon-override-hint">
            Damage overrides per attack from the base ability.
          </p>
          {attacks.map((attack, index) => (
            <div key={index} className="weapon-attack-override">
              <h5 className="weapon-attack-override-title">
                {attacks.length > 1 ? `Attack Override ${index + 1}` : 'Attack Override'}
              </h5>
              <div className="weapon-dmg-overrides">
                <AbilityAttackDmgFields
                  value={dmgOverrides[index]}
                  onChange={(dmg) => updateAttackOverride(index, dmg)}
                  idPrefix={`${idPrefix}-attack-${index}`}
                  attackClass={attack.attackClass}
                />
              </div>
            </div>
          ))}
        </div>
      ) : null}
      {config.abilityName && restores.length > 0 ? (
        <div className="weapon-attack-overrides">
          <p className="weapon-override-hint">
            Restore overrides per entry from the base ability.
          </p>
          {restores.map((_, index) => (
            <div key={index} className="weapon-attack-override">
              <h5 className="weapon-attack-override-title">
                {restores.length > 1 ? `Restore Override ${index + 1}` : 'Restore Override'}
              </h5>
              <div className="weapon-dmg-overrides">
                <AbilityRestoreFields
                  value={restoreOverrides[index]}
                  onChange={(restore) => updateRestoreOverride(index, restore)}
                  idPrefix={`${idPrefix}-restore-${index}`}
                />
              </div>
            </div>
          ))}
        </div>
      ) : null}
    </div>
  );
}
