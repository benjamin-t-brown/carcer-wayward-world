import { useMemo } from 'react';
import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { OptionSelect } from '../elements/OptionSelect';
import { SearchSelect } from '../elements/SearchSelect';
import { SoundSearchField } from '../elements/SoundSearchField';
import { Button } from '../elements/Button';
import { SpritePicker } from '../elements/SpritePicker';
import { useAssets } from '../contexts/AssetsContext';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { EditorEmptyState } from './EditorEmptyState';
import {
  CharacterTemplate,
  CharacterTemplateBehaviorName,
  CHARACTER_TEMPLATE_BEHAVIOR_NAMES,
  CharacterSpriteLayout,
  CHARACTER_SPRITE_LAYOUTS,
  CombatBehaviorName,
  COMBAT_BEHAVIOR_NAMES,
  CombatSpellAi,
  CombatSpellAiCondition,
  CombatSpellAiConditionType,
  CombatSpellAiRule,
  CombatSpellAiSelection,
  CombatSpellAiTurnMode,
  CombatSpellAiZoneTargeting,
  COMBAT_SPELL_AI_CONDITION_TYPES,
  COMBAT_SPELL_AI_SELECTIONS,
  COMBAT_SPELL_AI_ZONE_TARGETINGS,
  createDefaultCharacterStats,
  createGenericTownspersonStats,
} from '../types/assets';

const DEFAULT_COMBAT_BEHAVIOR: CombatBehaviorName = 'SEEK_AND_MELEE';

// Re-export for backward compatibility
export type { CharacterTemplate };

const CHARACTER_TYPES = [
  'TOWNSPERSON',
  'TOWNSPERSON_STATIC',
  'ENEMY',
  'ENEMY_STATIC',
];

interface CharacterTemplateFormProps {
  character?: CharacterTemplate;
  updateCharacter: (character: CharacterTemplate) => void;
}

export function createDefaultCharacter(): CharacterTemplate {
  return {
    type: 'TOWNSPERSON',
    name: '',
    label: '',
    spritesheet: 'actors0',
    spriteOffset: 0,
    stats: createDefaultCharacterStats(),
  };
}

export function CharacterTemplateForm(props: CharacterTemplateFormProps) {
  const character = props.character;
  const { gameEvents, dropTables, abilities, spells } = useAssets();
  const { spriteMap } = useSDL2WAssets();

  const formData = character as CharacterTemplate;
  const setFormData = (data: CharacterTemplate) => {
    props.updateCharacter(data);
  };

  const stats = formData?.stats ?? createDefaultCharacterStats();

  const talkEvents = useMemo(() => {
    const talkOnly = gameEvents.filter((event) => event.eventType === 'TALK');
    const currentId = formData?.talk?.talkName;
    if (currentId && !talkOnly.some((event) => event.id === currentId)) {
      const current = gameEvents.find((event) => event.id === currentId);
      if (current) {
        return [...talkOnly, current];
      }
    }
    return talkOnly;
  }, [gameEvents, formData?.talk?.talkName]);

  const spriteName = `${formData?.spritesheet}_${formData?.spriteOffset}`;

  const handleSpriteChange = (spriteName: string) => {
    const selectedSprite = spriteMap[spriteName];
    if (selectedSprite) {
      setFormData({
        ...formData,
        spritesheet: selectedSprite.pictureAlias,
        spriteOffset: selectedSprite.index,
      });
    }
  };

  const updateField = <K extends keyof CharacterTemplate>(
    field: K,
    value: CharacterTemplate[K]
  ) => {
    setFormData({ ...formData, [field]: value });
  };

  const updateTalk = (field: 'talkName' | 'portraitName', value: string) => {
    setFormData({
      ...formData,
      talk: {
        ...formData.talk,
        [field]: value || undefined,
      },
    });
  };

  const updateBehavior = (value: string) => {
    if (!value) {
      const { behavior: _behavior, ...rest } = formData;
      setFormData(rest);
      return;
    }
    setFormData({
      ...formData,
      behavior: {
        behaviorName: value as CharacterTemplateBehaviorName,
      },
    });
  };

  const updateCombatBehavior = (
    field: 'town' | 'combat',
    value: string
  ) => {
    const nextTown =
      field === 'town'
        ? ((value || DEFAULT_COMBAT_BEHAVIOR) as CombatBehaviorName)
        : (formData.combatBehavior?.town ?? DEFAULT_COMBAT_BEHAVIOR);
    const nextCombat =
      field === 'combat'
        ? ((value || DEFAULT_COMBAT_BEHAVIOR) as CombatBehaviorName)
        : (formData.combatBehavior?.combat ?? DEFAULT_COMBAT_BEHAVIOR);
    setFormData({
      ...formData,
      combatBehavior: {
        town: nextTown,
        combat: nextCombat,
      },
    });
  };

  const updateGenericStat = (
    field: 'str' | 'mnd' | 'con' | 'agi' | 'lck',
    value: number | undefined
  ) => {
    setFormData({
      ...formData,
      stats: {
        ...stats,
        generic: {
          ...stats.generic,
          [field]: value,
        },
      },
    });
  };

  const updateWeaponMastery = (
    field: 'edged' | 'pole' | 'blunt' | 'range' | 'unarmed',
    value: number | undefined
  ) => {
    setFormData({
      ...formData,
      stats: {
        ...stats,
        trainable: {
          ...stats.trainable,
          weapon: {
            ...stats.trainable?.weapon,
            [field]: value,
          },
        },
      },
    });
  };

  const updateMagicMastery = (
    field: 'mana' | 'abilityPower' | 'attunement' | 'faith' | 'lore',
    value: number | undefined
  ) => {
    setFormData({
      ...formData,
      stats: {
        ...stats,
        trainable: {
          ...stats.trainable,
          magic: {
            ...stats.trainable?.magic,
            [field]: value,
          },
        },
      },
    });
  };

  const updateBodyMastery = (
    field:
      | 'resistPhysical'
      | 'resistMagical'
      | 'healingEffectiveness'
      | 'dr'
      | 'armorTraining',
    value: number | undefined
  ) => {
    setFormData({
      ...formData,
      stats: {
        ...stats,
        trainable: {
          ...stats.trainable,
          body: {
            ...stats.trainable?.body,
            [field]: value,
          },
        },
      },
    });
  };

  const updateSkill = (
    field:
      | 'trickery'
      | 'brutishness'
      | 'stealth'
      | 'social'
      | 'magicItemUse'
      | 'cooking'
      | 'acrobatics'
      | 'survival'
      | 'focus'
      | 'conditioning',
    value: number | undefined
  ) => {
    setFormData({
      ...formData,
      stats: {
        ...stats,
        skills: {
          ...stats.skills,
          [field]: value,
        },
      },
    });
  };

  const updateCombat = (
    field: 'hp' | 'mp' | 'defaultAbility',
    value: number | string | undefined,
  ) => {
    const nextCombat = {
      ...formData.combat,
      [field]:
        field === 'defaultAbility'
          ? (typeof value === 'string' && value.trim() ? value.trim() : undefined)
          : value,
    };
    setFormData({
      ...formData,
      combat: nextCombat,
    });
  };

  const updateCombatDropTable = (index: number, name: string) => {
    const current = [...(formData.combat?.dropTables ?? [])];
    current[index] = name;
    setFormData({
      ...formData,
      combat: {
        ...formData.combat,
        dropTables: current,
      },
    });
  };

  const addCombatDropTable = () => {
    setFormData({
      ...formData,
      combat: {
        ...formData.combat,
        dropTables: [...(formData.combat?.dropTables ?? []), ''],
      },
    });
  };

  const removeCombatDropTable = (index: number) => {
    const next =
      formData.combat?.dropTables?.filter((_, i) => i !== index) ?? [];
    setFormData({
      ...formData,
      combat: {
        ...formData.combat,
        dropTables: next.length > 0 ? next : undefined,
      },
    });
  };

  const updateSound = (field: 'deathSoundName', value: string) => {
    setFormData({
      ...formData,
      sound: {
        ...formData.sound,
        [field]: value || undefined,
      },
    });
  };

  const addStatus = () => {
    setFormData({
      ...formData,
      statuses: [...(formData.statuses || []), { status: '' }],
    });
  };

  const removeStatus = (index: number) => {
    setFormData({
      ...formData,
      statuses: formData.statuses?.filter((_, i) => i !== index) || [],
    });
  };

  const updateStatus = (index: number, value: string) => {
    setFormData({
      ...formData,
      statuses:
        formData.statuses?.map((s, i) =>
          i === index ? { status: value } : s
        ) || [],
    });
  };

  const updateVision = (field: 'radius', value: number | undefined) => {
    setFormData({
      ...formData,
      vision: {
        ...formData.vision,
        [field]: value,
      },
    });
  };

  const updateCombatSpellAi = (next: CombatSpellAi | undefined) => {
    setFormData({
      ...formData,
      combatSpellAi: next,
    });
  };

  const knownSpells = formData?.combatSpellAi?.spells ?? [];
  const spellRules = formData?.combatSpellAi?.rules ?? [];
  const turnModes = formData?.combatSpellAi?.turnModes ?? [];
  const usesTurnModes = turnModes.length > 0;
  const isSpellcasterAi =
    formData?.combatBehavior?.town === 'SPELLCASTER' ||
    formData?.combatBehavior?.combat === 'SPELLCASTER' ||
    turnModes.some((mode) => mode.behavior === 'SPELLCASTER');

  const setKnownSpell = (index: number, name: string) => {
    const nextSpells = [...knownSpells];
    nextSpells[index] = name;
    updateCombatSpellAi({
      ...formData.combatSpellAi,
      spells: nextSpells,
    });
  };

  const addKnownSpell = () => {
    updateCombatSpellAi({
      ...formData.combatSpellAi,
      spells: [...knownSpells, ''],
    });
  };

  const removeKnownSpell = (index: number) => {
    const nextSpells = knownSpells.filter((_, i) => i !== index);
    updateCombatSpellAi({
      ...formData.combatSpellAi,
      spells: nextSpells,
    });
  };

  const setTurnMode = (index: number, mode: CombatSpellAiTurnMode) => {
    const nextModes = [...turnModes];
    nextModes[index] = mode;
    updateCombatSpellAi({
      ...formData.combatSpellAi,
      turnModes: nextModes,
    });
  };

  const addTurnMode = () => {
    const seedBehavior =
      turnModes.length === 0
        ? formData.combatBehavior?.combat || DEFAULT_COMBAT_BEHAVIOR
        : 'SPELLCASTER';
    updateCombatSpellAi({
      ...formData.combatSpellAi,
      turnModes: [...turnModes, { behavior: seedBehavior, weight: 1 }],
    });
  };

  const removeTurnMode = (index: number) => {
    updateCombatSpellAi({
      ...formData.combatSpellAi,
      turnModes: turnModes.filter((_, i) => i !== index),
    });
  };

  const setSpellRule = (index: number, rule: CombatSpellAiRule) => {
    const nextRules = [...spellRules];
    nextRules[index] = rule;
    updateCombatSpellAi({
      ...formData.combatSpellAi,
      rules: nextRules,
    });
  };

  const addSpellRule = () => {
    updateCombatSpellAi({
      ...formData.combatSpellAi,
      rules: [
        ...spellRules,
        {
          spell: '',
          conditions: [{ type: 'ALWAYS' }],
          zoneTargeting: formData.combatSpellAi?.zoneTargeting,
        },
      ],
    });
  };

  const removeSpellRule = (index: number) => {
    updateCombatSpellAi({
      ...formData.combatSpellAi,
      rules: spellRules.filter((_, i) => i !== index),
    });
  };

  const setSpellRuleCondition = (
    ruleIndex: number,
    conditionIndex: number,
    condition: CombatSpellAiCondition,
  ) => {
    const rule = spellRules[ruleIndex];
    const conditions = [...(rule.conditions ?? [])];
    conditions[conditionIndex] = condition;
    setSpellRule(ruleIndex, { ...rule, conditions });
  };

  const addSpellRuleCondition = (ruleIndex: number) => {
    const rule = spellRules[ruleIndex];
    setSpellRule(ruleIndex, {
      ...rule,
      conditions: [...(rule.conditions ?? []), { type: 'ALWAYS' }],
    });
  };

  const removeSpellRuleCondition = (ruleIndex: number, conditionIndex: number) => {
    const rule = spellRules[ruleIndex];
    setSpellRule(ruleIndex, {
      ...rule,
      conditions: (rule.conditions ?? []).filter((_, i) => i !== conditionIndex),
    });
  };

  const conditionNeedsValue = (type: CombatSpellAiConditionType) =>
    type === 'SELF_HP_BELOW_PERCENT' ||
    type === 'SELF_HP_ABOVE_PERCENT' ||
    type === 'ZONE_HOSTILE_COUNT_AT_LEAST' ||
    type === 'OWN_TURN_AT_MOST';

  const conditionNeedsStatus = (type: CombatSpellAiConditionType) =>
    type === 'HAS_STATUS' || type === 'LACKS_STATUS';

  const applyGenericTownspersonPreset = () => {
    setFormData({
      ...formData,
      stats: createGenericTownspersonStats(),
      sound: {
        ...formData.sound,
        deathSoundName: 'yell_body',
        deathSound: undefined,
      },
      vision: {
        ...formData.vision,
        radius: 6,
      },
      combat: {
        ...formData.combat,
        hp: 50,
      },
    });
  };

  if (!character) {
    return <EditorEmptyState message="Select a character to edit" />;
  }

  return (
    <div className="item-form character-template-form">
      <h2>Edit Character</h2>
      <form>
        <div className="form-fields-inline">
          <OptionSelect
            id="character-type"
            name="type"
            label="Type"
            value={formData.type}
            onChange={(value) => updateField('type', value as CharacterTemplate['type'])}
            options={CHARACTER_TYPES.map((type) => ({
              value: type,
              label: type,
            }))}
            required
          />

          <TextInput
            id="character-name"
            name="name"
            label="Name"
            value={formData.name}
            onChange={(value) => updateField('name', value)}
            placeholder="e.g., exampleTownsperson"
            required
          />

          <TextInput
            id="character-label"
            name="label"
            label="Label"
            value={formData.label}
            onChange={(value) => updateField('label', value)}
            placeholder="e.g., Example Townsperson"
            required
          />
        </div>

        <div className="character-sprite-row">
          <div className="form-group form-block">
            <label htmlFor="character-sprite-picker">Sprite</label>
            <div style={{ marginTop: '4px' }}>
              <SpritePicker
                value={spriteName}
                onChange={handleSpriteChange}
                scale={2}
              />
            </div>
            <div style={{ marginTop: '4px', fontSize: '11px', color: '#858585' }}>
              Spritesheet: {formData.spritesheet || 'Not set'} | Offset:{' '}
              {formData.spriteOffset ?? 'Not set'}
            </div>
          </div>
          <OptionSelect
            id="character-sprite-layout"
            name="spriteLayout"
            label="Sprite Layout"
            className="character-sprite-layout"
            value={formData.spriteLayout ?? 'default'}
            onChange={(value) => {
              const layout = value as CharacterSpriteLayout;
              if (layout === 'default') {
                const { spriteLayout: _layout, ...rest } = formData;
                setFormData(rest);
                return;
              }
              updateField('spriteLayout', layout);
            }}
            options={CHARACTER_SPRITE_LAYOUTS.map((layout) => ({
              value: layout,
              label: layout === 'weaponCombos' ? 'weaponCombos (equipped weapons)' : layout,
            }))}
          />
        </div>

        <div className="form-section">
          <h3>Presets</h3>
          <Button
            type="button"
            variant="secondary"
            onClick={applyGenericTownspersonPreset}
          >
            Generic Townsperson
          </Button>
        </div>

        <div className="form-section">
          <h3>Stats</h3>
          <div className="form-fields-inline" style={{ marginBottom: '16px' }}>
            <NumberInput
              id="combat-hp"
              name="combatHp"
              label="HP"
              value={formData.combat?.hp || 0}
              onChange={(value) => updateCombat('hp', value)}
              min={0}
              tinge="green"
            />
            <NumberInput
              id="combat-mp"
              name="combatMp"
              label="MP"
              value={formData.combat?.mp || 0}
              onChange={(value) => updateCombat('mp', value)}
              min={0}
              tinge="blue"
            />
            <NumberInput
              id="vision-radius"
              name="visionRadius"
              label="Vision"
              value={formData.vision?.radius || 0}
              onChange={(value) => updateVision('radius', value)}
              min={0}
            />
          </div>

          <div className="form-subsection">
            <h4>Generic Combat</h4>
            <div className="form-fields-inline">
              <NumberInput
                id="stat-str"
                name="statStr"
                label="STR"
                value={stats.generic?.str || 0}
                onChange={(value) => updateGenericStat('str', value)}
                min={0}
                tinge="indigo"
              />
              <NumberInput
                id="stat-mnd"
                name="statMnd"
                label="MND"
                value={stats.generic?.mnd || 0}
                onChange={(value) => updateGenericStat('mnd', value)}
                min={0}
                tinge="indigo"
              />
              <NumberInput
                id="stat-con"
                name="statCon"
                label="CON"
                value={stats.generic?.con || 0}
                onChange={(value) => updateGenericStat('con', value)}
                min={0}
                tinge="indigo"
              />
              <NumberInput
                id="stat-agi"
                name="statAgi"
                label="AGI"
                value={stats.generic?.agi || 0}
                onChange={(value) => updateGenericStat('agi', value)}
                min={0}
                tinge="indigo"
              />
              <NumberInput
                id="stat-lck"
                name="statLck"
                label="LCK"
                value={stats.generic?.lck || 0}
                onChange={(value) => updateGenericStat('lck', value)}
                min={0}
                tinge="indigo"
              />
            </div>
          </div>

          <div className="form-subsection">
            <h4>Weapon Mastery</h4>
            <div className="form-fields-inline">
              <NumberInput
                id="weapon-edged"
                name="weaponEdged"
                label="Edged"
                value={stats.trainable?.weapon?.edged || 0}
                onChange={(value) => updateWeaponMastery('edged', value)}
                min={0}
              />
              <NumberInput
                id="weapon-pole"
                name="weaponPole"
                label="Pole"
                value={stats.trainable?.weapon?.pole || 0}
                onChange={(value) => updateWeaponMastery('pole', value)}
                min={0}
              />
              <NumberInput
                id="weapon-blunt"
                name="weaponBlunt"
                label="Blunt"
                value={stats.trainable?.weapon?.blunt || 0}
                onChange={(value) => updateWeaponMastery('blunt', value)}
                min={0}
              />
              <NumberInput
                id="weapon-range"
                name="weaponRange"
                label="Range"
                value={stats.trainable?.weapon?.range || 0}
                onChange={(value) => updateWeaponMastery('range', value)}
                min={0}
              />
              <NumberInput
                id="weapon-unarmed"
                name="weaponUnarmed"
                label="Unarmed"
                value={stats.trainable?.weapon?.unarmed || 0}
                onChange={(value) => updateWeaponMastery('unarmed', value)}
                min={0}
              />
            </div>
          </div>

          <div className="form-subsection">
            <h4>Magic Mastery</h4>
            <div className="form-fields-inline">
              <NumberInput
                id="magic-mana"
                name="magicMana"
                label="Mana"
                value={stats.trainable?.magic?.mana || 0}
                onChange={(value) => updateMagicMastery('mana', value)}
                min={0}
              />
              <NumberInput
                id="magic-ability-power"
                name="magicAbilityPower"
                label="Ability Power"
                value={stats.trainable?.magic?.abilityPower || 0}
                onChange={(value) => updateMagicMastery('abilityPower', value)}
                min={0}
              />
              <NumberInput
                id="magic-attunement"
                name="magicAttunement"
                label="Attunement"
                value={stats.trainable?.magic?.attunement || 0}
                onChange={(value) => updateMagicMastery('attunement', value)}
                min={0}
              />
              <NumberInput
                id="magic-faith"
                name="magicFaith"
                label="Faith"
                value={stats.trainable?.magic?.faith || 0}
                onChange={(value) => updateMagicMastery('faith', value)}
                min={0}
              />
              <NumberInput
                id="magic-lore"
                name="magicLore"
                label="Lore"
                value={stats.trainable?.magic?.lore || 0}
                onChange={(value) => updateMagicMastery('lore', value)}
                min={0}
              />
            </div>
          </div>

          <div className="form-subsection">
            <h4>Body Mastery</h4>
            <div className="form-fields-inline">
              <NumberInput
                id="body-resist-physical"
                name="bodyResistPhysical"
                label="Resist Physical"
                value={stats.trainable?.body?.resistPhysical || 0}
                onChange={(value) => updateBodyMastery('resistPhysical', value)}
                min={0}
              />
              <NumberInput
                id="body-resist-magical"
                name="bodyResistMagical"
                label="Resist Magical"
                value={stats.trainable?.body?.resistMagical || 0}
                onChange={(value) => updateBodyMastery('resistMagical', value)}
                min={0}
              />
              <NumberInput
                id="body-healing-effectiveness"
                name="bodyHealingEffectiveness"
                label="Healing Effectiveness"
                value={stats.trainable?.body?.healingEffectiveness || 0}
                onChange={(value) =>
                  updateBodyMastery('healingEffectiveness', value)
                }
                min={0}
              />
              <NumberInput
                id="body-dr"
                name="bodyDr"
                label="DR"
                value={stats.trainable?.body?.dr || 0}
                onChange={(value) => updateBodyMastery('dr', value)}
                min={0}
              />
              <NumberInput
                id="body-armor-training"
                name="bodyArmorTraining"
                label="Armor Training"
                value={stats.trainable?.body?.armorTraining || 0}
                onChange={(value) => updateBodyMastery('armorTraining', value)}
                min={0}
              />
            </div>
          </div>

          <div className="form-subsection">
            <h4>Skills</h4>
            <div className="form-fields-inline">
              <NumberInput
                id="skill-trickery"
                name="skillTrickery"
                label="Trickery"
                value={stats.skills?.trickery || 0}
                onChange={(value) => updateSkill('trickery', value)}
                min={0}
                max={25}
              />
              <NumberInput
                id="skill-brutishness"
                name="skillBrutishness"
                label="Brutishness"
                value={stats.skills?.brutishness || 0}
                onChange={(value) => updateSkill('brutishness', value)}
                min={0}
                max={25}
              />
              <NumberInput
                id="skill-stealth"
                name="skillStealth"
                label="Stealth"
                value={stats.skills?.stealth || 0}
                onChange={(value) => updateSkill('stealth', value)}
                min={0}
                max={25}
              />
              <NumberInput
                id="skill-social"
                name="skillSocial"
                label="Social"
                value={stats.skills?.social || 0}
                onChange={(value) => updateSkill('social', value)}
                min={0}
                max={25}
              />
              <NumberInput
                id="skill-magic-item-use"
                name="skillMagicItemUse"
                label="Engineering"
                value={stats.skills?.magicItemUse || 0}
                onChange={(value) => updateSkill('magicItemUse', value)}
                min={0}
                max={25}
              />
              <NumberInput
                id="skill-cooking"
                name="skillCooking"
                label="Cooking"
                value={stats.skills?.cooking || 0}
                onChange={(value) => updateSkill('cooking', value)}
                min={0}
                max={25}
              />
              <NumberInput
                id="skill-acrobatics"
                name="skillAcrobatics"
                label="Acrobatics"
                value={stats.skills?.acrobatics || 0}
                onChange={(value) => updateSkill('acrobatics', value)}
                min={0}
                max={25}
              />
              <NumberInput
                id="skill-survival"
                name="skillSurvival"
                label="Survival"
                value={stats.skills?.survival || 0}
                onChange={(value) => updateSkill('survival', value)}
                min={0}
                max={25}
              />
              <NumberInput
                id="skill-focus"
                name="skillFocus"
                label="Focus"
                value={stats.skills?.focus || 0}
                onChange={(value) => updateSkill('focus', value)}
                min={0}
                max={25}
              />
              <NumberInput
                id="skill-conditioning"
                name="skillConditioning"
                label="Conditioning"
                value={stats.skills?.conditioning || 0}
                onChange={(value) => updateSkill('conditioning', value)}
                min={0}
                max={25}
              />
            </div>
          </div>
        </div>

        <div className="form-section">
          <h3>Optional Properties</h3>

          <div className="form-subsection">
            <h4>Talk</h4>
            <div className="form-fields-inline talk-fields-inline">
              <div className="talk-event-row">
                <SearchSelect
                  id="talk-name"
                  name="talkName"
                  label="Talk Event"
                  value={formData.talk?.talkName || ''}
                  onChange={(value) => updateTalk('talkName', value)}
                  items={talkEvents}
                  getItemKey={(event) => event.id}
                  getItemLabel={(event) => event.title?.trim() || event.id}
                  searchFields={(event) => [
                    event.id,
                    event.title ?? '',
                    event.eventType,
                  ]}
                  placeholder="Search talk events..."
                  emptyLabel="(no talk event)"
                  renderItem={(event) => (
                    <>
                      <div style={{ fontWeight: 600 }}>
                        {event.title?.trim() || event.id}
                      </div>
                      <div
                        style={{
                          fontSize: '10px',
                          color: '#858585',
                          marginTop: '2px',
                        }}
                      >
                        {event.id} • {event.eventType}
                      </div>
                    </>
                  )}
                />
                {formData.talk?.talkName ? (
                  <a
                    className="template-edit-link"
                    href={`${window.location.origin}${window.location.pathname}#/editor/specialEvents?event=${encodeURIComponent(formData.talk.talkName)}`}
                    target="_blank"
                    rel="noopener noreferrer"
                  >
                    Edit talk event
                  </a>
                ) : null}
              </div>
              <div className="form-group form-block talk-portrait-field">
                <label htmlFor="talk-portrait-picker">Portrait Sprite</label>
                <div style={{ margin: '8px', fontSize: '11px', color: '#858585' }}>
                  Shown on the talk event. If empty, the event's default
                  icon is used.
                </div>
                <div className="talk-portrait-picker">
                  <SpritePicker
                    value={formData.talk?.portraitName || ''}
                    onChange={(value) => updateTalk('portraitName', value)}
                    scale={2}
                    defaultSpritesheet="portraits0"
                  />
                </div>
                {formData.talk?.portraitName ? (
                  <div
                    style={{
                      marginTop: '4px',
                      fontSize: '11px',
                      color: '#858585',
                    }}
                  >
                    Selected: {formData.talk.portraitName}
                  </div>
                ) : null}
              </div>
            </div>
          </div>

          <div className="form-subsection">
            <h4>Map Behavior</h4>
            <div className="form-fields-inline">
              <OptionSelect
                id="behavior-name"
                name="behaviorName"
                label="Map Behavior"
                value={formData.behavior?.behaviorName || ''}
                onChange={updateBehavior}
                className="behavior-select-wide"
                options={[
                  { value: '', label: '(none)' },
                  ...CHARACTER_TEMPLATE_BEHAVIOR_NAMES.map((behavior) => ({
                    value: behavior,
                    label: behavior,
                  })),
                ]}
              />
            </div>
          </div>

          <div className="form-subsection">
            <h4>Combat Behavior</h4>
            <div className="form-fields-inline">
              <OptionSelect
                id="combat-behavior-town"
                name="combatBehaviorTown"
                label="Town"
                value={formData.combatBehavior?.town || DEFAULT_COMBAT_BEHAVIOR}
                onChange={(value) => updateCombatBehavior('town', value)}
                className="behavior-select-wide"
                options={COMBAT_BEHAVIOR_NAMES.map((behavior) => ({
                  value: behavior,
                  label: behavior,
                }))}
              />
              {!usesTurnModes ? (
                <OptionSelect
                  id="combat-behavior-combat"
                  name="combatBehaviorCombat"
                  label="Combat"
                  value={
                    formData.combatBehavior?.combat || DEFAULT_COMBAT_BEHAVIOR
                  }
                  onChange={(value) => updateCombatBehavior('combat', value)}
                  className="behavior-select-wide"
                  options={COMBAT_BEHAVIOR_NAMES.map((behavior) => ({
                    value: behavior,
                    label: behavior,
                  }))}
                />
              ) : null}
              {turnModes.map((mode, index) => (
                <div key={index} className="form-fields-inline">
                  <OptionSelect
                    id={`combat-spell-ai-turn-mode-${index}`}
                    name={`combatSpellAiTurnMode${index}`}
                    label={index === 0 ? 'Combat turn mode' : 'Turn mode'}
                    value={mode.behavior}
                    onChange={(value) =>
                      setTurnMode(index, {
                        ...mode,
                        behavior: value as CombatBehaviorName,
                      })
                    }
                    className="behavior-select-wide"
                    options={COMBAT_BEHAVIOR_NAMES.map((behavior) => ({
                      value: behavior,
                      label: behavior,
                    }))}
                  />
                  <NumberInput
                    id={`combat-spell-ai-turn-weight-${index}`}
                    name={`combatSpellAiTurnWeight${index}`}
                    label="Weight"
                    value={mode.weight ?? 1}
                    onChange={(value) =>
                      setTurnMode(index, { ...mode, weight: value })
                    }
                    min={0}
                  />
                  <Button
                    type="button"
                    variant="small"
                    className="btn-danger"
                    onClick={() => removeTurnMode(index)}
                  >
                    Remove
                  </Button>
                </div>
              ))}
              <Button type="button" variant="secondary" onClick={addTurnMode}>
                + Add turn mode
              </Button>
            </div>
          </div>

          {isSpellcasterAi ? (
          <div className="form-subsection">
            <h4>Spell AI</h4>
            <div className="form-fields-inline">
              <OptionSelect
                id="combat-spell-ai-selection"
                name="combatSpellAiSelection"
                label="Selection"
                value={formData.combatSpellAi?.selection || 'FIRST'}
                onChange={(value) =>
                  updateCombatSpellAi({
                    ...formData.combatSpellAi,
                    selection: value as CombatSpellAiSelection,
                  })
                }
                options={COMBAT_SPELL_AI_SELECTIONS.map((selection) => ({
                  value: selection,
                  label: selection === 'FIRST' ? 'First match' : 'Random',
                }))}
              />
              <OptionSelect
                id="combat-spell-ai-zone-targeting"
                name="combatSpellAiZoneTargeting"
                label="Zone targeting"
                value={formData.combatSpellAi?.zoneTargeting || 'DEFAULT'}
                onChange={(value) =>
                  updateCombatSpellAi({
                    ...formData.combatSpellAi,
                    zoneTargeting: value as CombatSpellAiZoneTargeting,
                  })
                }
                options={COMBAT_SPELL_AI_ZONE_TARGETINGS.map((targeting) => ({
                  value: targeting,
                  label:
                    targeting === 'RANDOM_IN_RANGE'
                      ? 'Random character in range'
                      : targeting === 'ONLY_PC'
                        ? 'Only PC'
                        : 'Default',
                }))}
              />
              <NumberInput
                id="combat-spell-ai-skip"
                name="combatSpellAiSkip"
                label="Skip chance %"
                value={formData.combatSpellAi?.skipChance || 0}
                onChange={(value) =>
                  updateCombatSpellAi({
                    ...formData.combatSpellAi,
                    skipChance: value,
                  })
                }
                min={0}
                max={100}
              />
            </div>
            <h4 style={{ marginTop: '12px' }}>Known Spells</h4>
            {knownSpells.map((spellName, index) => (
              <div key={index} className="status-item combat-drop-table-item">
                <SearchSelect
                  id={`combat-spell-ai-spell-${index}`}
                  name={`combatSpellAiSpell${index}`}
                  label={`Spell ${index + 1}`}
                  value={spellName}
                  onChange={(value) => setKnownSpell(index, value)}
                  items={spells}
                  getItemKey={(spell) => spell.name}
                  getItemLabel={(spell) => spell.label?.trim() || spell.name}
                  searchFields={(spell) => [spell.name, spell.label ?? '']}
                  placeholder="Search spells..."
                  emptyLabel="(select spell)"
                  allowEmpty
                />
                {spellName ? (
                  <a
                    className="template-edit-link"
                    href={`${window.location.origin}${window.location.pathname}#/editor/spellTemplates?spell=${encodeURIComponent(spellName)}`}
                    target="_blank"
                    rel="noopener noreferrer"
                  >
                    Edit spell
                  </a>
                ) : null}
                <Button
                  type="button"
                  variant="small"
                  className="btn-danger"
                  onClick={() => removeKnownSpell(index)}
                >
                  Remove
                </Button>
              </div>
            ))}
            <div style={{ marginTop: '6px' }}>
              <Button type="button" variant="secondary" onClick={addKnownSpell}>
                + Add Known Spell
              </Button>
            </div>
            <h4 style={{ marginTop: '12px' }}>Rules</h4>
            {spellRules.map((rule, ruleIndex) => (
              <div key={ruleIndex} className="form-subsection" style={{ marginTop: '8px' }}>
                <div className="form-fields-inline">
                  <SearchSelect
                    id={`combat-spell-ai-rule-spell-${ruleIndex}`}
                    name={`combatSpellAiRuleSpell${ruleIndex}`}
                    label="Cast"
                    value={rule.spell}
                    onChange={(value) => setSpellRule(ruleIndex, { ...rule, spell: value })}
                    items={spells.filter(
                      (spell) =>
                        knownSpells.includes(spell.name) || spell.name === rule.spell,
                    )}
                    getItemKey={(spell) => spell.name}
                    getItemLabel={(spell) => spell.label?.trim() || spell.name}
                    searchFields={(spell) => [spell.name, spell.label ?? '']}
                    placeholder="Search known spells..."
                    emptyLabel="(select spell)"
                    allowEmpty
                  />
                  <NumberInput
                    id={`combat-spell-ai-rule-weight-${ruleIndex}`}
                    name={`combatSpellAiRuleWeight${ruleIndex}`}
                    label="Weight"
                    value={rule.weight ?? 1}
                    onChange={(value) =>
                      setSpellRule(ruleIndex, { ...rule, weight: value })
                    }
                    min={1}
                  />
                  <OptionSelect
                    id={`combat-spell-ai-rule-zone-${ruleIndex}`}
                    name={`combatSpellAiRuleZone${ruleIndex}`}
                    label="Zone targeting"
                    value={rule.zoneTargeting || 'DEFAULT'}
                    onChange={(value) =>
                      setSpellRule(ruleIndex, {
                        ...rule,
                        zoneTargeting: value as CombatSpellAiZoneTargeting,
                      })
                    }
                    options={COMBAT_SPELL_AI_ZONE_TARGETINGS.map((targeting) => ({
                      value: targeting,
                      label:
                        targeting === 'RANDOM_IN_RANGE'
                          ? 'Random character in range'
                          : targeting === 'ONLY_PC'
                            ? 'Only PC'
                            : 'Default',
                    }))}
                  />
                  <Button
                    type="button"
                    variant="small"
                    className="btn-danger"
                    onClick={() => removeSpellRule(ruleIndex)}
                  >
                    Remove rule
                  </Button>
                </div>
                {(rule.conditions ?? []).map((condition, conditionIndex) => (
                  <div
                    key={conditionIndex}
                    className="form-fields-inline"
                    style={{ marginTop: '8px' }}
                  >
                    <OptionSelect
                      id={`combat-spell-ai-cond-type-${ruleIndex}-${conditionIndex}`}
                      name={`combatSpellAiCondType${ruleIndex}_${conditionIndex}`}
                      label="When"
                      value={condition.type}
                      onChange={(value) =>
                        setSpellRuleCondition(ruleIndex, conditionIndex, {
                          type: value as CombatSpellAiConditionType,
                          value: conditionNeedsValue(value as CombatSpellAiConditionType)
                            ? condition.value ?? 0
                            : undefined,
                          status: conditionNeedsStatus(value as CombatSpellAiConditionType)
                            ? condition.status
                            : undefined,
                        })
                      }
                      options={COMBAT_SPELL_AI_CONDITION_TYPES.map((type) => ({
                        value: type,
                        label: type,
                      }))}
                    />
                    {conditionNeedsValue(condition.type) ? (
                      <NumberInput
                        id={`combat-spell-ai-cond-value-${ruleIndex}-${conditionIndex}`}
                        name={`combatSpellAiCondValue${ruleIndex}_${conditionIndex}`}
                        label="Value"
                        value={condition.value ?? 0}
                        onChange={(value) =>
                          setSpellRuleCondition(ruleIndex, conditionIndex, {
                            ...condition,
                            value,
                          })
                        }
                        min={0}
                      />
                    ) : null}
                    {conditionNeedsStatus(condition.type) ? (
                      <TextInput
                        id={`combat-spell-ai-cond-status-${ruleIndex}-${conditionIndex}`}
                        name={`combatSpellAiCondStatus${ruleIndex}_${conditionIndex}`}
                        label="Status"
                        value={condition.status ?? ''}
                        onChange={(value) =>
                          setSpellRuleCondition(ruleIndex, conditionIndex, {
                            ...condition,
                            status: value,
                          })
                        }
                        placeholder="e.g. BLESSED"
                      />
                    ) : null}
                    <Button
                      type="button"
                      variant="small"
                      className="btn-danger"
                      onClick={() =>
                        removeSpellRuleCondition(ruleIndex, conditionIndex)
                      }
                    >
                      Remove
                    </Button>
                  </div>
                ))}
                <div style={{ marginTop: '6px' }}>
                  <Button
                    type="button"
                    variant="secondary"
                    onClick={() => addSpellRuleCondition(ruleIndex)}
                  >
                    + Add Condition
                  </Button>
                </div>
              </div>
            ))}
            <div style={{ marginTop: '6px' }}>
              <Button type="button" variant="secondary" onClick={addSpellRule}>
                + Add Rule
              </Button>
            </div>
          </div>
          ) : null}

          <div className="form-subsection">
            <div className="form-fields-inline combat-default-ability-row">
              <SearchSelect
                id="combat-default-ability"
                name="combatDefaultAbility"
                className="combat-default-ability-search"
                label="Default Ability"
                value={formData.combat?.defaultAbility || ''}
                onChange={(value) => updateCombat('defaultAbility', value)}
                items={abilities}
                getItemKey={(a) => a.name}
                getItemLabel={(a) => a.label?.trim() || a.name}
                searchFields={(a) => [a.name, a.label ?? '', a.type ?? '']}
                placeholder="Search abilities..."
                emptyLabel="(punch / MELEE_ATTACK_DEFAULT)"
                allowEmpty
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
              {formData.combat?.defaultAbility ? (
                <a
                  className="template-edit-link"
                  href={`${window.location.origin}${window.location.pathname}#/editor/abilityTemplates?ability=${encodeURIComponent(formData.combat.defaultAbility)}`}
                  target="_blank"
                  rel="noopener noreferrer"
                >
                  Edit ability
                </a>
              ) : null}
            </div>
          </div>

          <div className="form-subsection">
            <h4>Drop Tables</h4>
              {formData.combat?.dropTables?.map((tableName, index) => (
                <div key={index} className="status-item combat-drop-table-item">
                  <SearchSelect
                    id={`combat-drop-table-${index}`}
                    name={`combatDropTable${index}`}
                    className="combat-drop-table-search"
                    label={`Drop table ${index + 1}`}
                    value={tableName}
                    onChange={(value) => updateCombatDropTable(index, value)}
                    items={dropTables}
                    getItemKey={(table) => table.name}
                    getItemLabel={(table) => table.label?.trim() || table.name}
                    searchFields={(table) => [table.name, table.label ?? '']}
                    placeholder="Search drop tables..."
                    emptyLabel="(select drop table)"
                    allowEmpty
                  />
                  <Button
                    type="button"
                    variant="small"
                    className="btn-danger"
                    onClick={() => removeCombatDropTable(index)}
                  >
                    Remove
                  </Button>
                </div>
              ))}
              <div style={{ marginTop: '6px' }}>
                <Button
                  type="button"
                  variant="secondary"
                  onClick={addCombatDropTable}
                >
                  + Add Drop Table
                </Button>
              </div>
          </div>

          <div className="form-subsection">
            <h4>Sound</h4>
            <div className="form-fields-inline character-sound-fields">
              <SoundSearchField
                id="sound-death"
                name="soundDeath"
                label="Death Sound"
                value={
                  formData.sound?.deathSoundName ||
                  formData.sound?.deathSound ||
                  ''
                }
                onChange={(value) => updateSound('deathSoundName', value)}
              />
            </div>
          </div>

          <div className="form-subsection">
            <h4>Statuses</h4>
            <div id="statuses-list">
              {formData.statuses?.map((status, index) => (
                <div key={index} className="status-item">
                  <TextInput
                    label="Status"
                    value={status.status}
                    onChange={(value) => updateStatus(index, value)}
                    placeholder="Status name"
                  />
                  <Button
                    type="button"
                    variant="small"
                    className="btn-danger"
                    onClick={() => removeStatus(index)}
                  >
                    Remove
                  </Button>
                </div>
              ))}
            </div>
            <div style={{ marginTop: '6px' }}>
              <Button type="button" variant="secondary" onClick={addStatus}>
                + Add Status
              </Button>
            </div>
          </div>
        </div>
      </form>
    </div>
  );
}
