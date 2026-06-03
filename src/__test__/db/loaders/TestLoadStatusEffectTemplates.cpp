#include "db/loaders/LoadStatusEffectTemplates.h"
#include "lib/sdl2w/Logger.h"
#include <unordered_map>

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestLoadStatusEffectTemplates" << LOG_ENDL;

  std::unordered_map<std::string, model::StatusEffectTemplate> statusEffectTemplates;

  try {
    db::loadStatusEffectTemplates("assets/db/status-effects.json", statusEffectTemplates);
    LOG(INFO) << "Loaded " << statusEffectTemplates.size() << " status effect templates"
              << LOG_ENDL;

    const auto burningIt = statusEffectTemplates.find("BURNING");
    if (burningIt == statusEffectTemplates.end()) {
      LOG(ERROR) << "Missing BURNING status effect" << LOG_ENDL;
      return 1;
    }
    if (burningIt->second.events.empty()) {
      LOG(ERROR) << "BURNING should have events" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestLoadStatusEffectTemplates completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what() << LOG_ENDL;
    return 1;
  }
}
