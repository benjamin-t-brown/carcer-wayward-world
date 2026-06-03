import { useMemo } from 'react';
import { TextInput } from '../../elements/TextInput';
import { NumberInput } from '../../elements/NumberInput';
import { OptionSelect } from '../../elements/OptionSelect';
import { Button } from '../../elements/Button';
import { useSDL2WAssets } from '../../contexts/SDL2WAssetsContext';
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
  PROJECTILE_PATHS,
  ABILITY_COST_TYPES,
  STATUS_EVENT_TYPES,
  STATUS_EFFECT_CONDITIONS,
  STATUS_ACTION_TARGET_TYPES,
  CURRENT_STAT_ENUMS,
  Dice,
} from '../../types/combat';

function enumOptions<T extends string>(values: T[]) {
  return values.map((value) => ({ value, label: value }));
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
  allowEmpty: boolean
): { value: string; label: string }[] {
  const unique = new Set(names);
  if (currentValue && !unique.has(currentValue)) {
    unique.add(currentValue);
  }
  const sorted = [...unique].sort((a, b) => a.localeCompare(b));
  const options = allowEmpty ? [{ value: '', label: '(none)' }] : [];
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
  const update = <K extends keyof TargetSelectInfo>(field: K, fieldValue: TargetSelectInfo[K]) => {
    onChange({ ...value, [field]: fieldValue });
  };

  return (
    <div className="form-fields-inline">
      <OptionSelect
        id={`${idPrefix}-target-type`}
        name="targetType"
        label="Target Type"
        value={value.targetType}
        onChange={(v) => update('targetType', v as TargetSelectInfo['targetType'])}
        options={enumOptions(TARGET_SELECT_TYPES)}
      />
      <OptionSelect
        id={`${idPrefix}-allegiance`}
        name="allegianceSelectType"
        label="Allegiance"
        value={value.allegianceSelectType}
        onChange={(v) =>
          update('allegianceSelectType', v as TargetSelectInfo['allegianceSelectType'])
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
  const { animations, sounds } = useSDL2WAssets();

  const animationOptions = useMemo(
    () =>
      assetNameOptions(
        animations.map((a) => a.name),
        value.dmgAnim,
        false
      ),
    [animations, value.dmgAnim]
  );

  const projectileAnimOptions = useMemo(
    () =>
      assetNameOptions(
        animations.map((a) => a.name),
        value.projectileAnim,
        true
      ),
    [animations, value.projectileAnim]
  );

  const soundOptions = useMemo(
    () => (current: string) =>
      assetNameOptions(
        sounds.map((s) => s.name),
        current,
        false
      ),
    [sounds]
  );

  const update = <K extends keyof AbilityDepiction>(field: K, fieldValue: AbilityDepiction[K]) => {
    onChange({ ...value, [field]: fieldValue });
  };

  if (!compact) {
    return (
      <div className="depiction-fields">
      <div className="form-fields-inline">
        <OptionSelect
          id={`${idPrefix}-dmg-anim`}
          name="dmgAnim"
          label="Dmg Anim"
          value={value.dmgAnim}
          onChange={(v) => update('dmgAnim', v)}
          options={animationOptions}
        />
        <OptionSelect
          id={`${idPrefix}-projectile-anim`}
          name="projectileAnim"
          label="Projectile Anim"
          value={value.projectileAnim}
          onChange={(v) => update('projectileAnim', v)}
          options={projectileAnimOptions}
        />
        <OptionSelect
          id={`${idPrefix}-projectile-path`}
          name="projectilePath"
          label="Projectile Path"
          value={value.projectilePath}
          onChange={(v) => update('projectilePath', v as AbilityDepiction['projectilePath'])}
          options={enumOptions(PROJECTILE_PATHS)}
        />
        <OptionSelect
          id={`${idPrefix}-start-sound`}
          name="startSound"
          label="Start Sound"
          value={value.startSound}
          onChange={(v) => update('startSound', v)}
          options={soundOptions(value.startSound)}
        />
        <OptionSelect
          id={`${idPrefix}-dmg-sound`}
          name="dmgSound"
          label="Dmg Sound"
          value={value.dmgSound}
          onChange={(v) => update('dmgSound', v)}
          options={soundOptions(value.dmgSound)}
        />
        <label className="inline-checkbox">
          <input
            type="checkbox"
            checked={value.projectileHasFacing ?? false}
            onChange={(e) => update('projectileHasFacing', e.target.checked)}
          />
          Projectile Has Facing
        </label>
      </div>
      </div>
    );
  }

  return (
    <div className="depiction-fields">
      <div className="form-fields-inline">
        <OptionSelect
          id={`${idPrefix}-dmg-anim`}
          name="dmgAnim"
          label="Dmg Anim"
          value={value.dmgAnim}
          onChange={(v) => update('dmgAnim', v)}
          options={animationOptions}
        />
        <OptionSelect
          id={`${idPrefix}-projectile-anim`}
          name="projectileAnim"
          label="Projectile Anim"
          value={value.projectileAnim}
          onChange={(v) => update('projectileAnim', v)}
          options={projectileAnimOptions}
        />
        <OptionSelect
          id={`${idPrefix}-projectile-path`}
          name="projectilePath"
          label="Path"
          value={value.projectilePath}
          onChange={(v) => update('projectilePath', v as AbilityDepiction['projectilePath'])}
          options={enumOptions(PROJECTILE_PATHS)}
        />
        <label className="inline-checkbox">
          <input
            type="checkbox"
            checked={value.projectileHasFacing ?? false}
            onChange={(e) => update('projectileHasFacing', e.target.checked)}
          />
          Has Facing
        </label>
      </div>
      <div className="form-fields-inline">
        <OptionSelect
          id={`${idPrefix}-start-sound`}
          name="startSound"
          label="Start Sound"
          value={value.startSound}
          onChange={(v) => update('startSound', v)}
          options={soundOptions(value.startSound)}
        />
        <OptionSelect
          id={`${idPrefix}-dmg-sound`}
          name="dmgSound"
          label="Dmg Sound"
          value={value.dmgSound}
          onChange={(v) => update('dmgSound', v)}
          options={soundOptions(value.dmgSound)}
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
  const update = <K extends keyof AbilitySave>(field: K, fieldValue: AbilitySave[K]) => {
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
}: {
  value: AbilityAttackDmg;
  onChange: (value: AbilityAttackDmg) => void;
  idPrefix: string;
}) {
  const update = <K extends keyof AbilityAttackDmg>(field: K, fieldValue: AbilityAttackDmg[K]) => {
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
      <OptionSelect
        id={`${idPrefix}-dmg-stat`}
        name="dmgStat"
        label="Dmg Stat"
        value={value.dmgStat}
        onChange={(v) => update('dmgStat', v as AbilityAttackDmg['dmgStat'])}
        options={enumOptions(STATS_ENUMS)}
      />
      <NumberInput
        id={`${idPrefix}-dmg-stat-mult`}
        name="dmgStatMult"
        label="Stat Mult"
        value={value.dmgStatMult}
        onChange={(v) => update('dmgStatMult', v ?? 0)}
      />
      <NumberInput
        id={`${idPrefix}-attack-bonus`}
        name="attackBonus"
        label="Attack Bonus"
        value={value.attackBonus}
        onChange={(v) => update('attackBonus', v ?? 0)}
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

  const updateDmg = (field: keyof AbilityAttackDmg, fieldValue: AbilityAttackDmg[keyof AbilityAttackDmg]) => {
    onChange({ ...attack, dmg: { ...dmg, [field]: fieldValue } });
  };
  return (
    <div className="combat-list-item">
      <div className="form-fields-inline">
        <OptionSelect
          id={`${idPrefix}-class`}
          name="attackClass"
          label="Class"
          value={attack.attackClass}
          onChange={(v) => onChange({ ...attack, attackClass: v as AbilityAttack['attackClass'] })}
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
          onChange={(v) => updateDmg('dmgStat', v as AbilityAttackDmg['dmgStat'])}
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
        />
        <div className="form-group combat-list-item-remove">
          <label className="combat-list-item-remove-label" aria-hidden="true">
            &nbsp;
          </label>
          <div className="combat-list-item-remove-btn">
            <button type="button" className="btn btn-small btn-danger" onClick={onRemove}>
              Remove
            </button>
          </div>
        </div>
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
        {status.save && (
          <AbilitySaveFields
            value={status.save}
            onChange={(save) => onChange({ ...status, save })}
            idPrefix={`${idPrefix}-save`}
          />
        )}
        <div className="form-group combat-list-item-remove">
          <label className="combat-list-item-remove-label" aria-hidden="true">
            &nbsp;
          </label>
          <div className="combat-list-item-remove-btn">
            <button type="button" className="btn btn-small btn-danger" onClick={onRemove}>
              Remove
            </button>
          </div>
        </div>
      </div>
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
  const update = <K extends keyof AbilityRestore>(field: K, fieldValue: AbilityRestore[K]) => {
    onChange({ ...restore, [field]: fieldValue });
  };
  const diceString = restore.restoreDice.join(',');

  return (
    <div className="combat-list-item">
      <div className="form-fields-inline">
        <OptionSelect
          id={`${idPrefix}-which`}
          name="restoreWhich"
          label="Restore"
          value={restore.restoreWhich}
          onChange={(v) => update('restoreWhich', v as AbilityRestore['restoreWhich'])}
          options={enumOptions(CURRENT_STAT_ENUMS)}
        />
        <TextInput
          id={`${idPrefix}-dice`}
          name="restoreDice"
          label="Dice"
          value={diceString}
          onChange={(v) =>
            update(
              'restoreDice',
              v
                .split(',')
                .map((s) => s.trim())
                .filter(Boolean) as AbilityRestore['restoreDice']
            )
          }
        />
        <NumberInput
          id={`${idPrefix}-bonus`}
          name="restoreBonus"
          label="Bonus"
          value={restore.restoreBonus}
          onChange={(v) => update('restoreBonus', v ?? 0)}
        />
        <OptionSelect
          id={`${idPrefix}-stat`}
          name="restoreStat"
          label="Stat"
          value={restore.restoreStat}
          onChange={(v) => update('restoreStat', v as AbilityRestore['restoreStat'])}
          options={enumOptions(STATS_ENUMS)}
        />
        <NumberInput
          id={`${idPrefix}-mult`}
          name="statMult"
          label="× Stat"
          value={restore.restoreStatMult}
          onChange={(v) => update('restoreStatMult', v ?? 0)}
        />
        <div className="form-group combat-list-item-remove">
          <label className="combat-list-item-remove-label" aria-hidden="true">
            &nbsp;
          </label>
          <div className="combat-list-item-remove-btn">
            <button type="button" className="btn btn-small btn-danger" onClick={onRemove}>
              Remove
            </button>
          </div>
        </div>
      </div>
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
    <div className="status-item">
      <div className="form-fields-inline">
        <OptionSelect
          id={`${idPrefix}-type`}
          name="type"
          label="Event"
          value={event.type}
          onChange={(v) => onChange({ ...event, type: v as StatusEffectEvent['type'] })}
          options={enumOptions(STATUS_EVENT_TYPES)}
        />
        <OptionSelect
          id={`${idPrefix}-condition`}
          name="condition"
          label="Condition"
          value={event.condition}
          onChange={(v) =>
            onChange({ ...event, condition: v as StatusEffectEvent['condition'] })
          }
          options={enumOptions(STATUS_EFFECT_CONDITIONS)}
        />
      </div>
      <button type="button" className="btn btn-small" onClick={onRemove}>
        Remove Event
      </button>
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
    <div className="status-item">
      <div className="form-fields-inline">
        <OptionSelect
          id={`${idPrefix}-attack-type`}
          name="attackType"
          label="Attack Type"
          value={resistance.attackType}
          onChange={(v) =>
            onChange({ ...resistance, attackType: v as Resistance['attackType'] })
          }
          options={enumOptions(ATTACK_CLASSES)}
        />
        <NumberInput
          id={`${idPrefix}-mod`}
          name="mod"
          label="Mod"
          value={resistance.mod}
          onChange={(v) => onChange({ ...resistance, mod: v ?? 0 })}
        />
      </div>
      <button type="button" className="btn btn-small" onClick={onRemove}>
        Remove Resistance
      </button>
    </div>
  );
}

export function StatusEffectActionEditor({
  action,
  index,
  abilityOptions,
  onChange,
  onRemove,
}: {
  action: StatusEffectAction;
  index: number;
  abilityOptions: { value: string; label: string }[];
  onChange: (action: StatusEffectAction) => void;
  onRemove: () => void;
}) {
  const idPrefix = `status-action-${index}`;
  return (
    <div className="status-item">
      <div className="form-fields-inline">
        <OptionSelect
          id={`${idPrefix}-target`}
          name="statusActionTargetType"
          label="Target"
          value={action.statusActionTargetType}
          onChange={(v) =>
            onChange({
              ...action,
              statusActionTargetType: v as StatusEffectAction['statusActionTargetType'],
            })
          }
          options={enumOptions(STATUS_ACTION_TARGET_TYPES)}
        />
        <OptionSelect
          id={`${idPrefix}-action`}
          name="abilityName"
          label="Ability"
          value={action.abilityName}
          onChange={(v) => onChange({ ...action, abilityName: v })}
          options={abilityOptions}
        />
      </div>
      <button type="button" className="btn btn-small" onClick={onRemove}>
        Remove Invoked Action
      </button>
    </div>
  );
}

export { ABILITY_COST_TYPES, enumOptions };
