import { TextInput } from '../elements/TextInput';
import { NumberInput } from '../elements/NumberInput';
import { OptionSelect } from '../elements/OptionSelect';
import { Button } from '../elements/Button';

export interface CharacterTemplate {
  type: string;
  name: string;
  label: string;
  spritesheetName: string;
  spriteOffset: string;
  // Optional fields
  talk?: {
    talkName?: string;
    portraitName?: string;
  };
  behavior?: {
    behaviorName?: string;
  };
  combat?: {
    stats?: {
      str?: number;
      mnd?: number;
      con?: number;
      agi?: number;
      lck?: number;
    };
    hp?: number;
    mp?: number;
    dropTable?: string;
  };
  sound?: {
    deathSoundName?: string;
    weaponSoundName?: string;
  };
  statuses?: Array<{
    status: string;
  }>;
  vision?: {
    radius?: number;
  };
}

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
    spritesheetName: '',
    spriteOffset: '',
  };
}

export function CharacterTemplateForm(props: CharacterTemplateFormProps) {
  const character = props.character;

  const formData = character as CharacterTemplate;
  const setFormData = (data: CharacterTemplate) => {
    props.updateCharacter(data);
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

  const updateCombatStats = (
    field: 'str' | 'mnd' | 'con' | 'agi' | 'lck',
    value: number | undefined
  ) => {
    setFormData({
      ...formData,
      combat: {
        ...formData.combat,
        stats: {
          ...formData.combat?.stats,
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
    return <div>Select a character to edit</div>;
  }

  return (
    <div className="item-form">
      <h2>Edit Character</h2>
      <form>
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

        <TextInput
          id="character-spritesheet"
          name="spritesheetName"
          label="Spritesheet Name"
          value={formData.spritesheetName}
          onChange={(value) => updateField('spritesheetName', value)}
          placeholder="e.g., characters"
          required
        />

        <TextInput
          id="character-sprite-offset"
          name="spriteOffset"
          label="Sprite Offset"
          value={formData.spriteOffset}
          onChange={(value) => updateField('spriteOffset', value)}
          placeholder="e.g., 0,0"
          required
        />

        <div className="form-section">
          <h3>Optional Properties</h3>

          <div className="form-subsection">
            <h4>Talk</h4>
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

          <div className="form-subsection">
            <h4>Behavior</h4>
            <TextInput
              id="behavior-name"
              name="behaviorName"
              label="Behavior Name"
              value={formData.behavior?.behaviorName || ''}
              onChange={(value) => updateBehavior('behaviorName', value)}
              placeholder="e.g., behaviorId"
            />
          </div>

          <div className="form-subsection">
            <h4>Combat</h4>
            <div className="form-group">
              <label>Stats</label>
              <div className="form-row">
                <NumberInput
                  id="combat-str"
                  name="combatStr"
                  label="STR"
                  value={formData.combat?.stats?.str || 0}
                  onChange={(value) => updateCombatStats('str', value)}
                  min={0}
                />
                <NumberInput
                  id="combat-mnd"
                  name="combatMnd"
                  label="MND"
                  value={formData.combat?.stats?.mnd || 0}
                  onChange={(value) => updateCombatStats('mnd', value)}
                  min={0}
                />
                <NumberInput
                  id="combat-con"
                  name="combatCon"
                  label="CON"
                  value={formData.combat?.stats?.con || 0}
                  onChange={(value) => updateCombatStats('con', value)}
                  min={0}
                />
                <NumberInput
                  id="combat-agi"
                  name="combatAgi"
                  label="AGI"
                  value={formData.combat?.stats?.agi || 0}
                  onChange={(value) => updateCombatStats('agi', value)}
                  min={0}
                />
                <NumberInput
                  id="combat-lck"
                  name="combatLck"
                  label="LCK"
                  value={formData.combat?.stats?.lck || 0}
                  onChange={(value) => updateCombatStats('lck', value)}
                  min={0}
                />
              </div>
            </div>
            <div className="form-row">
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
            </div>
            <TextInput
              id="combat-drop-table"
              name="combatDropTable"
              label="Drop Table"
              value={formData.combat?.dropTable || ''}
              onChange={(value) => updateCombat('dropTable', value)}
              placeholder="e.g., dropTableId"
            />
          </div>

          <div className="form-subsection">
            <h4>Sound</h4>
            <TextInput
              id="sound-death"
              name="soundDeath"
              label="Death Sound Name"
              value={formData.sound?.deathSoundName || ''}
              onChange={(value) => updateSound('deathSoundName', value)}
              placeholder="e.g., soundName"
            />
            <TextInput
              id="sound-weapon"
              name="soundWeapon"
              label="Weapon Sound Name"
              value={formData.sound?.weaponSoundName || ''}
              onChange={(value) => updateSound('weaponSoundName', value)}
              placeholder="e.g., soundName"
            />
          </div>

          <div className="form-subsection">
            <h4>Vision</h4>
            <NumberInput
              id="vision-radius"
              name="visionRadius"
              label="Radius"
              value={formData.vision?.radius || 0}
              onChange={(value) => updateVision('radius', value)}
              min={0}
            />
          </div>

          <div className="form-group">
            <label>Statuses</label>
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
            <div style={{ marginTop: '10px' }}>
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
