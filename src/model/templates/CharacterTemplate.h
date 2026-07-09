#pragma once

#include "model/stats/CharacterStats.h"
#include "bmin/DynArray.h"
#include "bmin/String.h"

namespace model {

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

struct CharacterTemplateSound {
  bmin::String deathSoundName;
  bmin::String weaponSoundName;
};

struct CharacterTemplateStatus {
  bmin::String status;
};

struct CharacterTemplateVision {
  int radius;
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
  CharacterTemplateSound sound;
  bmin::DynArray<CharacterTemplateStatus> statuses;
  CharacterTemplateVision vision;
};

bmin::String characterGetSprite(const CharacterTemplate& character);

} // namespace model
