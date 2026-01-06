#include "lib/sdl2w/Logger.h"
#include "runner/ConditionEvaluator.h"

#define TEST_NAME "TestConditionalEvaluator"

int main(int argc, char** argv) {
  LOG(INFO) << "Starting " << TEST_NAME << LOG_ENDL;
  std::unordered_map<std::string, std::string> initialStorage = {
      // clang-format off
      {"a", "0"},
      {"b", "1"},
      {"c", "2"},
      {"d", "3"},
      // z is undefined
      // clang-format on
  };
  const std::vector<std::pair<std::string, bool>> basicTestCases = {
      // clang-format off
      {"IS(a)", false}, // IS("0") should be false
      {"IS(b)", true},
      {"IS(z)", false},
      {"IS(true)", true},
      {"IS(false)", false},
      {"ISNOT(a)", true}, // ISNOT("0") should be true
      {"ISNOT(b)", false},
      {"ISNOT(z)", true}, // ISNOT(undefined) should be true
      {"EQ(a, b)", false},
      {"EQ(a, a)", true},
      {"EQ(z, z)", false}, // undefined == undefined should be false, any undefined EQ is false
      {"EQ(a, 0)", true},
      {"EQ(b, 1)", true},
      {"EQ(z, 0)", false},
      {"NEQ(a, b)", true},
      {"NEQ(a, a)", false},
      {"NEQ(z, z)", true}, // undefined != undefined should be true, opposite of EQ(z, z)
      {"NEQ(z, 0)", true},
      {"GT(a, b)", false},
      {"GT(b, a)", true},
      {"GT(a, z)", false}, // a > undefined
      {"GT(b, 0)", true},
      {"GT(b, 0.5)", true},
      {"GT(a, 0)", false},
      {"GT(a, -1)", true},
      {"GTE(a, b)", false},
      {"GTE(b, a)", true},
      {"GTE(a, a)", true},
      {"GTE(a, z)", false}, // a >= undefined should be false
      {"GTE(a, 0)", true},
      {"GTE(b, 0)", true},
      {"GTE(a, -1)", true},
      {"GTE(b, 100)", false},
      {"LT(a, b)", true},
      {"LT(b, a)", false},
      {"LT(a, a)", false},
      {"LT(a, z)", false}, // a < undefined
      {"LT(a, 0)", false},
      {"LT(a, 1)", true},
      {"LTE(a, b)", true},
      {"LTE(b, a)", false},
      {"LTE(a, a)", true},
      {"LTE(a, b)", true},
      {"LTE(a, z)", false}, // a <= undefined
      {"LTE(a, 0)", true},
      {"LTE(a, -1)", false},
      {"ALL(IS(a), IS(b))", false}, // since a is 0, this should be false
      {"ALL(IS(b), IS(c))", true},
      {"ALL(IS(a), IS(z))", false},
      {"ALL(EQ(a, a), EQ(b, b))", true},
      {"ALL(EQ(a, b), EQ(b, b))", false},
      {"ALL(EQ(a, b), EQ(c, d))", false},
      {"ALL(EQ(a, a), EQ(b, b), ALL(EQ(c, c), EQ(d, d)))", true},
      {"ANY(IS(a), IS(b))", true},
      {"ANY(IS(a), IS(z))", false}, // tests is: 0 or is: undefined
      {"ANY(EQ(a, a), EQ(b, b))", true},
      {"ANY(EQ(a, b), EQ(b, b))", true},
      {"ANY(EQ(a, b), EQ(c, d))", false},
      {"ANY(EQ(a, b), EQ(c, d), ANY(EQ(a, b), EQ(b, b)))", true},
      // clang-format on
  };
  const std::vector<std::pair<std::string, bool>> invalidSyntax = {
      // clang-format off
    {"a", false},
    {"1", false},
    {"IS(a", false},
    {"UNKNOWN_FUNC(1)", false},
    {"ALL(a, b)", false},
      // clang-format on
  };

  int runOnlyIndex = -1; // debug
  try {
    LOG(INFO) << "== Running basic tests ==" << LOG_ENDL;

    std::vector<std::pair<std::string, bool>> failedTests;
    for (int i = 0; i < static_cast<int>(basicTestCases.size()); i++) {
      const auto& [condition, expected] = basicTestCases[i];
      if (i == runOnlyIndex || runOnlyIndex == -1) {
        runner::ConditionEvaluator evaluator(initialStorage, condition);
        bool result = evaluator.evalCondition(condition);
        LOG(INFO) << "Running test " << i << ": " << condition << " -> "
                  << (result ? "true" : "false") << LOG_ENDL;
        if (result != expected) {
          LOG(ERROR) << " Test " << i << " failed: " << condition << " should be "
                     << (expected ? "true" : "false") << LOG_ENDL;
          failedTests.push_back({condition, result});
        }
      }
    }

    LOG(INFO) << "== Running invalid syntax tests ==" << LOG_ENDL;

    for (int i = 0; i < static_cast<int>(invalidSyntax.size()); i++) {
      const auto& [condition, expected] = invalidSyntax[i];
      if (i == runOnlyIndex || runOnlyIndex == -1) {
        runner::ConditionEvaluator evaluator(initialStorage, condition);
        try {
          LOG(INFO) << "Running invalid syntax test " << i << ": " << condition
                    << LOG_ENDL;
          evaluator.evalCondition(condition);
        } catch (const std::exception& e) {
          // expect invalid syntax
          LOG(INFO) << " Exception caught for invalid syntax: " << e.what() << LOG_ENDL;
        }
      }
    }
    if (!failedTests.empty()) {
      LOG(ERROR) << "Test failed: " << failedTests.size() << " tests failed out of "
                 << basicTestCases.size() << LOG_ENDL;
      return 1;
    }
    LOG(INFO) << TEST_NAME << " completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error in test: " << e.what() << LOG_ENDL;
    return 1;
  }
}
