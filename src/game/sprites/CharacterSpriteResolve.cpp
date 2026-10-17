#include "game/sprites/CharacterSpriteResolve.h"

#include "bmin/StringInterop.h"
#include "db/Database.h"
#include "model/instances/CharacterPlayer.h"

#include <algorithm>
#include <charconv>
#include <stdexcept>
#include <system_error>

namespace game {

namespace {

// weaponCombos sheet layout (relative to spriteOffset):
// 0 town idle, 1 unarmed combat idle, 2 unarmed/town attack,
// 3-4 knife, 5-6 sword, 7-8 staff, 9-10 spear,
// 11-13 knife+knife, 14-16 knife+sword, 17-19 sword+knife, 20-22 sword+sword.
constexpr int kBlockRegular = 0;
constexpr int kBlockKnife = 3;
constexpr int kBlockSword = 5;
constexpr int kBlockStaff = 7;
constexpr int kBlockSpear = 9;
constexpr int kBlockKnifeKnife = 11;
constexpr int kBlockKnifeSword = 14;
constexpr int kBlockSwordKnife = 17;
constexpr int kBlockSwordSword = 20;

bool isKnifeOrSword(model::WeaponVisual visual) {
  return visual == model::WeaponVisual::Knife || visual == model::WeaponVisual::Sword;
}

int singleWeaponBlockBase(model::WeaponVisual visual) {
  switch (visual) {
  case model::WeaponVisual::Knife:
    return kBlockKnife;
  case model::WeaponVisual::Sword:
    return kBlockSword;
  case model::WeaponVisual::Staff:
    return kBlockStaff;
  case model::WeaponVisual::Spear:
    return kBlockSpear;
  case model::WeaponVisual::None:
    break;
  }
  return kBlockRegular;
}

int dualWeaponBlockBase(model::WeaponVisual mainHand, model::WeaponVisual offHand) {
  if (mainHand == model::WeaponVisual::Knife && offHand == model::WeaponVisual::Knife) {
    return kBlockKnifeKnife;
  }
  if (mainHand == model::WeaponVisual::Knife && offHand == model::WeaponVisual::Sword) {
    return kBlockKnifeSword;
  }
  if (mainHand == model::WeaponVisual::Sword && offHand == model::WeaponVisual::Knife) {
    return kBlockSwordKnife;
  }
  if (mainHand == model::WeaponVisual::Sword && offHand == model::WeaponVisual::Sword) {
    return kBlockSwordSword;
  }
  return -1;
}

int parseSpriteOffsetIndex(const bmin::String& spriteOffset) {
  const auto view = bmin::toStringView(spriteOffset);
  int value = 0;
  const auto [ptr, ec] = std::from_chars(view.data(), view.data() + view.size(), value);
  if (ec != std::errc{} || ptr != view.data() + view.size()) {
    throw std::runtime_error("Invalid character spriteOffset");
  }
  return value;
}

const model::CharacterInventoryItem*
findInventoryItemById(const model::CharacterPlayer& character, const bmin::String& itemId) {
  if (itemId.empty()) {
    return nullptr;
  }
  for (size_t i = 0; i < character.inventory.size(); ++i) {
    if (character.inventory[i].id == itemId) {
      return &character.inventory[i];
    }
  }
  return nullptr;
}

} // namespace

int weaponComboBlockBase(model::WeaponVisual mainHand,
                         model::WeaponVisual offHand,
                         bool inCombat) {
  if (!inCombat) {
    return kBlockRegular;
  }

  const bool hasMain = mainHand != model::WeaponVisual::None;
  const bool hasOff = offHand != model::WeaponVisual::None;

  if (!hasMain && !hasOff) {
    return kBlockRegular;
  }

  if (hasMain && hasOff) {
    if (isKnifeOrSword(mainHand) && isKnifeOrSword(offHand)) {
      const auto dual = dualWeaponBlockBase(mainHand, offHand);
      if (dual >= 0) {
        return dual;
      }
    }
    return singleWeaponBlockBase(mainHand);
  }

  if (hasMain) {
    return singleWeaponBlockBase(mainHand);
  }
  return singleWeaponBlockBase(offHand);
}

int weaponComboBlockFrameCount(int blockBase) {
  switch (blockBase) {
  case kBlockKnifeKnife:
  case kBlockKnifeSword:
  case kBlockSwordKnife:
  case kBlockSwordSword:
    return 3;
  default:
    return 2;
  }
}

bmin::String resolveCharacterSpriteName(const CharacterSpriteResolveArgs& args) {
  const auto& characterTemplate = args.characterTemplate;
  const auto base = parseSpriteOffsetIndex(characterTemplate.spriteOffset);
  auto pose = std::max(0, args.pose);

  if (characterTemplate.spriteLayout != model::CharacterSpriteLayout::WeaponCombos) {
    pose = std::min(pose, 1);
    return characterTemplate.spritesheetName + "_" + bmin::toString(base + pose);
  }

  const auto blockBase =
      weaponComboBlockBase(args.mainHand, args.offHand, args.inCombat);
  // Regular block: 0 town idle, 1 unarmed combat idle, 2 attack.
  if (blockBase == kBlockRegular) {
    int frame = 0;
    if (pose > 0) {
      frame = 2;
    } else if (args.inCombat) {
      frame = 1;
    }
    return characterTemplate.spritesheetName + "_" + bmin::toString(base + frame);
  }
  const auto frameCount = weaponComboBlockFrameCount(blockBase);
  pose = std::min(pose, frameCount - 1);
  return characterTemplate.spritesheetName + "_" +
         bmin::toString(base + blockBase + pose);
}

model::WeaponVisual weaponVisualFromItemTemplate(const model::ItemTemplate* itemTemplate) {
  if (itemTemplate == nullptr || !itemTemplate->weapon.has_value()) {
    return model::WeaponVisual::None;
  }
  return itemTemplate->weapon->weaponVisual;
}

void resolveEquippedWeaponVisuals(const model::CharacterPlayer& partyMember,
                                  const db::Database& database,
                                  model::WeaponVisual& outMainHand,
                                  model::WeaponVisual& outOffHand) {
  outMainHand = model::WeaponVisual::None;
  outOffHand = model::WeaponVisual::None;

  const auto resolveSlot = [&](const bmin::String& weaponId) -> model::WeaponVisual {
    const auto* inventoryItem = findInventoryItemById(partyMember, weaponId);
    if (inventoryItem == nullptr) {
      return model::WeaponVisual::None;
    }
    const auto* itemTemplate =
        database.findItemTemplate(bmin::toStringView(inventoryItem->itemName));
    return weaponVisualFromItemTemplate(itemTemplate);
  };

  outMainHand = resolveSlot(partyMember.equipment.weapon0Id);
  outOffHand = resolveSlot(partyMember.equipment.weapon1Id);
}

} // namespace game
