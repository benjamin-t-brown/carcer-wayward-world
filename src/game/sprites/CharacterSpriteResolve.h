#pragma once

#include "bmin/String.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Items.h"

namespace db {
class Database;
}

namespace model {
struct CharacterPlayer;
}

namespace game {

struct CharacterSpriteResolveArgs {
  const model::CharacterTemplate& characterTemplate;
  model::WeaponVisual mainHand = model::WeaponVisual::None;
  model::WeaponVisual offHand = model::WeaponVisual::None;
  bool inCombat = false;
  /** 0 = idle, 1 = main/single attack, 2 = off-hand attack (dual blocks). */
  int pose = 0;
};

/** Resolve map/HUD sprite name for a character template and equipment visuals. */
bmin::String resolveCharacterSpriteName(const CharacterSpriteResolveArgs& args);

model::WeaponVisual weaponVisualFromItemTemplate(const model::ItemTemplate* itemTemplate);

void resolveEquippedWeaponVisuals(const model::CharacterPlayer& partyMember,
                                  const db::Database& database,
                                  model::WeaponVisual& outMainHand,
                                  model::WeaponVisual& outOffHand);

/** Relative block base index within a weaponCombos sheet (before spriteOffset). */
int weaponComboBlockBase(model::WeaponVisual mainHand,
                         model::WeaponVisual offHand,
                         bool inCombat);

int weaponComboBlockFrameCount(int blockBase);

} // namespace game
