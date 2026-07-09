#include "db/loaders/LoadAbilityTemplates.h"
#include "lib/sdl2w/Logger.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestLoadAbilityTemplates" << LOG_ENDL;

  bmin::Map<String, model::AbilityTemplate> abilityTemplates;

  try {
    db::loadAbilityTemplates("assets/db/abilities.json", abilityTemplates);
    LOG(INFO) << "Loaded " << abilityTemplates.size() << " ability templates" << LOG_ENDL;

    const auto meleeIt = abilityTemplates.find(String("MELEE_ATTACK_METAL_SWORD"));
    if (meleeIt == abilityTemplates.end()) {
      LOG(ERROR) << "Missing MELEE_ATTACK_METAL_SWORD ability" << LOG_ENDL;
      return 1;
    }
    if (meleeIt->value.attacks.empty()) {
      LOG(ERROR) << "MELEE_ATTACK_METAL_SWORD should have attacks" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestLoadAbilityTemplates completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what() << LOG_ENDL;
    return 1;
  }
}
