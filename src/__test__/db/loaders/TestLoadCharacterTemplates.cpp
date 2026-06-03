#include "db/Database.h"
#include "db/loaders/LoadCharacterTemplates.h"
#include "lib/sdl2w/Logger.h"

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

    std::unordered_map<std::string, model::CharacterTemplate> backcompatCharacters;
    db::loadCharacterTemplates("__test__/assets/characters-backcompat.json",
                               backcompatCharacters);
    const auto backcompatIt = backcompatCharacters.find("backcompatEnemy");
    if (backcompatIt == backcompatCharacters.end()) {
      LOG(ERROR) << "Missing backcompatEnemy" << LOG_ENDL;
      return 1;
    }
    ok = assertEqual(backcompatIt->second.stats.generic.str, 7, "backcompatEnemy.str") && ok;

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
