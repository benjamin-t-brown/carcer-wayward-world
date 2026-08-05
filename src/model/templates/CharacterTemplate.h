#pragma once

#include "model/stats/CharacterStats.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace db {
class Database;
}

namespace model {

struct CharacterInstance;

enum class CharacterTemplateType {
  TOWNSPERSON,
  TOWNSPERSON_STATIC,
  ENEMY,
  ENEMY_STATIC,
};

enum class CharacterTemplateBehaviorName {
  MOVE_RANDOMLY,
  IMMOBILE,
  IMMOBILE_UNTIL_ENEMY_SPOTTED,
  SEEK_MARKER,
  MOVE_LEFT_RIGHT,
  MOVE_UP_DOWN,
};

enum class CombatBehaviorName {
  SEEK_AND_MELEE,
};

struct CharacterTemplateTalk {
  bmin::String talkName;
  bmin::String portraitName;
};

struct CharacterTemplateBehavior {
  bmin::String behaviorName;
};

struct CharacterTemplateCombat {
  int hp = 0;
  int mp = 0;
  bmin::String dropTable;
};

struct CharacterTemplateCombatBehavior {
  CombatBehaviorName town = CombatBehaviorName::SEEK_AND_MELEE;
  CombatBehaviorName combat = CombatBehaviorName::SEEK_AND_MELEE;
};

struct CharacterTemplateSound {
  bmin::String deathSoundName;
  bmin::String weaponSoundName;
};

struct CharacterTemplateStatus {
  bmin::String status;
};

struct CharacterTemplateVision {
  int radius = 0;
};

struct CharacterTemplate {
  CharacterTemplateType type;
  bmin::String name;
  bmin::String label;
  bmin::String spritesheetName;
  bmin::String spriteOffset;
  CharacterTemplateTalk talk;
  CharacterTemplateBehavior behavior;
  CharacterStats stats;
  CharacterTemplateCombat combat;
  CharacterTemplateCombatBehavior combatBehavior;
  CharacterTemplateSound sound;
  bmin::DynArray<CharacterTemplateStatus> statuses;
  CharacterTemplateVision vision;
};

bmin::String characterGetSprite(const CharacterTemplate& character);
bmin::String characterGetSpriteAtIndexOffset(const CharacterTemplate& characterTemplate,
                                             int indexOffset);

/** Copy AI/faction fields from template onto a map character instance. */
void applyCharacterTemplateToInstance(CharacterInstance& character,
                                      const CharacterTemplate& characterTemplate);

/** Lookup templateName on the database and apply; returns false if missing. */
bool tryApplyCharacterTemplateToInstance(CharacterInstance& character,
                                         const db::Database& database);

} // namespace model
