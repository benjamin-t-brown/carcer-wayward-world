import { Button } from '../../elements/Button';
import { NumberInput } from '../../elements/NumberInput';
import { OptionSelect } from '../../elements/OptionSelect';
import { SearchSelect } from '../../elements/SearchSelect';
import { SoundSearchField } from '../../elements/SoundSearchField';
import { SpritePicker } from '../../elements/SpritePicker';
import { TextInput } from '../../elements/TextInput';
import {
  CHARACTER_TEMPLATE_BEHAVIOR_NAMES,
  COMBAT_BEHAVIOR_NAMES,
  CharacterTemplate,
  CombatBehaviorName,
  GameEvent,
  createDefaultCharacterStats,
} from '../../types/assets';

const CHARACTER_TYPES = [
  'TOWNSPERSON',
  'TOWNSPERSON_STATIC',
  'ENEMY',
  'ENEMY_STATIC',
];

const DEFAULT_COMBAT_BEHAVIOR: CombatBehaviorName = 'SEEK_AND_MELEE';

export type UpdateCharacterField = <K extends keyof CharacterTemplate>(
  field: K,
  value: CharacterTemplate[K],
) => void;

interface CharacterIdentitySectionProps {
  character: CharacterTemplate;
  spriteName: string;
  updateField: UpdateCharacterField;
  onSpriteChange: (spriteName: string) => void;
}

export function CharacterIdentitySection({
  character,
  spriteName,
  updateField,
  onSpriteChange,
}: CharacterIdentitySectionProps) {
  return (
    <>
      <div className="form-fields-inline">
        <OptionSelect
          id="character-type"
          name="type"
          label="Type"
          value={character.type}
          onChange={(value) =>
            updateField('type', value as CharacterTemplate['type'])
          }
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
          value={character.name}
          onChange={(value) => updateField('name', value)}
          placeholder="e.g., exampleTownsperson"
          required
        />
        <TextInput
          id="character-label"
          name="label"
          label="Label"
          value={character.label}
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
            onChange={onSpriteChange}
            scale={2}
          />
        </div>
        <div style={{ marginTop: '4px', fontSize: '11px', color: '#858585' }}>
          Spritesheet: {character.spritesheet || 'Not set'} | Offset:{' '}
          {character.spriteOffset ?? 'Not set'}
        </div>
      </div>
    </>
  );
}

type CharacterStats = ReturnType<typeof createDefaultCharacterStats>;

interface CharacterStatsSectionProps {
  stats: CharacterStats;
  updateGenericStat: (
    field: 'str' | 'mnd' | 'con' | 'agi' | 'lck',
    value: number | undefined,
  ) => void;
  updateWeaponMastery: (
    field: 'edged' | 'pole' | 'blunt' | 'range' | 'unarmed',
    value: number | undefined,
  ) => void;
  updateMagicMastery: (
    field: 'mana' | 'abilityPower' | 'attunement' | 'faith' | 'lore',
    value: number | undefined,
  ) => void;
  updateBodyMastery: (
    field:
      | 'resistPhysical'
      | 'resistMagical'
      | 'healingEffectiveness'
      | 'dr'
      | 'armorTraining',
    value: number | undefined,
  ) => void;
  updateSkill: (
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
    value: number | undefined,
  ) => void;
}

export function CharacterStatsSection({
  stats,
  updateGenericStat,
  updateWeaponMastery,
  updateMagicMastery,
  updateBodyMastery,
  updateSkill,
}: CharacterStatsSectionProps) {
  return (
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
  );
}

interface CharacterOptionalPropertiesSectionProps {
  character: CharacterTemplate;
  talkEvents: GameEvent[];
  updateTalk: (field: 'talkName' | 'portraitName', value: string) => void;
  updateBehavior: (value: string) => void;
  updateCombatBehavior: (field: 'town' | 'combat', value: string) => void;
  updateCombat: (
    field: 'hp' | 'mp' | 'dropTable',
    value: number | string | undefined,
  ) => void;
  updateSound: (
    field: 'deathSoundName' | 'weaponSoundName',
    value: string,
  ) => void;
  updateVision: (field: 'radius', value: number | undefined) => void;
  updateStatus: (index: number, value: string) => void;
  addStatus: () => void;
  removeStatus: (index: number) => void;
}

export function CharacterOptionalPropertiesSection({
  character,
  talkEvents,
  updateTalk,
  updateBehavior,
  updateCombatBehavior,
  updateCombat,
  updateSound,
  updateVision,
  updateStatus,
  addStatus,
  removeStatus,
}: CharacterOptionalPropertiesSectionProps) {
  return (
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
              value={character.talk?.talkName || ''}
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
            {character.talk?.talkName ? (
              <a
                className="template-edit-link"
                href={`${window.location.origin}${window.location.pathname}#/editor/specialEvents?event=${encodeURIComponent(character.talk.talkName)}`}
                target="_blank"
                rel="noopener noreferrer"
              >
                Edit talk event
              </a>
            ) : null}
          </div>
          <div className="form-group form-block talk-portrait-field">
            <label htmlFor="talk-portrait-picker">Portrait Sprite</label>
            <div className="talk-portrait-picker">
              <SpritePicker
                value={character.talk?.portraitName || ''}
                onChange={(value) => updateTalk('portraitName', value)}
                scale={2}
                defaultSpritesheet="portraits0"
              />
            </div>
            {character.talk?.portraitName ? (
              <div
                style={{
                  marginTop: '4px',
                  fontSize: '11px',
                  color: '#858585',
                }}
              >
                Selected: {character.talk.portraitName}
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
            value={character.behavior?.behaviorName || ''}
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
            value={character.combatBehavior?.town || DEFAULT_COMBAT_BEHAVIOR}
            onChange={(value) => updateCombatBehavior('town', value)}
            className="behavior-select-wide"
            options={COMBAT_BEHAVIOR_NAMES.map((behavior) => ({
              value: behavior,
              label: behavior,
            }))}
          />
          <OptionSelect
            id="combat-behavior-combat"
            name="combatBehaviorCombat"
            label="Combat"
            value={character.combatBehavior?.combat || DEFAULT_COMBAT_BEHAVIOR}
            onChange={(value) => updateCombatBehavior('combat', value)}
            className="behavior-select-wide"
            options={COMBAT_BEHAVIOR_NAMES.map((behavior) => ({
              value: behavior,
              label: behavior,
            }))}
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
            value={character.combat?.hp || 0}
            onChange={(value) => updateCombat('hp', value)}
            min={0}
          />
          <NumberInput
            id="combat-mp"
            name="combatMp"
            label="MP"
            value={character.combat?.mp || 0}
            onChange={(value) => updateCombat('mp', value)}
            min={0}
          />
          <TextInput
            id="combat-drop-table"
            name="combatDropTable"
            label="Drop Table"
            value={character.combat?.dropTable || ''}
            onChange={(value) => updateCombat('dropTable', value)}
            placeholder="e.g., dropTableId"
          />
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
              character.sound?.deathSoundName ||
              character.sound?.deathSound ||
              ''
            }
            onChange={(value) => updateSound('deathSoundName', value)}
          />
          <SoundSearchField
            id="sound-weapon"
            name="soundWeapon"
            label="Weapon Sound"
            value={
              character.sound?.weaponSoundName ||
              character.sound?.weaponSound ||
              ''
            }
            onChange={(value) => updateSound('weaponSoundName', value)}
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
            value={character.vision?.radius || 0}
            onChange={(value) => updateVision('radius', value)}
            min={0}
          />
        </div>
      </div>

      <div className="form-subsection">
        <h4>Statuses</h4>
        <div id="statuses-list">
          {character.statuses?.map((status, index) => (
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
  );
}
