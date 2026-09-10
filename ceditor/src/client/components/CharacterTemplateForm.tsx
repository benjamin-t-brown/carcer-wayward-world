import { useMemo } from 'react';
import { useAssets } from '../contexts/AssetsContext';
import { useSDL2WAssets } from '../contexts/SDL2WAssetsContext';
import {
  CharacterTemplate,
  CharacterTemplateBehaviorName,
  CombatBehaviorName,
  createDefaultCharacterStats,
} from '../types/assets';
import {
  CharacterIdentitySection,
  CharacterOptionalPropertiesSection,
  CharacterStatsSection,
} from './character/CharacterTemplateSections';
import { EditorEmptyState } from './EditorEmptyState';
import './CharacterTemplateForm.css';

const DEFAULT_COMBAT_BEHAVIOR: CombatBehaviorName = 'SEEK_AND_MELEE';

export type { CharacterTemplate };

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
  const { gameEvents } = useAssets();
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
    value: CharacterTemplate[K],
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

  const updateCombatBehavior = (field: 'town' | 'combat', value: string) => {
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
    value: number | undefined,
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
    value: number | undefined,
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
    value: number | undefined,
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
    value: number | undefined,
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
    value: number | undefined,
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
    value: number | string | undefined,
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
    value: string,
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
        formData.statuses?.map((status, i) =>
          i === index ? { status: value } : status,
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
        <CharacterIdentitySection
          character={formData}
          spriteName={spriteName}
          updateField={updateField}
          onSpriteChange={handleSpriteChange}
        />
        <CharacterStatsSection
          stats={stats}
          updateGenericStat={updateGenericStat}
          updateWeaponMastery={updateWeaponMastery}
          updateMagicMastery={updateMagicMastery}
          updateBodyMastery={updateBodyMastery}
          updateSkill={updateSkill}
        />
        <CharacterOptionalPropertiesSection
          character={formData}
          talkEvents={talkEvents}
          updateTalk={updateTalk}
          updateBehavior={updateBehavior}
          updateCombatBehavior={updateCombatBehavior}
          updateCombat={updateCombat}
          updateSound={updateSound}
          updateVision={updateVision}
          updateStatus={updateStatus}
          addStatus={addStatus}
          removeStatus={removeStatus}
        />
      </form>
    </div>
  );
}
