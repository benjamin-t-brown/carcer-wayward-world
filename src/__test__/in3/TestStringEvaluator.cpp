#include "bmin/DynArray.h"
#include "bmin/String.h"
#include "bmin/Map.h"
#include "sdl2w/Logger.h"
#include "in3/EventRunnerHelpers.h"
#include "in3/StringEvaluator.h"
#include "in3/QuestProgress.h"
#include "model/templates/Quests.hpp"

#define TEST_NAME "TestStringEvaluator"

int main(int argc, char** argv) {
  LOG(INFO) << "Starting " << TEST_NAME << LOG_ENDL;
  bmin::Map<bmin::String, bmin::String> initialStorage;
  // clang-format off
  initialStorage.insert(bmin::String("a"), bmin::String("0"));
  initialStorage.insert(bmin::String("b"), bmin::String("1"));
  initialStorage.insert(bmin::String("c"), bmin::String("2"));
  initialStorage.insert(bmin::String("d"), bmin::String("3"));
  // z is undefined
  // clang-format on

  const bmin::DynArray<std::pair<bmin::String, std::pair<bmin::String, bmin::String>>>
      basicTestCases = {
          // clang-format off
      {"SET_BOOL(bool1, true)", {"bool1", "true"}},
      {"SET_BOOL(bool2, false)", {"bool2", "false"}},
      {"SET_BOOL(bool3)", {"bool3", "true"}}, // empty defaults to true
      {"SET_NUM(num1, 5)", {"num1", "5"}},
      {"SET_NUM(num2, 3.14)", {"num2", "3.14"}},
      {"SET_NUM(num3, -10)", {"num3", "-10"}},
      {"SET_STR(str1, hello)", {"str1", "hello"}},
      {"SET_STR(str2, test value)", {"str2", "test value"}},
      {"SET_PORT(claire)", {"tmp.talk.port", "claire"}},
      {"SET_PORT()", {"tmp.talk.port", ""}},
      {"ADD_ITEM_TO_PLAYER(BeerPappysLager)", {"vars.items.BeerPappysLager", "1"}},
      {"MOD_NUM(a, 1)", {"a", "1"}},
      {"MOD_NUM(a, -1)", {"a", "0"}},
      {"MOD_NUM(newNum, 10)", {"newNum", "10"}}, // undefined + 10 = 10
          // clang-format on
      };

  const bmin::DynArray<std::pair<bmin::String, bool>> invalidSyntax = {
      // clang-format off
      {"a", false},
      {"1", false},
      {"GET(a", false},
      {"UNKNOWN_FUNC(1)", false},
      // clang-format on
  };

  int runOnlyIndex = -1; // debug
  try {
    LOG(INFO) << "== Running basic tests ==" << LOG_ENDL;

    bmin::DynArray<std::pair<bmin::String, bmin::String>> failedTests;
    for (int i = 0; i < static_cast<int>(basicTestCases.size()); i++) {
      const auto& [expression, expectedPair] = basicTestCases[i];
      if (i == runOnlyIndex || runOnlyIndex == -1) {
        in3::StringEvaluator evaluator(initialStorage, expression);
        evaluator.evalStr(expression);
        bmin::String result =
            in3::getStorage(initialStorage, expectedPair.first).value_or("");
        LOG(INFO) << "Running test " << i << ": " << expression << " -> storage["
                  << expectedPair.first << "] = \"" << result << "\"" << LOG_ENDL;

        if (result != expectedPair.second) {
          LOG(ERROR) << " Test " << i << " failed: " << expression << " should be \""
                     << expectedPair.second << "\" but got \"" << result << "\""
                     << LOG_ENDL;
          failedTests.pushBack({expression, result});
        }
      }
    }

    LOG(INFO) << "== Running invalid syntax tests ==" << LOG_ENDL;

    for (int i = 0; i < static_cast<int>(invalidSyntax.size()); i++) {
      const auto& [expression, expected] = invalidSyntax[i];
      if (i == runOnlyIndex || runOnlyIndex == -1) {
        bmin::Map<bmin::String, bmin::String> storage = initialStorage;
        in3::StringEvaluator evaluator(storage, expression);
        try {
          LOG(INFO) << "Running invalid syntax test " << i << ": " << expression
                    << LOG_ENDL;
          evaluator.evalStr(expression);
          LOG(ERROR) << " Test " << i
                     << " should have thrown exception for: " << expression << LOG_ENDL;
        } catch (const std::exception& e) {
          // expect invalid syntax
          LOG(INFO) << " Exception caught for invalid syntax: " << e.what() << LOG_ENDL;
        }
      }
    }

    if (!failedTests.empty()) {
      LOG(ERROR) << "Test failed: " << failedTests.size() << " tests failed" << LOG_ENDL;
      in3::setQuestTemplates(nullptr);
      return 1;
    }

    LOG(INFO) << "== Running quest tests ==" << LOG_ENDL;
    model::QuestTemplate rock;
    rock.id = "alineaBartoRock";
    model::QuestStep getRock;
    getRock.id = "get-rock";
    model::QuestStep throwRock;
    throwRock.id = "throw-rock";
    model::QuestStep leaveTavern;
    leaveTavern.id = "leave-tavern";
    model::QuestStep hitBartolo;
    hitBartolo.id = "hit-bartolo";
    throwRock.subSteps.pushBack(leaveTavern);
    throwRock.subSteps.pushBack(hitBartolo);
    rock.steps.pushBack(getRock);
    rock.steps.pushBack(throwRock);

    model::QuestTemplate emptyQuest;
    emptyQuest.id = "emptyQuest";

    bmin::Map<bmin::String, model::QuestTemplate> quests;
    quests[rock.id] = rock;
    quests[emptyQuest.id] = emptyQuest;
    in3::setQuestTemplates(&quests);

    bmin::Map<bmin::String, bmin::String> questStorage;
    {
      in3::StringEvaluator evaluator(questStorage, "START_QUEST(emptyQuest)");
      evaluator.evalStr("START_QUEST(emptyQuest)");
      if (in3::getStorage(questStorage, "vars.quests.emptyQuest.step")) {
        LOG(ERROR) << "START_QUEST with no steps should be a noop" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
    }
    {
      in3::StringEvaluator evaluator(questStorage, "START_QUEST(alineaBartoRock)");
      evaluator.evalStr("START_QUEST(alineaBartoRock)");
      auto step = in3::getStorage(questStorage, "vars.quests.alineaBartoRock.step");
      if (!step || *step != "get-rock") {
        LOG(ERROR) << "START_QUEST should set the first top-level step" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!evaluator.funcs.questUpdated) {
        LOG(ERROR) << "START_QUEST should flag a journal update" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
    }
    {
      bmin::Map<bmin::String, bmin::String> jumpStorage;
      in3::StringEvaluator startEvaluator(jumpStorage, "START_QUEST(alineaBartoRock)");
      startEvaluator.evalStr("START_QUEST(alineaBartoRock)");
      in3::StringEvaluator evaluator(jumpStorage,
                                     "SET_QUEST_STEP_EQ(alineaBartoRock, throw-rock)");
      evaluator.evalStr("SET_QUEST_STEP_EQ(alineaBartoRock, throw-rock)");
      auto step = in3::getStorage(jumpStorage, "vars.quests.alineaBartoRock.step");
      if (!step || *step != "throw-rock") {
        LOG(ERROR) << "SET_QUEST_STEP_EQ should set the current quest step" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!in3::questStepEq(jumpStorage, "alineaBartoRock", "throw-rock")) {
        LOG(ERROR) << "SET_QUEST_STEP_EQ should satisfy QUEST_STEP_EQ" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (in3::questStepIsCompleted(jumpStorage, "alineaBartoRock", "get-rock")) {
        LOG(ERROR) << "SET_QUEST_STEP_EQ should not complete skipped steps" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (in3::questStepVisibleInJournal(jumpStorage, "alineaBartoRock", "get-rock")) {
        LOG(ERROR) << "Skipped incomplete steps should stay hidden from the journal"
                   << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!in3::questStepVisibleInJournal(jumpStorage, "alineaBartoRock", "throw-rock")) {
        LOG(ERROR) << "The current quest step should be visible in the journal" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!evaluator.funcs.questUpdated) {
        LOG(ERROR) << "SET_QUEST_STEP_EQ should flag a journal update" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      in3::StringEvaluator completeEvaluator(
          jumpStorage, "COMPLETE_QUEST_STEP(alineaBartoRock, get-rock)");
      completeEvaluator.evalStr("COMPLETE_QUEST_STEP(alineaBartoRock, get-rock)");
      auto visibleAfterComplete =
          in3::questJournalVisibleStepIds(jumpStorage, "alineaBartoRock");
      if (visibleAfterComplete.size() != 2 || visibleAfterComplete[0] != "get-rock" ||
          visibleAfterComplete[1] != "throw-rock") {
        LOG(ERROR) << "Journal should show completed steps plus the current step" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
    }
    {
      bmin::Map<bmin::String, bmin::String> subStorage;
      in3::StringEvaluator startEvaluator(subStorage, "START_QUEST(alineaBartoRock)");
      startEvaluator.evalStr("START_QUEST(alineaBartoRock)");
      in3::StringEvaluator evaluator(
          subStorage, "COMPLETE_QUEST_SUB_STEP(alineaBartoRock, throw-rock, leave-tavern)");
      evaluator.evalStr("COMPLETE_QUEST_SUB_STEP(alineaBartoRock, throw-rock, leave-tavern)");
      auto step = in3::getStorage(subStorage, "vars.quests.alineaBartoRock.step");
      auto done =
          in3::getStorage(subStorage, "vars.quests.alineaBartoRock.completed.leave-tavern");
      if (!step || *step != "get-rock") {
        LOG(ERROR) << "COMPLETE_QUEST_SUB_STEP should not change the current top-level step"
                   << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!done || *done != "true") {
        LOG(ERROR) << "COMPLETE_QUEST_SUB_STEP should mark the sub-step complete" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!in3::questStepEq(subStorage, "alineaBartoRock", "leave-tavern")) {
        LOG(ERROR) << "QUEST_STEP_EQ should treat a completed sub-step as a separate check"
                   << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
    }
    {
      bmin::Map<bmin::String, bmin::String> showStorage;
      in3::StringEvaluator startEvaluator(showStorage, "START_QUEST(alineaBartoRock)");
      startEvaluator.evalStr("START_QUEST(alineaBartoRock)");
      in3::StringEvaluator jumpEvaluator(showStorage,
                                         "SET_QUEST_STEP_EQ(alineaBartoRock, throw-rock)");
      jumpEvaluator.evalStr("SET_QUEST_STEP_EQ(alineaBartoRock, throw-rock)");
      if (in3::questStepVisibleInJournal(showStorage, "alineaBartoRock", "leave-tavern") ||
          in3::questStepVisibleInJournal(showStorage, "alineaBartoRock", "hit-bartolo")) {
        LOG(ERROR) << "Incomplete sub-steps should stay hidden until shown or completed"
                   << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      in3::StringEvaluator showEvaluator(
          showStorage, "SHOW_QUEST_SUB_STEP(alineaBartoRock, throw-rock, leave-tavern)");
      showEvaluator.evalStr("SHOW_QUEST_SUB_STEP(alineaBartoRock, throw-rock, leave-tavern)");
      auto shown =
          in3::getStorage(showStorage, "vars.quests.alineaBartoRock.shown.leave-tavern");
      if (!shown || *shown != "true") {
        LOG(ERROR) << "SHOW_QUEST_SUB_STEP should mark the sub-step shown" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!in3::questSubStepIsShown(showStorage, "alineaBartoRock", "throw-rock",
                                    "leave-tavern")) {
        LOG(ERROR) << "SHOW_QUEST_SUB_STEP should satisfy QUEST_SUB_STEP_SHOWN" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!in3::questStepVisibleInJournal(showStorage, "alineaBartoRock", "leave-tavern")) {
        LOG(ERROR) << "A shown sub-step should be visible in the journal" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (in3::questStepVisibleInJournal(showStorage, "alineaBartoRock", "hit-bartolo")) {
        LOG(ERROR) << "SHOW_QUEST_SUB_STEP should not reveal sibling sub-steps" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!showEvaluator.funcs.questUpdated) {
        LOG(ERROR) << "SHOW_QUEST_SUB_STEP should flag a journal update" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      in3::StringEvaluator hideEvaluator(
          showStorage, "HIDE_QUEST_SUB_STEP(alineaBartoRock, throw-rock, leave-tavern)");
      hideEvaluator.evalStr("HIDE_QUEST_SUB_STEP(alineaBartoRock, throw-rock, leave-tavern)");
      if (in3::getStorage(showStorage, "vars.quests.alineaBartoRock.shown.leave-tavern")) {
        LOG(ERROR) << "HIDE_QUEST_SUB_STEP should clear the shown flag" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (in3::questStepVisibleInJournal(showStorage, "alineaBartoRock", "leave-tavern")) {
        LOG(ERROR) << "A hidden incomplete sub-step should leave the journal" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!hideEvaluator.funcs.questUpdated) {
        LOG(ERROR) << "HIDE_QUEST_SUB_STEP should flag a journal update" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      in3::StringEvaluator showAgainEvaluator(
          showStorage, "SHOW_QUEST_SUB_STEP(alineaBartoRock, throw-rock, leave-tavern)");
      showAgainEvaluator.evalStr("SHOW_QUEST_SUB_STEP(alineaBartoRock, throw-rock, leave-tavern)");
      in3::StringEvaluator completeEvaluator(
          showStorage, "COMPLETE_QUEST_SUB_STEP(alineaBartoRock, throw-rock, leave-tavern)");
      completeEvaluator.evalStr(
          "COMPLETE_QUEST_SUB_STEP(alineaBartoRock, throw-rock, leave-tavern)");
      in3::StringEvaluator hideCompletedEvaluator(
          showStorage, "HIDE_QUEST_SUB_STEP(alineaBartoRock, throw-rock, leave-tavern)");
      hideCompletedEvaluator.evalStr(
          "HIDE_QUEST_SUB_STEP(alineaBartoRock, throw-rock, leave-tavern)");
      if (!in3::questStepVisibleInJournal(showStorage, "alineaBartoRock", "leave-tavern")) {
        LOG(ERROR) << "A completed sub-step should stay in the journal after hide" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
    }
    {
      bmin::Map<bmin::String, bmin::String> restartStorage;
      in3::StringEvaluator startEvaluator(restartStorage, "START_QUEST(alineaBartoRock)");
      startEvaluator.evalStr("START_QUEST(alineaBartoRock)");
      in3::StringEvaluator showEvaluator(
          restartStorage, "SHOW_QUEST_SUB_STEP(alineaBartoRock, throw-rock, leave-tavern)");
      showEvaluator.evalStr("SHOW_QUEST_SUB_STEP(alineaBartoRock, throw-rock, leave-tavern)");
      in3::StringEvaluator restartEvaluator(restartStorage, "START_QUEST(alineaBartoRock)");
      restartEvaluator.evalStr("START_QUEST(alineaBartoRock)");
      if (in3::getStorage(restartStorage, "vars.quests.alineaBartoRock.shown.leave-tavern")) {
        LOG(ERROR) << "START_QUEST should clear shown sub-steps" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
    }
    {
      bmin::Map<bmin::String, bmin::String> mismatchStorage;
      in3::StringEvaluator evaluator(
          mismatchStorage, "COMPLETE_QUEST_SUB_STEP(alineaBartoRock, get-rock, leave-tavern)");
      evaluator.evalStr("COMPLETE_QUEST_SUB_STEP(alineaBartoRock, get-rock, leave-tavern)");
      if (in3::getStorage(mismatchStorage, "vars.quests.alineaBartoRock.completed.leave-tavern")) {
        LOG(ERROR) << "COMPLETE_QUEST_SUB_STEP should ignore a sub-step that is not under "
                      "the given step"
                   << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      in3::StringEvaluator showEvaluator(
          mismatchStorage, "SHOW_QUEST_SUB_STEP(alineaBartoRock, get-rock, leave-tavern)");
      showEvaluator.evalStr("SHOW_QUEST_SUB_STEP(alineaBartoRock, get-rock, leave-tavern)");
      if (in3::getStorage(mismatchStorage, "vars.quests.alineaBartoRock.shown.leave-tavern")) {
        LOG(ERROR) << "SHOW_QUEST_SUB_STEP should ignore a sub-step that is not under "
                      "the given step"
                   << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
    }
    {
      in3::StringEvaluator evaluator(questStorage, "COMPLETE_QUEST_STEP(alineaBartoRock, get-rock)");
      evaluator.evalStr("COMPLETE_QUEST_STEP(alineaBartoRock, get-rock)");
      auto step = in3::getStorage(questStorage, "vars.quests.alineaBartoRock.step");
      auto done = in3::getStorage(questStorage, "vars.quests.alineaBartoRock.completed.get-rock");
      if (!step || *step != "get-rock") {
        LOG(ERROR) << "COMPLETE_QUEST_STEP should not advance to the next listed step"
                   << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!done || *done != "true") {
        LOG(ERROR) << "COMPLETE_QUEST_STEP should mark the named step complete" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!in3::questStepIsCompleted(questStorage, "alineaBartoRock", "get-rock")) {
        LOG(ERROR) << "COMPLETE_QUEST_STEP should satisfy QUEST_STEP_COMPLETED" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (in3::questStepVisibleInJournal(questStorage, "alineaBartoRock", "throw-rock")) {
        LOG(ERROR) << "Incomplete non-current steps should stay hidden from the journal"
                   << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      {
        auto visible = in3::questJournalVisibleStepIds(questStorage, "alineaBartoRock");
        if (visible.size() != 1 || visible[0] != "get-rock") {
          LOG(ERROR) << "Journal should only list the current completed step" << LOG_ENDL;
          in3::setQuestTemplates(nullptr);
          return 1;
        }
      }
    }
    {
      in3::StringEvaluator evaluator(questStorage,
                                     "COMPLETE_QUEST_STEP(alineaBartoRock, leave-tavern)");
      evaluator.evalStr("COMPLETE_QUEST_STEP(alineaBartoRock, leave-tavern)");
      auto step = in3::getStorage(questStorage, "vars.quests.alineaBartoRock.step");
      auto done = in3::getStorage(questStorage, "vars.quests.alineaBartoRock.completed.leave-tavern");
      if (!step || *step != "get-rock") {
        LOG(ERROR) << "Completing a sub-step should not change the current top-level step"
                   << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!done || *done != "true") {
        LOG(ERROR) << "Completing a sub-step should mark it complete" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!in3::questStepEq(questStorage, "alineaBartoRock", "leave-tavern")) {
        LOG(ERROR) << "QUEST_STEP_EQ should treat a completed sub-step as a separate check"
                   << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
    }
    {
      in3::StringEvaluator evaluator(questStorage, "COMPLETE_QUEST(alineaBartoRock)");
      evaluator.evalStr("COMPLETE_QUEST(alineaBartoRock)");
      auto step = in3::getStorage(questStorage, "vars.quests.alineaBartoRock.step");
      if (!step || *step != "complete") {
        LOG(ERROR) << "COMPLETE_QUEST should set the step to complete" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (in3::questIsStarted(questStorage, "alineaBartoRock")) {
        LOG(ERROR) << "A completed quest should not count as started" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!in3::questIsComplete(questStorage, "alineaBartoRock")) {
        LOG(ERROR) << "COMPLETE_QUEST should make QUEST_IS_COMPLETE true" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      {
        auto visible = in3::questJournalVisibleStepIds(questStorage, "alineaBartoRock");
        if (visible.size() != 2 || visible[0] != "get-rock" || visible[1] != "leave-tavern") {
          LOG(ERROR) << "A finished quest journal should list completed steps only"
                     << LOG_ENDL;
          in3::setQuestTemplates(nullptr);
          return 1;
        }
      }
    }

    in3::setQuestTemplates(nullptr);

    {
      bmin::Map<bmin::String, bmin::String> itemStorage;
      in3::StringEvaluator evaluator(itemStorage, "ADD_ITEM_TO_PLAYER(BeerPappysLager)");
      evaluator.evalStr("ADD_ITEM_TO_PLAYER(BeerPappysLager)");
      if (evaluator.funcs.receivedItemNames.size() != 1 ||
          evaluator.funcs.receivedItemNames[0] != "BeerPappysLager") {
        LOG(ERROR) << "ADD_ITEM_TO_PLAYER should flag the granted item" << LOG_ENDL;
        return 1;
      }
    }

    LOG(INFO) << TEST_NAME << " completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error in test: " << e.what() << LOG_ENDL;
    return 1;
  }
}
