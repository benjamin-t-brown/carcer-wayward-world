#include "db/loaders/LoadQuestTemplates.h"
#include "sdl2w/Logger.h"
#include "bmin/String.h"
#include "bmin/Map.h"

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  LOG(INFO) << "Starting TestLoadQuestTemplates" << LOG_ENDL;

  bmin::Map<bmin::String, model::QuestTemplate> questTemplates;

  try {
    db::loadQuestTemplates("assets/db/quests.json", questTemplates);
    LOG(INFO) << "Loaded " << questTemplates.size() << " quest templates" << LOG_ENDL;

    if (questTemplates.size() < 5) {
      LOG(ERROR) << "Expected at least 5 quest templates, got " << questTemplates.size()
                 << LOG_ENDL;
      return 1;
    }

    const auto heistIt = questTemplates.find(bmin::String("alinea_omniflowerHeist"));
    if (heistIt == questTemplates.end()) {
      LOG(ERROR) << "Missing alinea_omniflowerHeist quest" << LOG_ENDL;
      return 1;
    }
    if (heistIt->value.label != bmin::String("Omniflower Heist")) {
      LOG(ERROR) << "alinea_omniflowerHeist label mismatch" << LOG_ENDL;
      return 1;
    }

    const auto rockIt = questTemplates.find(bmin::String("alineaBartoRock"));
    if (rockIt == questTemplates.end()) {
      LOG(ERROR) << "Missing alineaBartoRock quest" << LOG_ENDL;
      return 1;
    }
    if (rockIt->value.steps.size() != 2) {
      LOG(ERROR) << "alineaBartoRock expected 2 steps, got " << rockIt->value.steps.size()
                 << LOG_ENDL;
      return 1;
    }
    if (rockIt->value.steps[0].id != bmin::String("get-rock")) {
      LOG(ERROR) << "alineaBartoRock steps[0].id mismatch" << LOG_ENDL;
      return 1;
    }
    if (rockIt->value.steps[1].subSteps.size() != 2) {
      LOG(ERROR) << "alineaBartoRock throw-rock expected 2 subSteps, got "
                 << rockIt->value.steps[1].subSteps.size() << LOG_ENDL;
      return 1;
    }
    if (rockIt->value.steps[1].subSteps[1].id != bmin::String("hit-bartolo")) {
      LOG(ERROR) << "alineaBartoRock nested subStep id mismatch" << LOG_ENDL;
      return 1;
    }

    const auto tomeIt = questTemplates.find(bmin::String("Entomen's Tome"));
    if (tomeIt == questTemplates.end()) {
      LOG(ERROR) << "Missing Entomen's Tome quest" << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestLoadQuestTemplates completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what() << LOG_ENDL;
    return 1;
  }
}
