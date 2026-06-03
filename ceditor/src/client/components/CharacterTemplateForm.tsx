import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { OptionSelect } from '../elements/OptionSelect';
import { Button } from '../elements/Button';
import { SpritePicker } from '../elements/SpritePicker';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import { EditorEmptyState } from './EditorEmptyState';
import {
  CharacterTemplate,
  createDefaultCharacterStats,
} from '../types/assets';

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
  const { spriteMap } = useSDL2WAssets();

  const formData = character as CharacterTemplate;
  const setFormData = (data: CharacterTemplate) => {
    props.updateCharacter(data);
  };

  const stats = formData?.stats ?? createDefaultCharacterStats();

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

  const updateBehavior = (field: 'behaviorName', value: string) => {
    setFormData({
      ...formData,
      behavior: {
        ...formData.behavior,
        [field]: value || undefined,
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
    field: 'hp' | 'mp' | 'dropTable',
    value: number | string | undefined
  ) => {
    setFormData({
      ...formData,
      combat: {
        ...formData.combat,
        [field]: value,
      },
    });
  };

  const updateSound = (
    field: 'deathSoundName' | 'weaponSoundName',
    value: string
  ) => {
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
            onChange={(value) => updateField('type', value)}
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

        <div className="form-section">
          <h3>Stats</h3>

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
              />
              <NumberInput
                id="stat-mnd"
                name="statMnd"
                label="MND"
                value={stats.generic?.mnd || 0}
                onChange={(value) => updateGenericStat('mnd', value)}
                min={0}
              />
              <NumberInput
                id="stat-con"
                name="statCon"
                label="CON"
                value={stats.generic?.con || 0}
                onChange={(value) => updateGenericStat('con', value)}
                min={0}
              />
              <NumberInput
                id="stat-agi"
                name="statAgi"
                label="AGI"
                value={stats.generic?.agi || 0}
                onChange={(value) => updateGenericStat('agi', value)}
                min={0}
              />
              <NumberInput
                id="stat-lck"
                name="statLck"
                label="LCK"
                value={stats.generic?.lck || 0}
                onChange={(value) => updateGenericStat('lck', value)}
                min={0}
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
                label="Magic Item Use"
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
            <div className="form-fields-inline">
              <TextInput
                id="talk-name"
                name="talkName"
                label="Talk Name"
                value={formData.talk?.talkName || ''}
                onChange={(value) => updateTalk('talkName', value)}
                placeholder="e.g., talkId"
              />
              <TextInput
                id="talk-portrait"
                name="portraitName"
                label="Portrait Name"
                value={formData.talk?.portraitName || ''}
                onChange={(value) => updateTalk('portraitName', value)}
                placeholder="e.g., portraitSprite"
              />
            </div>
          </div>

          <div className="form-subsection">
            <h4>Behavior</h4>
            <div className="form-fields-inline">
              <TextInput
                id="behavior-name"
                name="behaviorName"
                label="Behavior Name"
                value={formData.behavior?.behaviorName || ''}
                onChange={(value) => updateBehavior('behaviorName', value)}
                placeholder="e.g., behaviorId"
              />
            </div>
          </div>

          <div className="form-subsection">
            <h4>Combat Resources</h4>
            <div className="form-fields-inline">
              <NumberInput
                id="combat-hp"
                name="combatHp"
                label="HP"
                value={formData.combat?.hp || 0}
                onChange={(value) => updateCombat('hp', value)}
                min={0}
              />
              <NumberInput
                id="combat-mp"
                name="combatMp"
                label="MP"
                value={formData.combat?.mp || 0}
                onChange={(value) => updateCombat('mp', value)}
                min={0}
              />
              <TextInput
                id="combat-drop-table"
                name="combatDropTable"
                label="Drop Table"
                value={formData.combat?.dropTable || ''}
                onChange={(value) => updateCombat('dropTable', value)}
                placeholder="e.g., dropTableId"
              />
            </div>
          </div>

          <div className="form-subsection">
            <h4>Sound</h4>
            <div className="form-fields-inline">
              <TextInput
                id="sound-death"
                name="soundDeath"
                label="Death Sound"
                value={formData.sound?.deathSoundName || ''}
                onChange={(value) => updateSound('deathSoundName', value)}
                placeholder="e.g., soundName"
              />
              <TextInput
                id="sound-weapon"
                name="soundWeapon"
                label="Weapon Sound"
                value={formData.sound?.weaponSoundName || ''}
                onChange={(value) => updateSound('weaponSoundName', value)}
                placeholder="e.g., soundName"
              />
            </div>
          </div>

          <div className="form-subsection">
            <h4>Vision</h4>
            <div className="form-fields-inline">
              <NumberInput
                id="vision-radius"
                name="visionRadius"
                label="Radius"
                value={formData.vision?.radius || 0}
                onChange={(value) => updateVision('radius', value)}
                min={0}
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
