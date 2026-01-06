#include "lib/sdl2w/Logger.h"
#include "runner/ConditionEvaluator.h"
#include <unordered_map>

#define TEST_NAME "TestConditionalEvaluator"

int main(int argc, char** argv) {
  LOG(INFO) << "Starting " << TEST_NAME << LOG_ENDL;
  std::unordered_map<std::string, std::string> initialStorage = {
      {"a", "0"},
      {"b", "1"},
      {"c", "2"},
      {"d", "3"},
      // z is undefined
  };
  const std::unordered_map<std::string, bool> basicTestCases = {
      // --- formatting
      {"IS(a)", true},
      {"IS(z)", false},
      // ---
      {"ISNOT(a)", false},
      {"EQ(a, b)", false},
      {"EQ(a, a)", true},
      {"EQ(z, z)", true}, // undefined == undefined
      {"NEQ(a, b)", true},
      {"NEQ(a, a)", false},
      {"NEQ(z, z)", false},
      {"GT(a, b)", false},
      {"GT(b, a)", true},
      {"GT(a, z)", false}, // a > undefined
      {"GTE(a, b)", false},
      {"GTE(b, a)", true},
      {"GTE(a, a)", true},
      {"GTE(a, z)", true}, // a >= undefined
      {"LT(a, b)", true},
      {"LT(b, a)", false},
      {"LT(a, a)", false},
      {"LT(a, z)", false}, // a < undefined
      {"LTE(a, b)", true},
      {"LTE(b, a)", false},
      {"LTE(a, a)", true},
      {"LTE(a, b)", true},
      {"LTE(a, z)", false}, // a <= undefined
      {"ALL(IS(a), IS(b))", true},
      {"ALL(IS(a), IS(z))", false},
      {"ALL(EQ(a, a), EQ(b, b))", true},
      {"ALL(EQ(a, b), EQ(b, b))", false},
      {"ALL(EQ(a, b), EQ(c, d))", false},
      {"ANY(IS(a), IS(b))", true},
      {"ANY(IS(a), IS(z))", true},
      {"ANY(EQ(a, a), EQ(b, b))", true},
      {"ANY(EQ(a, b), EQ(b, b))", true},
      {"ANY(EQ(a, b), EQ(c, d))", false},
      // ---
  };
  try {
    std::vector<std::pair<std::string, bool>> failedTests;
    for (const auto& [condition, expected] : basicTestCases) {
      runner::ConditionEvaluator evaluator(initialStorage, condition);
      bool result = evaluator.evalCondition(condition);
      if (result != expected) {
        failedTests.push_back({condition, result});
      }
    }
    if (!failedTests.empty()) {
      LOG(ERROR) << "Test failed: " << failedTests.size() << " tests failed" << LOG_ENDL;
      for (const auto& [condition, result] : failedTests) {
        LOG(ERROR) << "  " << condition << " -> " << result << LOG_ENDL;
      }
      return 1;
    }
    LOG(INFO) << TEST_NAME << " completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error in test: " << e.what() << LOG_ENDL;
    return 1;
  }
}
