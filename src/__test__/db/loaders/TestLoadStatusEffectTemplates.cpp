#include "db/loaders/LoadStatusEffectTemplates.h"
#include "sdl2w/Logger.h"
#include "bmin/String.h"
#include "bmin/Map.h"

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestLoadStatusEffectTemplates" << LOG_ENDL;

  bmin::Map<bmin::String, model::StatusEffectTemplate> statusEffectTemplates;

  try {
    db::loadStatusEffectTemplates("assets/db/status-effects.json", statusEffectTemplates);
    LOG(INFO) << "Loaded " << statusEffectTemplates.size() << " status effect templates"
              << LOG_ENDL;

    const auto burningIt = statusEffectTemplates.find(bmin::String("BURNING"));
    if (burningIt == statusEffectTemplates.end()) {
      LOG(ERROR) << "Missing BURNING status effect" << LOG_ENDL;
      return 1;
    }
    if (burningIt->value.baseDuration != 3) {
      LOG(ERROR) << "BURNING baseDuration should be 3" << LOG_ENDL;
      return 1;
    }
    if (burningIt->value.actions.empty()) {
      LOG(ERROR) << "BURNING should have actions" << LOG_ENDL;
      return 1;
    }
    if (burningIt->value.actions[0].events.empty()) {
      LOG(ERROR) << "BURNING action should have events" << LOG_ENDL;
      return 1;
    }
    if (burningIt->value.applyResistances.empty()) {
      LOG(ERROR) << "BURNING should have applyResistances" << LOG_ENDL;
      return 1;
    }
    if (burningIt->value.applyResistances[0].attackType !=
        model::DamageType::DAMAGE_TYPE_HEAT) {
      LOG(ERROR) << "BURNING resistance should be DAMAGE_TYPE_HEAT" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestLoadStatusEffectTemplates completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what() << LOG_ENDL;
    return 1;
  }
}
