import { useEffect } from 'react';
import { SearchSelect } from '../elements/SearchSelect';
import { AbilityAttackDmgFields } from './combat/CombatFormFields';
import { useAssets } from '../contexts/AssetsContext';
import {
  ItemWeaponConfig,
  resolveItemWeaponDmgOverrides,
} from '../types/assets';
import { AbilityAttackDmg, mergeAbilityAttackDmg } from '../types/combat';

interface ItemWeaponFieldsProps {
  weapon: ItemWeaponConfig | undefined;
  onChange: (weapon: ItemWeaponConfig | undefined) => void;
  idPrefix: string;
}

export function ItemWeaponFields({
  weapon,
  onChange,
  idPrefix,
}: ItemWeaponFieldsProps) {
  const { abilities } = useAssets();
  const config: ItemWeaponConfig = weapon ?? { abilityName: '' };

  const selectedAbility = abilities.find((a) => a.name === config.abilityName);
  const attacks = selectedAbility?.attacks ?? [];
  const dmgOverrides = resolveItemWeaponDmgOverrides(config, attacks);

  useEffect(() => {
    if (!weapon?.abilityName || attacks.length === 0) {
      return;
    }
    const hasLegacy = weapon.dmg !== undefined || weapon.attackIndex !== undefined;
    const needsSeed =
      hasLegacy ||
      !weapon.dmgOverrides ||
      weapon.dmgOverrides.length !== attacks.length;
    if (!needsSeed) {
      return;
    }
    onChange({
      abilityName: weapon.abilityName,
      dmgOverrides: resolveItemWeaponDmgOverrides(weapon, attacks),
    });
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [weapon?.abilityName, attacks.length]);

  const setConfig = (patch: Partial<ItemWeaponConfig>) => {
    const next: ItemWeaponConfig = { ...config, ...patch };
    if (patch.dmgOverrides) {
      next.dmgOverrides = patch.dmgOverrides.map((d) => mergeAbilityAttackDmg(d));
    }
    onChange(next);
  };

  const handleAbilityChange = (abilityName: string) => {
    if (!abilityName) {
      onChange(undefined);
      return;
    }
    const ability = abilities.find((a) => a.name === abilityName);
    const base = ability?.attacks ?? [];
    onChange({
      abilityName,
      dmgOverrides:
        base.length > 0
          ? base.map((attack) => mergeAbilityAttackDmg(attack.dmg))
          : undefined,
    });
  };

  const updateAttackOverride = (index: number, dmg: AbilityAttackDmg) => {
    const next = [...dmgOverrides];
    next[index] = mergeAbilityAttackDmg(dmg);
    setConfig({ dmgOverrides: next });
  };

  return (
    <div className="form-subsection">
      <h4>Weapon</h4>
      <div className="weapon-ability-row">
        <SearchSelect
          id={`${idPrefix}-ability`}
          name="weaponAbilityName"
          label="Base Ability"
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
      {config.abilityName && attacks.length === 0 ? (
        <p style={{ fontSize: '12px', color: '#858585', margin: '6px 0 0' }}>
          Selected ability has no attacks. Add attacks on the ability template
          first.
        </p>
      ) : null}
      {config.abilityName && attacks.length > 0 ? (
        <div className="weapon-attack-overrides">
          <p className="weapon-override-hint">
            Damage overrides per attack from the base ability. Use + under Dice
            to add another die (e.g. 2d6).
          </p>
          {attacks.map((attack, index) => (
            <div key={index} className="weapon-attack-override">
              <h5 className="weapon-attack-override-title">
                {attacks.length > 1 ? `Attack ${index + 1}` : 'Attack'}
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
    </div>
  );
}
