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
    throwRock.subSteps.pushBack(leaveTavern);
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
    }
    {
      in3::StringEvaluator evaluator(questStorage, "COMPLETE_QUEST_STEP(alineaBartoRock, get-rock)");
      evaluator.evalStr("COMPLETE_QUEST_STEP(alineaBartoRock, get-rock)");
      auto step = in3::getStorage(questStorage, "vars.quests.alineaBartoRock.step");
      auto done = in3::getStorage(questStorage, "vars.quests.alineaBartoRock.completed.get-rock");
      if (!step || *step != "throw-rock") {
        LOG(ERROR) << "Completing a top-level step should advance to the next" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (!done || *done != "true") {
        LOG(ERROR) << "COMPLETE_QUEST_STEP should mark the named step complete" << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
    }
    {
      in3::StringEvaluator evaluator(questStorage,
                                     "COMPLETE_QUEST_STEP(alineaBartoRock, leave-tavern)");
      evaluator.evalStr("COMPLETE_QUEST_STEP(alineaBartoRock, leave-tavern)");
      auto step = in3::getStorage(questStorage, "vars.quests.alineaBartoRock.step");
      auto done = in3::getStorage(questStorage, "vars.quests.alineaBartoRock.completed.leave-tavern");
      if (!step || *step != "throw-rock") {
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
    }

    in3::setQuestTemplates(nullptr);
    LOG(INFO) << TEST_NAME << " completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error in test: " << e.what() << LOG_ENDL;
    return 1;
  }
}
