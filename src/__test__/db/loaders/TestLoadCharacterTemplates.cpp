#include "db/Database.h"
#include "db/loaders/LoadCharacterTemplates.h"
#include "sdl2w/Logger.h"

namespace {

bool assertEqual(int actual, int expected, const char* label) {
  if (actual != expected) {
    LOG(ERROR) << label << " expected " << expected << " but got " << actual << LOG_ENDL;
    return false;
  }
  return true;
}

} // namespace

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestLoadCharacterTemplates" << LOG_ENDL;

  try {
    db::Database database;
    database.load();

    const model::CharacterTemplate& member = database.getCharacterTemplate("testPartyMember1");
    bool ok = true;
    ok = assertEqual(member.stats.generic.str, 10, "testPartyMember1.str") && ok;
    ok = assertEqual(member.combat.hp, 100, "testPartyMember1.hp") && ok;
    ok = assertEqual(member.combat.mp, 50, "testPartyMember1.mp") && ok;

    const model::CharacterTemplate& mage = database.getCharacterTemplate("goblinTestMage");
    ok = assertEqual(mage.combat.mp, 20, "goblinTestMage.mp") && ok;
    ok = assertEqual(static_cast<int>(mage.combatBehavior.combat),
                     static_cast<int>(model::CombatBehaviorName::SPELLCASTER),
                     "goblinTestMage combat SPELLCASTER") &&
         ok;
    ok = assertEqual(static_cast<int>(mage.combatSpellAi.spells.size()),
                     3,
                     "goblinTestMage.spell count") &&
         ok;
    if (!mage.combatSpellAi.spells.empty() && mage.combatSpellAi.spells[0] != "FLAME") {
      LOG(ERROR) << "goblinTestMage.spells[0] expected FLAME" << LOG_ENDL;
      ok = false;
    }
    ok = assertEqual(static_cast<int>(mage.combatSpellAi.zoneTargeting),
                     static_cast<int>(model::CombatSpellAiZoneTargeting::DEFAULT),
                     "goblinTestMage zone targeting default") &&
         ok;
    if (member.spritesheetName != "pc") {
      LOG(ERROR) << "testPartyMember1.spritesheet expected pc but got "
                 << member.spritesheetName << LOG_ENDL;
      ok = false;
    }
    if (member.spriteLayout != model::CharacterSpriteLayout::WeaponCombos) {
      LOG(ERROR) << "testPartyMember1.spriteLayout expected weaponCombos" << LOG_ENDL;
      ok = false;
    }

    bmin::Map<bmin::String, model::CharacterTemplate> backcompatCharacters;
    db::loadCharacterTemplates("__test__/assets/characters-backcompat.json",
                               backcompatCharacters);
    const auto backcompatIt = backcompatCharacters.find(bmin::String("backcompatEnemy"));
    if (backcompatIt == backcompatCharacters.end()) {
      LOG(ERROR) << "Missing backcompatEnemy" << LOG_ENDL;
      return 1;
    }
    ok = assertEqual(backcompatIt->value.stats.generic.str, 7, "backcompatEnemy.str") && ok;

    if (!ok) {
      LOG(ERROR) << "Character template assertions failed" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestLoadCharacterTemplates completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error loading character templates: " << e.what() << LOG_ENDL;
    return 1;
  }
}
