import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { TextArea } from '../elements/TextArea';
import { Button } from '../elements/Button';
import {
  StatusEffectTemplate,
  createDefaultStatusEffectTemplate,
  StatusEffectEvent,
  Resistance,
  StatusEffectAction,
} from '../types/combat';
import {
  TargetSelectFields,
  StatsFields,
  CurrentStatsFields,
  StatusEffectEventEditor,
  ResistanceEditor,
  StatusEffectActionEditor,
} from './combat/CombatFormFields';
import { useAssets } from '../contexts/AssetsContext';
import { EditorEmptyState } from './EditorEmptyState';

export type { StatusEffectTemplate };
export { createDefaultStatusEffectTemplate };

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

  const updateEvent = (index: number, event: StatusEffectEvent) => {
    const events = [...statusEffect.events];
    events[index] = event;
    updateField('events', events);
  };

  const addEvent = () => {
    updateField('events', [
      ...statusEffect.events,
      { type: 'STATUS_EVENT_ON_APPLIED', condition: 'CONDITION_ALWAYS' },
    ]);
  };

  const removeEvent = (index: number) => {
    updateField(
      'events',
      statusEffect.events.filter((_, i) => i !== index),
    );
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
            id="status-duration"
            name="duration"
            label="Duration (turns)"
            value={statusEffect.duration}
            onChange={(value) => updateField('duration', value ?? 0)}
            min={0}
          />
        </div>

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
          <h4>Events</h4>
          <p className="form-subsection-description">
            The status effect will trigger when any of these happen.
          </p>
          {statusEffect.events.map((event, index) => (
            <StatusEffectEventEditor
              key={index}
              event={event}
              index={index}
              onChange={(updated) => updateEvent(index, updated)}
              onRemove={() => removeEvent(index)}
            />
          ))}
          <Button type="button" variant="secondary" onClick={addEvent}>
            + Add Event
          </Button>
        </div>

        <div className="form-subsection">
          <h4>Apply Bonuses (stats)</h4>
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
          <h4>Target Info (optional)</h4>
          <label style={{ display: 'flex', alignItems: 'center', gap: '4px' }}>
            <input
              type="checkbox"
              checked={!!statusEffect.targetInfo}
              onChange={(e) =>
                updateField(
                  'targetInfo',
                  e.target.checked
                    ? {
                        targetType: 'TARGET_UNIT',
                        allegianceSelectType: 'TARGET_ALLEGIANCE_OTHER',
                        numTargetableUnits: 1,
                        zoneSize: { x: 1, y: 1 },
                        range: 1,
                      }
                    : undefined,
                )
              }
            />
            Has target selection
          </label>
          {statusEffect.targetInfo && (
            <TargetSelectFields
              value={statusEffect.targetInfo}
              onChange={(targetInfo) => updateField('targetInfo', targetInfo)}
              idPrefix="status-target"
            />
          )}
        </div>

        <div className="form-subsection">
          <h4>Invoke Abilities</h4>
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
            + Add Invoked Ability
          </Button>
        </div>
      </form>
    </div>
  );
}
