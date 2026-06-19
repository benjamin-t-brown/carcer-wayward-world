import { useMemo, type ReactNode } from 'react';
import { NumberInput } from '../../elements/NumberInput';
import { OptionSelect } from '../../elements/OptionSelect';
import { Button } from '../../elements/Button';
import { AnimationPreview } from '../../elements/AnimationPreview';
import { SoundPreview } from '../../elements/SoundPreview';
import { SearchSelect } from '../../elements/SearchSelect';
import { useSDL2WAssets } from '../../contexts/SDL2WAssetsContext';
import { useAssets } from '../../contexts/AssetsContext';
import {
  TargetSelectInfo,
  Stats,
  CurrentStats,
  AbilityDepiction,
  AbilitySave,
  AbilityAttackDmg,
  AbilityAttack,
  AbilityStatus,
  AbilityRestore,
  StatusEffectEvent,
  StatusEffectAction,
  Resistance,
  TARGET_SELECT_TYPES,
  TARGET_ALLEGIANCE_TYPES,
  STATS_ENUMS,
  DICE_TYPES,
  ATTACK_CLASSES,
  ABILITY_PROJECTILE_PATHS,
  PROJECTILE_TYPES,
  abilityDepictionHasProjectile,
  setAbilityDepictionHasProjectile,
  ABILITY_COST_TYPES,
  STATUS_EVENT_TYPES,
  STATUS_EFFECT_CONDITIONS,
  STATUS_ACTION_TARGET_TYPES,
  CURRENT_STAT_ENUMS,
  Dice,
  DAMAGE_TYPES,
  AttackClass,
  attackClassUsesAttackBonus,
} from '../../types/ability';

function enumOptions<T extends string>(values: T[]) {
  return values.map((value) => ({ value, label: value }));
}

function AbilityListItemRemoveButton({
  onRemove,
  ariaLabel,
}: {
  onRemove: () => void;
  ariaLabel: string;
}) {
  return (
    <div className="form-group combat-list-item-remove">
      <label className="combat-list-item-remove-label" aria-hidden="true">
        &nbsp;
      </label>
      <div className="combat-list-item-remove-btn">
        <Button
          type="button"
          variant="small"
          className="btn-danger btn-card-action"
          ariaLabel={ariaLabel}
          onClick={onRemove}
        >
          X
        </Button>
      </div>
    </div>
  );
}

function normalizeDie(die: Dice | undefined): Dice {
  if (die && DICE_TYPES.includes(die)) {
    return die;
  }
  return 'D6';
}

function normalizeDiceList(dice: Dice[], minCount = 1): Dice[] {
  const normalized = dice.map((die) => normalizeDie(die));
  while (normalized.length < minCount) {
    normalized.push('D6');
  }
  return normalized;
}

function diceSelectOptions(current: string) {
  if (current && !DICE_TYPES.includes(current as Dice)) {
    return [{ value: current, label: current }, ...enumOptions(DICE_TYPES)];
  }
  return enumOptions(DICE_TYPES);
}

function DiceListEditor({
  idPrefix,
  label,
  dice,
  onChange,
  minCount = 1,
}: {
  idPrefix: string;
  label: string;
  dice: Dice[];
  onChange: (dice: Dice[]) => void;
  minCount?: number;
}) {
  const entries = normalizeDiceList(dice, minCount);

  const updateDie = (index: number, die: Dice) => {
    const next = [...entries];
    next[index] = die;
    onChange(next);
  };

  const removeDie = (index: number) => {
    if (entries.length <= minCount) {
      return;
    }
    onChange(entries.filter((_, i) => i !== index));
  };

  const addDie = () => {
    onChange([...entries, 'D6']);
  };

  return (
    <div className="form-group dice-list-field">
      <label>{label}</label>
      <div className="dice-list-entries">
        {entries.map((die, index) => (
          <div key={`${idPrefix}-${index}`} className="dice-list-entry">
            <OptionSelect
              id={`${idPrefix}-dice-${index}`}
              name={`${idPrefix}-dice-${index}`}
              className="dice-select-group"
              value={die}
              onChange={(v) => updateDie(index, v as Dice)}
              options={diceSelectOptions(die)}
            />
            {entries.length > minCount && (
              <Button
                variant="small"
                className="btn-danger btn-card-action"
                ariaLabel="Remove die"
                onClick={() => removeDie(index)}
              >
                X
              </Button>
            )}
          </div>
        ))}
        <Button
          type="button"
          variant="small"
          className="btn-card-action"
          ariaLabel="Add die"
          onClick={addDie}
        >
          +
        </Button>
      </div>
    </div>
  );
}

function assetNameOptions(
  names: string[],
  currentValue: string,
  allowEmpty: boolean,
  emptyLabel = '(none)',
): { value: string; label: string }[] {
  const unique = new Set(names);
  if (currentValue && !unique.has(currentValue)) {
    unique.add(currentValue);
  }
  const sorted = [...unique].sort((a, b) => a.localeCompare(b));
  const options = allowEmpty ? [{ value: '', label: emptyLabel }] : [];
  return [...options, ...sorted.map((name) => ({ value: name, label: name }))];
}

export function TargetSelectFields({
  value,
  onChange,
  idPrefix,
}: {
  value: TargetSelectInfo;
  onChange: (value: TargetSelectInfo) => void;
  idPrefix: string;
}) {
  const update = <K extends keyof TargetSelectInfo>(
    field: K,
    fieldValue: TargetSelectInfo[K],
  ) => {
    onChange({ ...value, [field]: fieldValue });
  };

  return (
    <div className="form-fields-inline">
      <OptionSelect
        id={`${idPrefix}-target-type`}
        name="targetType"
        label="Target Type"
        value={value.targetType}
        onChange={(v) =>
          update('targetType', v as TargetSelectInfo['targetType'])
        }
        options={enumOptions(TARGET_SELECT_TYPES)}
      />
      <OptionSelect
        id={`${idPrefix}-allegiance`}
        name="allegianceSelectType"
        label="Allegiance"
        value={value.allegianceSelectType}
        onChange={(v) =>
          update(
            'allegianceSelectType',
            v as TargetSelectInfo['allegianceSelectType'],
          )
        }
        options={enumOptions(TARGET_ALLEGIANCE_TYPES)}
      />
      <NumberInput
        id={`${idPrefix}-num-targets`}
        name="numTargetableUnits"
        label="Num Targets"
        value={value.numTargetableUnits}
        onChange={(v) => update('numTargetableUnits', v ?? 1)}
        min={1}
      />
      <NumberInput
        id={`${idPrefix}-zone-x`}
        name="zoneX"
        label="Zone W"
        value={value.zoneSize.x}
        onChange={(v) =>
          onChange({ ...value, zoneSize: { ...value.zoneSize, x: v ?? 1 } })
        }
        min={1}
      />
      <NumberInput
        id={`${idPrefix}-zone-y`}
        name="zoneY"
        label="Zone H"
        value={value.zoneSize.y}
        onChange={(v) =>
          onChange({ ...value, zoneSize: { ...value.zoneSize, y: v ?? 1 } })
        }
        min={1}
      />
      <NumberInput
        id={`${idPrefix}-range`}
        name="range"
        label="Range"
        value={value.range}
        onChange={(v) => update('range', v ?? 0)}
        min={0}
      />
    </div>
  );
}

export function StatsFields({
  value,
  onChange,
  idPrefix,
}: {
  value: Stats;
  onChange: (value: Stats) => void;
  idPrefix: string;
}) {
  const fields: (keyof Stats)[] = ['STR', 'MND', 'CON', 'AGI', 'LCK'];
  return (
    <div className="form-fields-inline">
      {fields.map((field) => (
        <NumberInput
          key={field}
          id={`${idPrefix}-${field}`}
          name={field}
          label={field}
          value={value[field] ?? 0}
          onChange={(v) => onChange({ ...value, [field]: v ?? 0 })}
        />
      ))}
    </div>
  );
}

export function CurrentStatsFields({
  value,
  onChange,
  idPrefix,
}: {
  value: CurrentStats;
  onChange: (value: CurrentStats) => void;
  idPrefix: string;
}) {
  const fields: (keyof CurrentStats)[] = ['HP', 'AP', 'MANA', 'AC'];
  return (
    <div className="form-fields-inline">
      {fields.map((field) => (
        <NumberInput
          key={field}
          id={`${idPrefix}-${field}`}
          name={field}
          label={field}
          value={value[field] ?? 0}
          onChange={(v) => onChange({ ...value, [field]: v ?? 0 })}
        />
      ))}
    </div>
  );
}

function DepictionAnimField({
  id,
  name,
  label,
  value,
  options,
  onChange,
}: {
  id: string;
  name: string;
  label: string;
  value: string;
  options: Array<{ value: string | number; label: string }>;
  onChange: (value: string) => void;
}) {
  return (
    <div className="depiction-preview-field">
      <AnimationPreview animationName={value} />
      <OptionSelect
        id={id}
        name={name}
        label={label}
        value={value}
        onChange={onChange}
        options={options}
      />
    </div>
  );
}

function DepictionSoundField({
  id,
  name,
  label,
  value,
  onChange,
}: {
  id: string;
  name: string;
  label: string;
  value: string;
  onChange: (value: string) => void;
}) {
  const { sounds } = useSDL2WAssets();

  const soundItems = useMemo(() => {
    const byName = new Map(sounds.map((sound) => [sound.name, sound]));
    if (value && !byName.has(value)) {
      byName.set(value, { name: value, path: '' });
    }
    return [...byName.values()].sort((a, b) => a.name.localeCompare(b.name));
  }, [sounds, value]);

  return (
    <div className="depiction-preview-field">
      <SoundPreview soundName={value} />
      <SearchSelect
        id={id}
        name={name}
        label={label}
        value={value}
        onChange={onChange}
        items={soundItems}
        getItemKey={(sound) => sound.name}
        getItemLabel={(sound) => sound.name}
        searchFields={(sound) => [sound.name]}
        placeholder="Search sounds..."
        emptyLabel="None"
        allowEmpty
      />
    </div>
  );
}

export function AbilityDepictionFields({
  value,
  onChange,
  idPrefix,
  compact = false,
}: {
  value: AbilityDepiction;
  onChange: (value: AbilityDepiction) => void;
  idPrefix: string;
  compact?: boolean;
}) {
  const { animations } = useSDL2WAssets();

  const animationOptions = useMemo(
    () =>
      assetNameOptions(
        animations.map((a) => a.name),
        value.dmgAnim,
        false,
      ),
    [animations, value.dmgAnim],
  );

  const projectileTypeOptions = useMemo(
    () =>
      enumOptions(
        PROJECTILE_TYPES.filter((type) => type !== 'PROJECTILE_NONE'),
      ),
    [],
  );

  const update = <K extends keyof AbilityDepiction>(
    field: K,
    fieldValue: AbilityDepiction[K],
  ) => {
    onChange({ ...value, [field]: fieldValue });
  };

  const hasProjectile = abilityDepictionHasProjectile(value);
  const projectilePathOptions = enumOptions(ABILITY_PROJECTILE_PATHS);

  const projectileFields = hasProjectile ? (
    <>
      <OptionSelect
        id={`${idPrefix}-projectile-type`}
        name="projectileType"
        label={compact ? 'Projectile' : 'Projectile Type'}
        value={value.projectileType}
        onChange={(v) =>
          update('projectileType', v as AbilityDepiction['projectileType'])
        }
        options={projectileTypeOptions}
      />
      <OptionSelect
        id={`${idPrefix}-projectile-path`}
        name="projectilePath"
        label={compact ? 'Path' : 'Projectile Path'}
        value={value.projectilePath}
        onChange={(v) =>
          update('projectilePath', v as AbilityDepiction['projectilePath'])
        }
        options={projectilePathOptions}
      />
    </>
  ) : null;

  if (!compact) {
    return (
      <div className="depiction-fields">
        <label className="inline-checkbox" style={{ marginBottom: '8px' }}>
          <input
            type="checkbox"
            checked={hasProjectile}
            onChange={(e) =>
              onChange(setAbilityDepictionHasProjectile(value, e.target.checked))
            }
          />
          Has projectile
        </label>
        <div className="form-fields-inline">
          <DepictionAnimField
            id={`${idPrefix}-dmg-anim`}
            name="dmgAnim"
            label="Dmg Anim"
            value={value.dmgAnim}
            onChange={(v) => update('dmgAnim', v)}
            options={animationOptions}
          />
          {projectileFields}
          <DepictionSoundField
            id={`${idPrefix}-start-sound`}
            name="startSound"
            label="Start Sound"
            value={value.startSound}
            onChange={(v) => update('startSound', v)}
          />
          <DepictionSoundField
            id={`${idPrefix}-dmg-sound`}
            name="dmgSound"
            label="Dmg Sound"
            value={value.dmgSound}
            onChange={(v) => update('dmgSound', v)}
          />
        </div>
      </div>
    );
  }

  return (
    <div className="depiction-fields">
      <label className="inline-checkbox" style={{ marginBottom: '8px' }}>
        <input
          type="checkbox"
          checked={hasProjectile}
          onChange={(e) =>
            onChange(setAbilityDepictionHasProjectile(value, e.target.checked))
          }
        />
        Has projectile
      </label>
      <div className="form-fields-inline">
        <DepictionAnimField
          id={`${idPrefix}-dmg-anim`}
          name="dmgAnim"
          label="Dmg Anim"
          value={value.dmgAnim}
          onChange={(v) => update('dmgAnim', v)}
          options={animationOptions}
        />
        {projectileFields}
      </div>
      <div className="form-fields-inline">
        <DepictionSoundField
          id={`${idPrefix}-start-sound`}
          name="startSound"
          label="Start Sound"
          value={value.startSound}
          onChange={(v) => update('startSound', v)}
        />
        <DepictionSoundField
          id={`${idPrefix}-dmg-sound`}
          name="dmgSound"
          label="Dmg Sound"
          value={value.dmgSound}
          onChange={(v) => update('dmgSound', v)}
        />
      </div>
    </div>
  );
}

export function AbilitySaveFields({
  value,
  onChange,
  idPrefix,
}: {
  value: AbilitySave;
  onChange: (value: AbilitySave) => void;
  idPrefix: string;
}) {
  const update = <K extends keyof AbilitySave>(
    field: K,
    fieldValue: AbilitySave[K],
  ) => {
    onChange({ ...value, [field]: fieldValue });
  };

  return (
    <div className="form-fields-inline">
      <OptionSelect
        id={`${idPrefix}-save-stat`}
        name="saveStat"
        label="Save Stat"
        value={value.saveStat}
        onChange={(v) => update('saveStat', v as AbilitySave['saveStat'])}
        options={enumOptions(STATS_ENUMS)}
      />
      <NumberInput
        id={`${idPrefix}-save-base`}
        name="saveBase"
        label="Save Base"
        value={value.saveBase}
        onChange={(v) => update('saveBase', v ?? 0)}
      />
      <OptionSelect
        id={`${idPrefix}-save-against`}
        name="saveAgainst"
        label="Save Against"
        value={value.saveAgainst}
        onChange={(v) => update('saveAgainst', v as AbilitySave['saveAgainst'])}
        options={enumOptions(STATS_ENUMS)}
      />
      <NumberInput
        id={`${idPrefix}-save-against-base`}
        name="saveAgainstBase"
        label="Against Base"
        value={value.saveAgainstBase}
        onChange={(v) => update('saveAgainstBase', v ?? 0)}
      />
    </div>
  );
}

export function AbilityAttackDmgFields({
  value,
  onChange,
  idPrefix,
  attackClass = 'ATTACK_CLASS_MELEE',
}: {
  value: AbilityAttackDmg;
  onChange: (value: AbilityAttackDmg) => void;
  idPrefix: string;
  attackClass?: AttackClass;
}) {
  const attackBonusEnabled = attackClassUsesAttackBonus(attackClass);
  const update = <K extends keyof AbilityAttackDmg>(
    field: K,
    fieldValue: AbilityAttackDmg[K],
  ) => {
    onChange({ ...value, [field]: fieldValue });
  };

  return (
    <div className="form-fields-inline">
      <DiceListEditor
        idPrefix={`${idPrefix}-dmg`}
        label="Dice"
        dice={value.dmgDice}
        onChange={(dmgDice) => update('dmgDice', dmgDice)}
      />
      <NumberInput
        id={`${idPrefix}-dmg-bonus`}
        name="dmgBonus"
        label="Dmg Bonus"
        value={value.dmgBonus}
        onChange={(v) => update('dmgBonus', v ?? 0)}
      />

      {/* <NumberInput
        id={`${idPrefix}-dmg-stat-mult`}
        name="dmgStatMult"
        label="Stat Mult"
        value={value.dmgStatMult}
        onChange={(v) => update('dmgStatMult', v ?? 0)}
      /> */}
      <NumberInput
        id={`${idPrefix}-attack-bonus`}
        name="attackBonus"
        label="Attack Bonus"
        value={value.attackBonus}
        onChange={(v) => update('attackBonus', v ?? 0)}
        disabled={!attackBonusEnabled}
      />
      <OptionSelect
        id={`${idPrefix}-dmg-stat`}
        name="dmgStat"
        label="Dmg Stat"
        value={value.dmgStat}
        onChange={(v) => update('dmgStat', v as AbilityAttackDmg['dmgStat'])}
        options={enumOptions(STATS_ENUMS)}
      />
    </div>
  );
}

export function AbilityAttackEditor({
  attack,
  index,
  onChange,
  onRemove,
}: {
  attack: AbilityAttack;
  index: number;
  onChange: (attack: AbilityAttack) => void;
  onRemove: () => void;
}) {
  const idPrefix = `attack-${index}`;
  const dmg = attack.dmg ?? {
    dmgDice: ['D6'] as AbilityAttackDmg['dmgDice'],
    dmgBonus: 0,
    dmgStat: 'STAT_STR' as AbilityAttackDmg['dmgStat'],
    dmgStatMult: 1,
    attackBonus: 0,
  };

  const updateDmg = (
    field: keyof AbilityAttackDmg,
    fieldValue: AbilityAttackDmg[keyof AbilityAttackDmg],
  ) => {
    onChange({ ...attack, dmg: { ...dmg, [field]: fieldValue } });
  };
  const attackBonusEnabled = attackClassUsesAttackBonus(attack.attackClass);
  return (
    <div className="combat-list-item">
      <div className="form-fields-inline">
        <OptionSelect
          id={`${idPrefix}-class`}
          name="attackClass"
          label="Class"
          value={attack.attackClass}
          onChange={(v) =>
            onChange({
              ...attack,
              attackClass: v as AbilityAttack['attackClass'],
            })
          }
          options={enumOptions(ATTACK_CLASSES)}
        />
        <DiceListEditor
          idPrefix={`${idPrefix}-dmg`}
          label="Dice"
          dice={dmg.dmgDice}
          onChange={(dmgDice) => updateDmg('dmgDice', dmgDice)}
        />
        <NumberInput
          id={`${idPrefix}-dmg-bonus`}
          name="dmgBonus"
          label="Dmg +"
          value={dmg.dmgBonus}
          onChange={(v) => updateDmg('dmgBonus', v ?? 0)}
        />
        <OptionSelect
          id={`${idPrefix}-dmg-stat`}
          name="dmgStat"
          label="Stat"
          value={dmg.dmgStat}
          onChange={(v) =>
            updateDmg('dmgStat', v as AbilityAttackDmg['dmgStat'])
          }
          options={enumOptions(STATS_ENUMS)}
        />
        {/* <NumberInput
          id={`${idPrefix}-dmg-stat-mult`}
          name="dmgStatMult"
          label="× Stat"
          value={dmg.dmgStatMult}
          onChange={(v) => updateDmg('dmgStatMult', v ?? 0)}
        /> */}
        <NumberInput
          id={`${idPrefix}-attack-bonus`}
          name="attackBonus"
          label="Atk +"
          value={dmg.attackBonus}
          onChange={(v) => updateDmg('attackBonus', v ?? 0)}
          disabled={!attackBonusEnabled}
        />
        <AbilityListItemRemoveButton
          onRemove={onRemove}
          ariaLabel="Remove attack"
        />
      </div>
      {attack.save && (
        <div className="form-fields-inline" style={{ marginTop: '6px' }}>
          <AbilitySaveFields
            value={attack.save}
            onChange={(save) => onChange({ ...attack, save })}
            idPrefix={`${idPrefix}-save`}
          />
        </div>
      )}
    </div>
  );
}

export function AbilityStatusEditor({
  status,
  index,
  statusEffectOptions,
  onChange,
  onRemove,
}: {
  status: AbilityStatus;
  index: number;
  statusEffectOptions: { value: string; label: string }[];
  onChange: (status: AbilityStatus) => void;
  onRemove: () => void;
}) {
  const idPrefix = `status-${index}`;
  return (
    <div className="combat-list-item">
      <div className="form-fields-inline">
        <OptionSelect
          id={`${idPrefix}-effect`}
          name="statusEffect"
          label="Status"
          value={status.statusEffect}
          onChange={(v) => onChange({ ...status, statusEffect: v })}
          options={statusEffectOptions}
        />
        <NumberInput
          id={`${idPrefix}-base-duration`}
          name="baseDuration"
          label="Base duration override"
          value={status.baseDuration ?? 0}
          onChange={(v) => {
            const next = { ...status };
            if (v === 0) {
              delete next.baseDuration;
            } else {
              next.baseDuration = v;
            }
            onChange(next);
          }}
          min={0}
        />
        <NumberInput
          id={`${idPrefix}-duration-bonus`}
          name="durationBonus"
          label="Duration bonus"
          value={status.durationBonus ?? 0}
          onChange={(v) => {
            const next = { ...status };
            if (v === 0) {
              delete next.durationBonus;
            } else {
              next.durationBonus = v;
            }
            onChange(next);
          }}
        />
        {status.save && (
          <AbilitySaveFields
            value={status.save}
            onChange={(save) => onChange({ ...status, save })}
            idPrefix={`${idPrefix}-save`}
          />
        )}
        <AbilityListItemRemoveButton
          onRemove={onRemove}
          ariaLabel="Remove status effect"
        />
      </div>
    </div>
  );
}

export function AbilityRestoreFields({
  value,
  onChange,
  idPrefix,
  trailing,
}: {
  value: AbilityRestore;
  onChange: (value: AbilityRestore) => void;
  idPrefix: string;
  trailing?: ReactNode;
}) {
  const update = <K extends keyof AbilityRestore>(
    field: K,
    fieldValue: AbilityRestore[K],
  ) => {
    onChange({ ...value, [field]: fieldValue });
  };

  return (
    <div className="form-fields-inline">
      <OptionSelect
        id={`${idPrefix}-which`}
        name="restoreWhich"
        label="Restore"
        value={value.restoreWhich}
        onChange={(v) =>
          update('restoreWhich', v as AbilityRestore['restoreWhich'])
        }
        options={enumOptions(CURRENT_STAT_ENUMS)}
      />
      <DiceListEditor
        idPrefix={`${idPrefix}-restore`}
        label="Dice"
        dice={value.restoreDice}
        onChange={(restoreDice) => update('restoreDice', restoreDice)}
      />
      <NumberInput
        id={`${idPrefix}-bonus`}
        name="restoreBonus"
        label="Bonus"
        value={value.restoreBonus}
        onChange={(v) => update('restoreBonus', v ?? 0)}
      />
      <OptionSelect
        id={`${idPrefix}-stat`}
        name="restoreStat"
        label="Stat"
        value={value.restoreStat}
        onChange={(v) =>
          update('restoreStat', v as AbilityRestore['restoreStat'])
        }
        options={enumOptions(STATS_ENUMS)}
      />
      <NumberInput
        id={`${idPrefix}-mult`}
        name="restoreStatMult"
        label="× Stat"
        value={value.restoreStatMult}
        onChange={(v) => update('restoreStatMult', v ?? 0)}
      />
      {trailing}
    </div>
  );
}

export function AbilityRestoreEditor({
  restore,
  index,
  onChange,
  onRemove,
}: {
  restore: AbilityRestore;
  index: number;
  onChange: (restore: AbilityRestore) => void;
  onRemove: () => void;
}) {
  const idPrefix = `restore-${index}`;

  return (
    <div className="combat-list-item">
      <AbilityRestoreFields
        value={restore}
        onChange={onChange}
        idPrefix={idPrefix}
        trailing={
          <AbilityListItemRemoveButton
            onRemove={onRemove}
            ariaLabel="Remove restore"
          />
        }
      />
    </div>
  );
}

export function StatusEffectEventEditor({
  event,
  index,
  onChange,
  onRemove,
}: {
  event: StatusEffectEvent;
  index: number;
  onChange: (event: StatusEffectEvent) => void;
  onRemove: () => void;
}) {
  const idPrefix = `event-${index}`;
  return (
    <div className="combat-list-item">
      <div className="form-fields-inline">
        <OptionSelect
          id={`${idPrefix}-type`}
          name="type"
          label="Event"
          value={event.type}
          onChange={(v) =>
            onChange({ ...event, type: v as StatusEffectEvent['type'] })
          }
          options={enumOptions(STATUS_EVENT_TYPES)}
          inputStyle={{
            width: '300px',
            maxWidth: 'unset',
          }}
        />
        <OptionSelect
          id={`${idPrefix}-condition`}
          name="condition"
          label="Condition"
          value={event.condition}
          onChange={(v) =>
            onChange({
              ...event,
              condition: v as StatusEffectEvent['condition'],
            })
          }
          options={enumOptions(STATUS_EFFECT_CONDITIONS)}
        />
        <AbilityListItemRemoveButton
          onRemove={onRemove}
          ariaLabel="Remove event"
        />
      </div>
    </div>
  );
}

export function ResistanceEditor({
  resistance,
  index,
  onChange,
  onRemove,
}: {
  resistance: Resistance;
  index: number;
  onChange: (resistance: Resistance) => void;
  onRemove: () => void;
}) {
  const idPrefix = `resistance-${index}`;
  return (
    <div className="combat-list-item">
      <div className="form-fields-inline">
        <OptionSelect
          id={`${idPrefix}-attack-type`}
          name="attackType"
          label="Damage Type"
          value={resistance.attackType}
          onChange={(v) =>
            onChange({
              ...resistance,
              attackType: v as Resistance['attackType'],
            })
          }
          options={enumOptions(DAMAGE_TYPES)}
        />
        <NumberInput
          id={`${idPrefix}-mod`}
          name="mod"
          label="Mod"
          value={resistance.mod}
          onChange={(v) => onChange({ ...resistance, mod: v ?? 0 })}
        />
        <AbilityListItemRemoveButton
          onRemove={onRemove}
          ariaLabel="Remove resistance"
        />
      </div>
    </div>
  );
}

export function StatusEffectActionEditor({
  action,
  index,
  onChange,
  onRemove,
}: {
  action: StatusEffectAction;
  index: number;
  onChange: (action: StatusEffectAction) => void;
  onRemove: () => void;
}) {
  const { abilities } = useAssets();
  const idPrefix = `status-action-${index}`;
  const events = action.events ?? [];

  const updateEvent = (eventIndex: number, event: StatusEffectEvent) => {
    const next = [...events];
    next[eventIndex] = event;
    onChange({ ...action, events: next });
  };

  const addEvent = () => {
    onChange({
      ...action,
      events: [
        ...events,
        { type: 'STATUS_EVENT_ON_APPLIED', condition: 'CONDITION_ALWAYS' },
      ],
    });
  };

  const removeEvent = (eventIndex: number) => {
    onChange({
      ...action,
      events: events.filter((_, i) => i !== eventIndex),
    });
  };

  return (
    <div className="combat-list-item">
      <div className="form-fields-inline">
        <OptionSelect
          id={`${idPrefix}-target`}
          name="statusActionTargetType"
          label="Target"
          value={action.statusActionTargetType}
          onChange={(v) =>
            onChange({
              ...action,
              statusActionTargetType:
                v as StatusEffectAction['statusActionTargetType'],
            })
          }
          options={enumOptions(STATUS_ACTION_TARGET_TYPES)}
          inputStyle={{
            width: '350px',
            maxWidth: 'unset',
          }}
        />
        <div className="invoke-ability-row">
          <SearchSelect
            id={`${idPrefix}-action`}
            name="abilityName"
            label="Ability"
            value={action.abilityName}
            onChange={(v) => onChange({ ...action, abilityName: v })}
            items={abilities}
            getItemKey={(a) => a.name}
            getItemLabel={(a) => a.label?.trim() || a.name}
            searchFields={(a) => [a.name, a.label ?? '', a.type ?? '']}
            placeholder="Search abilities..."
            emptyLabel="(select ability)"
            className="invoke-ability-search"
          />
          {action.abilityName ? (
            <a
              className="template-edit-link"
              href={`${window.location.origin}${window.location.pathname}#/editor/abilityTemplates?ability=${encodeURIComponent(action.abilityName)}`}
              target="_blank"
              rel="noopener noreferrer"
            >
              Edit ability
            </a>
          ) : null}
        </div>
        <AbilityListItemRemoveButton
          onRemove={onRemove}
          ariaLabel="Remove invoked ability"
        />
      </div>
      <div className="form-subsection" style={{ marginTop: '8px' }}>
        <h5>Events</h5>
        {events.map((event, eventIndex) => (
          <StatusEffectEventEditor
            key={eventIndex}
            event={event}
            index={eventIndex}
            onChange={(updated) => updateEvent(eventIndex, updated)}
            onRemove={() => removeEvent(eventIndex)}
          />
        ))}
        <Button type="button" variant="secondary" onClick={addEvent}>
          + Add Event
        </Button>
      </div>
    </div>
  );
}

export { ABILITY_COST_TYPES, enumOptions };
