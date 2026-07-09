#pragma once

#include "lib/Types.h"

namespace model {

struct GenericCombatStats {
  int str = 0;
  int mnd = 0;
  int con = 0;
  int agi = 0;
  int lck = 0;
};

struct WeaponMasteryStats {
  int edged = 0;
  int pole = 0;
  int blunt = 0;
  int range = 0;
  int unarmed = 0;
};

struct MagicMasteryStats {
  int mana = 0;
  int abilityPower = 0;
  int attunement = 0;
  int faith = 0;
  int lore = 0;
};

struct BodyMasteryStats {
  int resistPhysical = 0;
  int resistMagical = 0;
  int healingEffectiveness = 0;
  int dr = 0;
  int armorTraining = 0;
};

struct TrainableCombatStats {
  WeaponMasteryStats weapon;
  MagicMasteryStats magic;
  BodyMasteryStats body;
};

struct CharacterSkills {
  int trickery = 0;
  int stealth = 0;
  int social = 0;
  int magicItemUse = 0;
  int cooking = 0;
  int acrobatics = 0;
  int survival = 0;
  int focus = 0;
  int conditioning = 0;
};

struct CharacterStats {
  GenericCombatStats generic;
  TrainableCombatStats trainable;
  CharacterSkills skills;
};

struct CharacterTemplate;

void initCharacterStatsFromTemplate(CharacterStats& out, const CharacterTemplate& tmpl);

} // namespace model
