import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { OptionSelect } from '../elements/OptionSelect';
import { TextArea } from '../elements/TextArea';
import { Button } from '../elements/Button';
import {
  FeatTemplate,
  createDefaultFeatTemplate,
} from '../types/assets';
import { useAssets } from '../contexts/AssetsContext';
import { EditorEmptyState } from './EditorEmptyState';

export type { FeatTemplate };
export { createDefaultFeatTemplate };

interface FeatTemplateFormProps {
  feat?: FeatTemplate;
  updateFeat: (feat: FeatTemplate) => void;
}

export function FeatTemplateForm(props: FeatTemplateFormProps) {
  const { feats: allFeats } = useAssets();
  const feat = props.feat;

  if (!feat) {
    return <EditorEmptyState message="Select a feat to edit" />;
  }

  const setFormData = (data: FeatTemplate) => {
    props.updateFeat(data);
  };

  const updateField = <K extends keyof FeatTemplate>(
    field: K,
    value: FeatTemplate[K]
  ) => {
    setFormData({ ...feat, [field]: value });
  };

  // const updateEffect = (index: number, effect: FeatEffect) => {
  //   const effects = [...(feat.effects || [])];
  //   effects[index] = effect;
  //   setFormData({ ...feat, effects });
  // };

  // const addEffect = () => {
  //   setFormData({
  //     ...feat,
  //     effects: [
  //       ...(feat.effects || []),
  //       { type: 'MODIFY_DERIVED', when: 'ALWAYS', target: '', op: 'ADD', value: 0 },
  //     ],
  //   });
  // };

  // const removeEffect = (index: number) => {
  //   setFormData({
  //     ...feat,
  //     effects: feat.effects?.filter((_, i) => i !== index) || [],
  //   });
  // };

  // const toggleRequiredFeat = (featId: string) => {
  //   const current = feat.requiresFeats || [];
  //   const next = current.includes(featId)
  //     ? current.filter((id) => id !== featId)
  //     : [...current, featId];
  //   setFormData({ ...feat, requiresFeats: next });
  // };

  const prerequisiteOptions = allFeats.filter((entry) => entry.id !== feat.id);

  return (
    <div className="item-form feat-template-form">
      <h2>Edit Feat</h2>
      <form>
        <div className="form-fields-inline">
          <TextInput
            id="feat-id"
            name="id"
            label="ID"
            value={feat.id}
            onChange={(value) => updateField('id', value)}
            required
          />
          <TextInput
            id="feat-label"
            name="label"
            label="Label"
            value={feat.label}
            onChange={(value) => updateField('label', value)}
            required
          />
          <OptionSelect
            id="feat-implementation"
            name="implementation"
            label="Implementation"
            value={feat.implementation || 'DATA'}
            onChange={(value) =>
              updateField('implementation', value as FeatTemplate['implementation'])
            }
            options={[
              { value: 'DATA', label: 'DATA' },
              { value: 'CUSTOM', label: 'CUSTOM' },
            ]}
          />
          {/* <TextInput
            id="feat-excludes-group"
            name="excludesGroup"
            label="Excludes Group"
            value={feat.excludesGroup || ''}
            onChange={(value) => updateField('excludesGroup', value)}
            placeholder="e.g., build_size"
          /> */}
        </div>

        <div className="form-group form-block">
          <TextArea
            id="feat-description"
            name="description"
            label="Description"
            value={feat.description || ''}
            onChange={(value) => updateField('description', value)}
            rows={3}
          />
        </div>
{/* 
        <div className="form-subsection">
          <h4>Prerequisites</h4>
          <div className="form-fields-inline">
            {prerequisiteOptions.map((entry) => (
              <label key={entry.id} style={{ display: 'flex', alignItems: 'center', gap: '4px' }}>
                <input
                  type="checkbox"
                  checked={(feat.requiresFeats || []).includes(entry.id)}
                  onChange={() => toggleRequiredFeat(entry.id)}
                />
                {entry.id}
              </label>
            ))}
          </div>
        </div> */}

        {/* <div className="form-subsection">
          <h4>Effects</h4>
          {feat.effects?.map((effect, index) => (
            <div key={index} className="status-item">
              <div className="form-fields-inline">
                <OptionSelect
                  id={`effect-type-${index}`}
                  name={`effectType${index}`}
                  label="Type"
                  value={effect.type}
                  onChange={(value) =>
                    updateEffect(index, { ...effect, type: value as FeatEffect['type'] })
                  }
                  options={FEAT_EFFECT_TYPES.map((type) => ({ value: type, label: type }))}
                />
                <OptionSelect
                  id={`effect-when-${index}`}
                  name={`effectWhen${index}`}
                  label="When"
                  value={effect.when || 'ALWAYS'}
                  onChange={(value) =>
                    updateEffect(index, {
                      ...effect,
                      when: value as FeatEffect['when'],
                    })
                  }
                  options={FEAT_CONDITION_TYPES.map((when) => ({ value: when, label: when }))}
                />
                <TextInput
                  id={`effect-target-${index}`}
                  name={`effectTarget${index}`}
                  label="Target"
                  value={effect.target || ''}
                  onChange={(value) => updateEffect(index, { ...effect, target: value })}
                />
                <TextInput
                  id={`effect-op-${index}`}
                  name={`effectOp${index}`}
                  label="Op"
                  value={effect.op || 'ADD'}
                  onChange={(value) => updateEffect(index, { ...effect, op: value })}
                />
                <NumberInput
                  id={`effect-value-${index}`}
                  name={`effectValue${index}`}
                  label="Value"
                  value={effect.value || 0}
                  onChange={(value) => updateEffect(index, { ...effect, value: value || 0 })}
                />
              </div>
              <Button type="button" variant="small" className="btn-danger" onClick={() => removeEffect(index)}>
                Remove Effect
              </Button>
            </div>
          ))}
          <Button type="button" variant="secondary" onClick={addEffect}>
            + Add Effect
          </Button>
        </div> */}
      </form>
    </div>
  );
}
