#pragma once

#include "model/stats/CharacterStats.h"
#include <string>
#include <vector>

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
  std::string talkName;
  std::string portraitName;
};

struct CharacterTemplateBehavior {
  std::string behaviorName;
};

struct CharacterTemplateCombat {
  int hp = 0;
  int mp = 0;
  std::string dropTable;
};

struct CharacterTemplateSound {
  std::string deathSoundName;
  std::string weaponSoundName;
};

struct CharacterTemplateStatus {
  std::string status;
};

struct CharacterTemplateVision {
  int radius;
};

struct CharacterTemplate {
  CharacterTemplateType type;
  std::string name;
  std::string label;
  std::string spritesheetName;
  std::string spriteOffset;
  CharacterTemplateTalk talk;
  CharacterTemplateBehavior behavior;
  CharacterStats stats;
  CharacterTemplateCombat combat;
  CharacterTemplateSound sound;
  std::vector<CharacterTemplateStatus> statuses;
  CharacterTemplateVision vision;
};

std::string characterGetSprite(const CharacterTemplate& character);

} // namespace model
