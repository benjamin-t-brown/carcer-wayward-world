#include "game/sprites/CharacterSpriteResolve.h"
#include "model/templates/CharacterTemplate.h"
#include "model/templates/Items.h"
#include "sdl2w/Logger.h"
#include "bmin/String.h"

namespace {

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

bool assertEqualStr(const bmin::String& actual,
                    const char* expected,
                    const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

model::CharacterTemplate makeTemplate(model::CharacterSpriteLayout layout) {
  auto characterTemplate = model::CharacterTemplate{};
  characterTemplate.type = model::CharacterTemplateType::TOWNSPERSON;
  characterTemplate.name = "test";
  characterTemplate.label = "Test";
  characterTemplate.spritesheetName = "pc";
  characterTemplate.spriteOffset = "0";
  characterTemplate.spriteLayout = layout;
  return characterTemplate;
}

} // namespace

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  LOG(INFO) << "Starting TestCharacterSpriteResolve" << LOG_ENDL;

  bool ok = true;

  ok = assertEqual(game::weaponComboBlockBase(model::WeaponVisual::None,
                                              model::WeaponVisual::None,
                                              false),
                   0,
                   "non-combat regular") &&
       ok;
  ok = assertEqual(game::weaponComboBlockBase(model::WeaponVisual::Knife,
                                              model::WeaponVisual::None,
                                              true),
                   3,
                   "combat knife") &&
       ok;
  ok = assertEqual(game::weaponComboBlockBase(model::WeaponVisual::Sword,
                                              model::WeaponVisual::None,
                                              true),
                   5,
                   "combat sword") &&
       ok;
  ok = assertEqual(game::weaponComboBlockBase(model::WeaponVisual::Staff,
                                              model::WeaponVisual::None,
                                              true),
                   7,
                   "combat staff") &&
       ok;
  ok = assertEqual(game::weaponComboBlockBase(model::WeaponVisual::Spear,
                                              model::WeaponVisual::None,
                                              true),
                   9,
                   "combat spear") &&
       ok;
  ok = assertEqual(game::weaponComboBlockBase(model::WeaponVisual::Knife,
                                              model::WeaponVisual::Knife,
                                              true),
                   11,
                   "knife+knife") &&
       ok;
  ok = assertEqual(game::weaponComboBlockBase(model::WeaponVisual::Knife,
                                              model::WeaponVisual::Sword,
                                              true),
                   14,
                   "knife+sword") &&
       ok;
  ok = assertEqual(game::weaponComboBlockBase(model::WeaponVisual::Sword,
                                              model::WeaponVisual::Knife,
                                              true),
                   17,
                   "sword+knife") &&
       ok;
  ok = assertEqual(game::weaponComboBlockBase(model::WeaponVisual::Sword,
                                              model::WeaponVisual::Sword,
                                              true),
                   20,
                   "sword+sword") &&
       ok;
  ok = assertEqual(game::weaponComboBlockBase(model::WeaponVisual::Knife,
                                              model::WeaponVisual::Staff,
                                              true),
                   3,
                   "knife+staff falls back to knife") &&
       ok;
  ok = assertEqual(game::weaponComboBlockBase(model::WeaponVisual::Staff,
                                              model::WeaponVisual::Knife,
                                              true),
                   7,
                   "staff+knife falls back to staff") &&
       ok;
  ok = assertEqual(game::weaponComboBlockBase(model::WeaponVisual::None,
                                              model::WeaponVisual::None,
                                              true),
                   0,
                   "unarmed combat regular") &&
       ok;

  ok = assertEqual(game::weaponComboBlockFrameCount(14), 3, "dual frame count") && ok;
  ok = assertEqual(game::weaponComboBlockFrameCount(3), 2, "single frame count") && ok;

  const auto weaponCombos = makeTemplate(model::CharacterSpriteLayout::WeaponCombos);
  ok = assertEqualStr(game::resolveCharacterSpriteName(
                          {.characterTemplate = weaponCombos,
                           .mainHand = model::WeaponVisual::Knife,
                           .inCombat = true,
                           .pose = 0}),
                      "pc_3",
                      "knife idle") &&
       ok;
  ok = assertEqualStr(game::resolveCharacterSpriteName(
                          {.characterTemplate = weaponCombos,
                           .mainHand = model::WeaponVisual::Knife,
                           .offHand = model::WeaponVisual::Sword,
                           .inCombat = true,
                           .pose = 1}),
                      "pc_15",
                      "knife+sword main attack") &&
       ok;
  ok = assertEqualStr(game::resolveCharacterSpriteName(
                          {.characterTemplate = weaponCombos,
                           .mainHand = model::WeaponVisual::Knife,
                           .offHand = model::WeaponVisual::Sword,
                           .inCombat = true,
                           .pose = 2}),
                      "pc_16",
                      "knife+sword off attack") &&
       ok;
  ok = assertEqualStr(game::resolveCharacterSpriteName(
                          {.characterTemplate = weaponCombos,
                           .mainHand = model::WeaponVisual::Sword,
                           .inCombat = false,
                           .pose = 1}),
                      "pc_2",
                      "non-combat uses regular attack (skips reserved slot 1)") &&
       ok;
  ok = assertEqualStr(game::resolveCharacterSpriteName(
                          {.characterTemplate = weaponCombos,
                           .mainHand = model::WeaponVisual::None,
                           .inCombat = false,
                           .pose = 0}),
                      "pc_0",
                      "town idle") &&
       ok;
  ok = assertEqualStr(game::resolveCharacterSpriteName(
                          {.characterTemplate = weaponCombos,
                           .mainHand = model::WeaponVisual::None,
                           .inCombat = true,
                           .pose = 0}),
                      "pc_1",
                      "unarmed combat idle") &&
       ok;
  ok = assertEqualStr(game::resolveCharacterSpriteName(
                          {.characterTemplate = weaponCombos,
                           .mainHand = model::WeaponVisual::None,
                           .inCombat = true,
                           .pose = 1}),
                      "pc_2",
                      "unarmed combat attack") &&
       ok;
  ok = assertEqualStr(game::resolveCharacterSpriteName(
                          {.characterTemplate = weaponCombos,
                           .mainHand = model::WeaponVisual::Knife,
                           .inCombat = true,
                           .pose = 2}),
                      "pc_4",
                      "single clamps off-hand pose to attack") &&
       ok;

  auto defaultTemplate = makeTemplate(model::CharacterSpriteLayout::Default);
  defaultTemplate.spritesheetName = "actors0";
  defaultTemplate.spriteOffset = "60";
  ok = assertEqualStr(game::resolveCharacterSpriteName(
                          {.characterTemplate = defaultTemplate,
                           .mainHand = model::WeaponVisual::Knife,
                           .inCombat = true,
                           .pose = 1}),
                      "actors0_61",
                      "default layout ignores equipment") &&
       ok;

  if (!ok) {
    LOG(ERROR) << "TestCharacterSpriteResolve assertions failed" << LOG_ENDL;
    return 1;
  }

  LOG(INFO) << "TestCharacterSpriteResolve completed successfully" << LOG_ENDL;
  return 0;
}
