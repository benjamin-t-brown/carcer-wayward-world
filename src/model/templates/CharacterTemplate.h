#pragma once

#include <string>
#include <vector>

namespace model {

enum class CharacterTemplateType {
  TOWNSPERSON,
  TOWNSPERSON_STATIC,
  ENEMY,
  ENEMY_STATIC,
};

struct CharacterTemplateTalk {
  std::string talkName;
  std::string portraitName;
};

struct CharacterTemplateBehavior {
  std::string behaviorName;
};

struct CharacterTemplateCombatStats {
  int str = 0;
  int mnd = 0;
  int con = 0;
  int agi = 0;
  int lck = 0;
};

struct CharacterTemplateCombat {
  CharacterTemplateCombatStats stats;
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
  CharacterTemplateCombat combat;
  CharacterTemplateSound sound;
  std::vector<CharacterTemplateStatus> statuses;
  CharacterTemplateVision vision;
};

std::string characterGetSprite(const CharacterTemplate& character);

} // namespace model
