import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { TextArea } from '../elements/TextArea';
import { Button } from '../elements/Button';
import {
  StatusEffectTemplate,
  createDefaultStatusEffectTemplate,
  createDefaultStatusEffectDurationScale,
  Resistance,
  StatusEffectAction,
  StatsEnum,
  STATS_ENUMS,
} from '../types/combat';
import {
  StatsFields,
  CurrentStatsFields,
  ResistanceEditor,
  StatusEffectActionEditor,
} from './combat/CombatFormFields';
import { OptionSelect } from '../elements/OptionSelect';
import { useAssets } from '../contexts/AssetsContext';
import { EditorEmptyState } from './EditorEmptyState';

export type { StatusEffectTemplate };
export { createDefaultStatusEffectTemplate };

function enumOptions<T extends string>(values: readonly T[]) {
  return values.map((v) => ({ value: v, label: v }));
}

interface StatusEffectTemplateFormProps {
  statusEffect?: StatusEffectTemplate;
  updateStatusEffect: (statusEffect: StatusEffectTemplate) => void;
}

export function StatusEffectTemplateForm(props: StatusEffectTemplateFormProps) {
  const { abilities } = useAssets();
  const statusEffect = props.statusEffect;

  if (!statusEffect) {
    return <EditorEmptyState message="Select a status effect to edit" />;
  }

  const setFormData = (data: StatusEffectTemplate) =>
    props.updateStatusEffect(data);
  const updateField = <K extends keyof StatusEffectTemplate>(
    field: K,
    value: StatusEffectTemplate[K],
  ) => {
    setFormData({ ...statusEffect, [field]: value });
  };

  const updateResistance = (index: number, resistance: Resistance) => {
    const applyResistances = [...(statusEffect.applyResistances || [])];
    applyResistances[index] = resistance;
    updateField('applyResistances', applyResistances);
  };

  const addResistance = () => {
    updateField('applyResistances', [
      ...(statusEffect.applyResistances || []),
      { attackType: 'DAMAGE_TYPE_EDGED', mod: 0 },
      { attackType: 'DAMAGE_TYPE_BASHING', mod: 0 },
      { attackType: 'DAMAGE_TYPE_PIERCING', mod: 0 },
      { attackType: 'DAMAGE_TYPE_HEAT', mod: 0 },
      { attackType: 'DAMAGE_TYPE_FREEZE', mod: 0 },
      { attackType: 'DAMAGE_TYPE_STATIC', mod: 0 },
      { attackType: 'DAMAGE_TYPE_NECROTIC', mod: 0 },
      { attackType: 'DAMAGE_TYPE_EPHEMERAL', mod: 0 },
      { attackType: 'DAMAGE_TYPE_TRUE', mod: 0 },
    ]);
  };

  const removeResistance = (index: number) => {
    updateField(
      'applyResistances',
      statusEffect.applyResistances?.filter((_, i) => i !== index) || [],
    );
  };

  const updateInvokedAction = (index: number, invoked: StatusEffectAction) => {
    const actions = [...(statusEffect.actions || [])];
    actions[index] = invoked;
    updateField('actions', actions);
  };

  const addInvokedAction = () => {
    updateField('actions', [
      ...(statusEffect.actions || []),
      {
        statusActionTargetType: 'STATUS_ACTION_TARGET_SELF',
        abilityName: abilities[0]?.name || '',
        events: [
          { type: 'STATUS_EVENT_ON_APPLIED', condition: 'CONDITION_ALWAYS' },
        ],
      },
    ]);
  };

  const removeInvokedAction = (index: number) => {
    updateField(
      'actions',
      statusEffect.actions?.filter((_, i) => i !== index) || [],
    );
  };

  return (
    <div className="item-form status-effect-template-form">
      <h2>Edit Status Effect</h2>
      <form>
        <div className="form-fields-inline">
          <TextInput
            id="status-name"
            name="name"
            label="Name (ID)"
            value={statusEffect.name}
            onChange={(value) => updateField('name', value)}
            required
          />
          <NumberInput
            id="status-base-duration"
            name="baseDuration"
            label="Base duration (turns)"
            value={statusEffect.baseDuration}
            onChange={(value) => updateField('baseDuration', value ?? 0)}
            min={0}
          />
        </div>

        <p className="form-subsection-description">
          Final duration is resolved at apply time from applier/victim stats and
          feats. Base duration is the default before those modifiers.
        </p>

        <div className="form-group form-block">
          <TextArea
            id="status-description"
            name="description"
            label="Description"
            value={statusEffect.description}
            onChange={(value) => updateField('description', value)}
            rows={3}
          />
        </div>

        <div className="form-subsection">
          <h4>Duration scale (optional)</h4>
          <p className="form-subsection-description">
            Extra turns from the applier&apos;s stat: floor(stat × multiplier).
          </p>
          <label style={{ display: 'flex', alignItems: 'center', gap: '4px' }}>
            <input
              type="checkbox"
              checked={!!statusEffect.durationScale}
              onChange={(e) =>
                updateField(
                  'durationScale',
                  e.target.checked
                    ? createDefaultStatusEffectDurationScale()
                    : undefined,
                )
              }
            />
            Scale duration from applier stat
          </label>
          {statusEffect.durationScale && (
            <div className="form-fields-inline">
              <OptionSelect
                id="status-duration-stat"
                name="durationStat"
                label="Stat"
                value={statusEffect.durationScale.durationStat}
                onChange={(v) =>
                  updateField('durationScale', {
                    ...statusEffect.durationScale!,
                    durationStat: v as StatsEnum,
                  })
                }
                options={enumOptions(STATS_ENUMS)}
              />
              <NumberInput
                id="status-duration-stat-mult"
                name="durationStatMult"
                label="× Stat"
                value={statusEffect.durationScale.durationStatMult}
                onChange={(v) =>
                  updateField('durationScale', {
                    ...statusEffect.durationScale!,
                    durationStatMult: v ?? 0,
                  })
                }
              />
            </div>
          )}
        </div>

        <div className="form-subsection">
          <h4>Apply Bonuses (stats)</h4>
          <p className="form-subsection-description">
            Passive modifiers on the bearer while this status is active.
          </p>
          <label style={{ display: 'flex', alignItems: 'center', gap: '4px' }}>
            <input
              type="checkbox"
              checked={!!statusEffect.applyBonuses}
              onChange={(e) =>
                updateField(
                  'applyBonuses',
                  e.target.checked
                    ? { STR: 0, MND: 0, CON: 0, AGI: 0, LCK: 0 }
                    : undefined,
                )
              }
            />
            Enable stat bonuses
          </label>
          {statusEffect.applyBonuses && (
            <StatsFields
              value={statusEffect.applyBonuses}
              onChange={(applyBonuses) =>
                updateField('applyBonuses', applyBonuses)
              }
              idPrefix="status-bonus"
            />
          )}
        </div>

        <div className="form-subsection">
          <h4>Apply Current Stat Change</h4>
          <p className="form-subsection-description">
            Passive current-stat change on the bearer while active.
          </p>
          <label style={{ display: 'flex', alignItems: 'center', gap: '4px' }}>
            <input
              type="checkbox"
              checked={!!statusEffect.applyCurrentStatChange}
              onChange={(e) =>
                updateField(
                  'applyCurrentStatChange',
                  e.target.checked
                    ? { HP: 0, AP: 0, MANA: 0, AC: 0 }
                    : undefined,
                )
              }
            />
            Enable current stat change
          </label>
          {statusEffect.applyCurrentStatChange && (
            <CurrentStatsFields
              value={statusEffect.applyCurrentStatChange}
              onChange={(applyCurrentStatChange) =>
                updateField('applyCurrentStatChange', applyCurrentStatChange)
              }
              idPrefix="status-current"
            />
          )}
        </div>

        <div className="form-subsection">
          <h4>Resistances</h4>
          {statusEffect.applyResistances?.map((resistance, index) => (
            <ResistanceEditor
              key={index}
              resistance={resistance}
              index={index}
              onChange={(updated) => updateResistance(index, updated)}
              onRemove={() => removeResistance(index)}
            />
          ))}
          <Button type="button" variant="secondary" onClick={addResistance}>
            + Add Resistance
          </Button>
        </div>

        <div className="form-subsection">
          <h4>Triggered abilities</h4>
          <p className="form-subsection-description">
            Only invoked abilities are event-driven. Each action lists the game
            events that trigger it.
          </p>
          {statusEffect.actions?.map((invoked, index) => (
            <StatusEffectActionEditor
              key={index}
              action={invoked}
              index={index}
              onChange={(updated) => updateInvokedAction(index, updated)}
              onRemove={() => removeInvokedAction(index)}
            />
          ))}
          <Button type="button" variant="secondary" onClick={addInvokedAction}>
            + Add Triggered Ability
          </Button>
        </div>
      </form>
    </div>
  );
}
