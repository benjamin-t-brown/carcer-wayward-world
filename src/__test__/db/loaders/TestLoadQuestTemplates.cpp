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

    const auto heistIt = questTemplates.find(bmin::String("que_alinea_OmniflowerHeist"));
    if (heistIt == questTemplates.end()) {
      LOG(ERROR) << "Missing que_alinea_OmniflowerHeist quest" << LOG_ENDL;
      return 1;
    }
    if (heistIt->value.label != bmin::String("Omniflower Heist")) {
      LOG(ERROR) << "que_alinea_OmniflowerHeist label mismatch" << LOG_ENDL;
      return 1;
    }

    const auto rockIt = questTemplates.find(bmin::String("que_alinea_NobleRuffian"));
    if (rockIt == questTemplates.end()) {
      LOG(ERROR) << "Missing que_alinea_NobleRuffian quest" << LOG_ENDL;
      return 1;
    }
    if (rockIt->value.steps.size() != 2) {
      LOG(ERROR) << "que_alinea_NobleRuffian expected 2 steps, got "
                 << rockIt->value.steps.size() << LOG_ENDL;
      return 1;
    }
    if (rockIt->value.steps[0].id != bmin::String("get-rock")) {
      LOG(ERROR) << "que_alinea_NobleRuffian steps[0].id mismatch" << LOG_ENDL;
      return 1;
    }
    if (rockIt->value.steps[1].subSteps.size() != 2) {
      LOG(ERROR) << "que_alinea_NobleRuffian throw-rock expected 2 subSteps, got "
                 << rockIt->value.steps[1].subSteps.size() << LOG_ENDL;
      return 1;
    }
    if (rockIt->value.steps[1].subSteps[1].id != bmin::String("hit-bartolo")) {
      LOG(ERROR) << "que_alinea_NobleRuffian nested subStep id mismatch" << LOG_ENDL;
      return 1;
    }
    if (rockIt->value.rewards.coins != 0 || rockIt->value.rewards.experience != 0 ||
        !rockIt->value.rewards.items.empty()) {
      LOG(ERROR) << "que_alinea_NobleRuffian should have empty default rewards" << LOG_ENDL;
      return 1;
    }

    const auto sealIt = questTemplates.find(bmin::String("que_alinea_sealOfApproval"));
    if (sealIt == questTemplates.end()) {
      LOG(ERROR) << "Missing que_alinea_sealOfApproval quest" << LOG_ENDL;
      return 1;
    }
    if (sealIt->value.rewards.coins != 50 || sealIt->value.rewards.experience != 5 ||
        !sealIt->value.rewards.items.empty()) {
      LOG(ERROR) << "que_alinea_sealOfApproval rewards mismatch" << LOG_ENDL;
      return 1;
    }

    bmin::Map<bmin::String, model::QuestTemplate> rewardFixtures;
    db::loadQuestTemplates("__test__/db/loaders/quest-rewards-fixture.json", rewardFixtures);
    const auto amountIt = rewardFixtures.find(bmin::String("rewardAmountQuest"));
    if (amountIt == rewardFixtures.end()) {
      LOG(ERROR) << "Missing rewardAmountQuest fixture" << LOG_ENDL;
      return 1;
    }
    if (amountIt->value.rewards.items.size() != 2 ||
        amountIt->value.rewards.items[0].name != "BeerPappysLager" ||
        amountIt->value.rewards.items[0].amount != 15 ||
        amountIt->value.rewards.items[1].name != "AlineaCorrespondence1" ||
        amountIt->value.rewards.items[1].amount != 1) {
      LOG(ERROR) << "rewardAmountQuest should parse item amounts and string items"
                 << LOG_ENDL;
      return 1;
    }

    const auto tomeIt = questTemplates.find(bmin::String("que_alinea_EntomenTome"));
    if (tomeIt == questTemplates.end()) {
      LOG(ERROR) << "Missing que_alinea_EntomenTome quest" << LOG_ENDL;
      return 1;
    }
    if (tomeIt->value.steps.size() != 3) {
      LOG(ERROR) << "que_alinea_EntomenTome expected 3 steps, got "
                 << tomeIt->value.steps.size() << LOG_ENDL;
      return 1;
    }
    if (tomeIt->value.steps[1].id != bmin::String("joinMerchantry") ||
        tomeIt->value.steps[1].subSteps.size() != 4) {
      LOG(ERROR) << "que_alinea_EntomenTome joinMerchantry expected 4 subSteps"
                 << LOG_ENDL;
      return 1;
    }
    if (!tomeIt->value.steps[1].subSteps[0].subSteps.empty()) {
      LOG(ERROR) << "que_alinea_EntomenTome nested empty subSteps should be ignored"
                 << LOG_ENDL;
      return 1;
    }

    LOG(INFO) << "TestLoadQuestTemplates completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error: " << e.what() << LOG_ENDL;
    return 1;
  }
}
