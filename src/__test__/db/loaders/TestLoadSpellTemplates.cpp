#include "db/loaders/LoadSpellTemplates.h"
#include "sdl2w/Logger.h"
#include "bmin/String.h"
#include "bmin/Map.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestLoadSpellTemplates" << LOG_ENDL;

  bmin::Map<bmin::String, model::SpellTemplate> spellTemplates;

  try {
    db::loadSpellTemplates("assets/db/spells.json", spellTemplates);
    LOG(INFO) << "Loaded " << spellTemplates.size() << " spell templates" << LOG_ENDL;

    if (spellTemplates.size() < 2) {
      LOG(ERROR) << "Expected at least 2 spell templates, got " << spellTemplates.size()
                 << LOG_ENDL;
      return 1;
    }

    const auto healIt = spellTemplates.find(bmin::String("HEAL_SELF"));
    if (healIt == spellTemplates.end()) {
      LOG(ERROR) << "Missing HEAL_SELF spell" << LOG_ENDL;
      return 1;
    }
    if (healIt->value.abilityName != bmin::String("SPELL_HEAL_SELF")) {
      LOG(ERROR) << "HEAL_SELF abilityName mismatch" << LOG_ENDL;
      return 1;
    }
    if (healIt->value.requiredRunes.size() != 1) {
      LOG(ERROR) << "HEAL_SELF expected exactly 1 requiredRunes entry, got "
                 << healIt->value.requiredRunes.size() << LOG_ENDL;
      return 1;
    }
    if (healIt->value.requiredRunes[0].type != model::RuneType::REGROWTH ||
        healIt->value.requiredRunes[0].count != 1) {
      LOG(ERROR) << "HEAL_SELF requiredRunes[0] type/count mismatch" << LOG_ENDL;
      return 1;
    }
    if (healIt->value.label.empty()) {
      LOG(ERROR) << "HEAL_SELF label should be set" << LOG_ENDL;
      return 1;
    }

    const auto singeIt = spellTemplates.find(bmin::String("SINGE"));
    if (singeIt == spellTemplates.end()) {
      LOG(ERROR) << "Missing SINGE spell" << LOG_ENDL;
      return 1;
    }
    if (singeIt->value.abilityName != bmin::String("SPELL_SINGE")) {
      LOG(ERROR) << "SINGE abilityName mismatch" << LOG_ENDL;
      return 1;
    }
    if (singeIt->value.requiredRunes.size() != 1) {
      LOG(ERROR) << "SINGE expected exactly 1 requiredRunes entry, got "
                 << singeIt->value.requiredRunes.size() << LOG_ENDL;
      return 1;
    }
    if (singeIt->value.requiredRunes[0].type != model::RuneType::HEAT ||
        singeIt->value.requiredRunes[0].count != 1) {
      LOG(ERROR) << "SINGE requiredRunes[0] type/count mismatch" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestLoadSpellTemplates completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what() << LOG_ENDL;
    return 1;
  }
}
