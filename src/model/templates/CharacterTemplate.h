#pragma once

#include "model/stats/CharacterStats.h"
#include "lib/Types.h"

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
  String talkName;
  String portraitName;
};

struct CharacterTemplateBehavior {
  String behaviorName;
};

struct CharacterTemplateCombat {
  int hp = 0;
  int mp = 0;
  String dropTable;
};

struct CharacterTemplateSound {
  String deathSoundName;
  String weaponSoundName;
};

struct CharacterTemplateStatus {
  String status;
};

struct CharacterTemplateVision {
  int radius;
};

struct CharacterTemplate {
  CharacterTemplateType type;
  String name;
  String label;
  String spritesheetName;
  String spriteOffset;
  CharacterTemplateTalk talk;
  CharacterTemplateBehavior behavior;
  CharacterStats stats;
  CharacterTemplateCombat combat;
  CharacterTemplateSound sound;
  bmin::DynArray<CharacterTemplateStatus> statuses;
  CharacterTemplateVision vision;
};

String characterGetSprite(const CharacterTemplate& character);

} // namespace model
