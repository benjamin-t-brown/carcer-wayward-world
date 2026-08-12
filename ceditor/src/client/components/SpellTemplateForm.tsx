import { TextInput } from '../elements/TextInput';
import { TextArea } from '../elements/TextArea';
import { SearchSelect } from '../elements/SearchSelect';
import { SpritePicker } from '../elements/SpritePicker';
import { Sprite } from '../elements/Sprite';
import { Button } from '../elements/Button';
import {
  SpellTemplate,
  SpellRuneRequirement,
  RuneType,
  RUNE_TYPES,
  createDefaultSpellTemplate,
  runeTypeToSpriteName,
} from '../types/spell';
import { useAssets } from '../contexts/AssetsContext';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { EditorEmptyState } from './EditorEmptyState';

export type { SpellTemplate };
export { createDefaultSpellTemplate };

interface SpellTemplateFormProps {
  spell?: SpellTemplate;
  updateSpell: (spell: SpellTemplate) => void;
}

function runeCountMap(
  requiredRunes: SpellRuneRequirement[],
): Map<RuneType, number> {
  const counts = new Map<RuneType, number>();
  for (const req of requiredRunes) {
    counts.set(req.type, Math.max(0, Math.floor(req.count)));
  }
  return counts;
}

function requiredRunesFromCounts(
  counts: Map<RuneType, number>,
): SpellRuneRequirement[] {
  const result: SpellRuneRequirement[] = [];
  for (const type of RUNE_TYPES) {
    const count = counts.get(type) ?? 0;
    if (count > 0) {
      result.push({ type, count });
    }
  }
  return result;
}

export function SpellTemplateForm(props: SpellTemplateFormProps) {
  const { abilities } = useAssets();
  const { spriteMap } = useSDL2WAssets();
  const spell = props.spell;

  if (!spell) {
    return <EditorEmptyState message="Select a spell to edit" />;
  }

  const setFormData = (data: SpellTemplate) => props.updateSpell(data);
  const updateField = <K extends keyof SpellTemplate>(
    field: K,
    value: SpellTemplate[K],
  ) => {
    setFormData({ ...spell, [field]: value });
  };

  const requiredRunes = spell.requiredRunes || [];
  const counts = runeCountMap(requiredRunes);

  const setRuneCount = (type: RuneType, nextCount: number) => {
    const next = new Map(counts);
    const clamped = Math.max(0, Math.floor(nextCount));
    if (clamped > 0) {
      next.set(type, clamped);
    } else {
      next.delete(type);
    }
    updateField('requiredRunes', requiredRunesFromCounts(next));
  };

  return (
    <div className="item-form spell-template-form">
      <h2>Edit Spell</h2>
      <form>
        <div className="form-subsection">
          <h4>Identity</h4>
          <div className="form-fields-inline">
            <TextInput
              id="spell-name"
              name="name"
              label="Name (ID)"
              value={spell.name}
              onChange={(value) => updateField('name', value)}
              required
            />
            <TextInput
              id="spell-label"
              name="label"
              label="Label"
              value={spell.label}
              onChange={(value) => updateField('label', value)}
            />
          </div>
          <div className="form-group form-block">
            <label htmlFor="spell-icon">Icon</label>
            <div style={{ marginTop: '4px' }}>
              <SpritePicker
                value={spell.icon}
                onChange={(value) => updateField('icon', value)}
                scale={2}
              />
            </div>
            {spell.icon ? (
              <div
                style={{ marginTop: '4px', fontSize: '11px', color: '#858585' }}
              >
                Selected: {spell.icon}
              </div>
            ) : null}
          </div>
          <div className="form-group form-block">
            <TextArea
              id="spell-description"
              name="description"
              label="Description"
              value={spell.description}
              onChange={(value) => updateField('description', value)}
              rows={3}
            />
          </div>
        </div>

        <div className="form-subsection">
          <h4>Ability</h4>
          <div className="form-fields-inline">
            <SearchSelect
              id="spell-ability"
              name="abilityName"
              label="Ability"
              className="spell-ability-field"
              value={spell.abilityName}
              onChange={(value) => updateField('abilityName', value)}
              items={abilities}
              getItemKey={(a) => a.name}
              getItemLabel={(a) => a.label?.trim() || a.name}
              searchFields={(a) => [a.name, a.label ?? '', a.type ?? '']}
              placeholder="Search abilities..."
              emptyLabel="(select ability)"
              required
              renderItem={(a) => (
                <>
                  <div style={{ fontWeight: 600 }}>
                    {a.label?.trim() || a.name}
                  </div>
                  <div
                    style={{
                      fontSize: '10px',
                      color: '#858585',
                      marginTop: '2px',
                    }}
                  >
                    {a.name}
                    {a.type ? ` • ${a.type}` : ''}
                  </div>
                </>
              )}
            />
            {spell.abilityName ? (
              <a
                className="template-edit-link"
                href={`#/editor/abilityTemplates?ability=${encodeURIComponent(spell.abilityName)}`}
              >
                Edit ability
              </a>
            ) : null}
          </div>
        </div>

        <div className="form-subsection">
          <h4>Required Runes</h4>
          <div id="required-runes-list">
            {RUNE_TYPES.map((type) => {
              const count = counts.get(type) ?? 0;
              const spriteName = runeTypeToSpriteName(type);
              return (
                <div
                  key={type}
                  className="status-effect-item"
                  style={{ padding: '10px 15px', marginBottom: '8px' }}
                >
                  <div
                    className="status-effect-header"
                    style={{ marginBottom: 0, gap: '12px' }}
                  >
                    <Sprite
                      sprite={spriteMap[spriteName]}
                      displaySize={32}
                    />
                    <span style={{ flex: 1, fontWeight: 600 }}>{type}</span>
                    <Button
                      type="button"
                      variant="small"
                      onClick={() => setRuneCount(type, count - 1)}
                      disabled={count <= 0}
                      ariaLabel={`Decrease ${type} rune cost`}
                    >
                      -
                    </Button>
                    <span
                      style={{
                        minWidth: '24px',
                        textAlign: 'center',
                        fontVariantNumeric: 'tabular-nums',
                      }}
                    >
                      {count}
                    </span>
                    <Button
                      type="button"
                      variant="small"
                      onClick={() => setRuneCount(type, count + 1)}
                      ariaLabel={`Increase ${type} rune cost`}
                    >
                      +
                    </Button>
                  </div>
                </div>
              );
            })}
          </div>
        </div>
      </form>
    </div>
  );
}
