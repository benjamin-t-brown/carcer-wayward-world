#include "lib/sdl2w/Logger.h"
#include "runner/StringEvaluator.h"
#include <unordered_map>

#define TEST_NAME "TestStringEvaluator"

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

  const std::vector<std::pair<std::string, std::pair<std::string, std::string>>>
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

  const std::vector<std::pair<std::string, bool>> invalidSyntax = {
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

    std::vector<std::pair<std::string, std::string>> failedTests;
    for (int i = 0; i < static_cast<int>(basicTestCases.size()); i++) {
      const auto& [expression, expectedPair] = basicTestCases[i];
      if (i == runOnlyIndex || runOnlyIndex == -1) {
        runner::StringEvaluator evaluator(initialStorage, expression);
        evaluator.evalStr(expression);
        std::string result =
            runner::getStorage(initialStorage, expectedPair.first).value_or("");
        LOG(INFO) << "Running test " << i << ": " << expression << " -> storage["
                  << expectedPair.first << "] = \"" << result << "\"" << LOG_ENDL;

        if (result != expectedPair.second) {
          LOG(ERROR) << " Test " << i << " failed: " << expression << " should be \""
                     << expectedPair.second << "\" but got \"" << result << "\""
                     << LOG_ENDL;
          failedTests.push_back({expression, result});
        }
      }
    }

    LOG(INFO) << "== Running invalid syntax tests ==" << LOG_ENDL;

    for (int i = 0; i < static_cast<int>(invalidSyntax.size()); i++) {
      const auto& [expression, expected] = invalidSyntax[i];
      if (i == runOnlyIndex || runOnlyIndex == -1) {
        std::unordered_map<std::string, std::string> storage = initialStorage;
        runner::StringEvaluator evaluator(storage, expression);
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
      return 1;
    }
    LOG(INFO) << TEST_NAME << " completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error in test: " << e.what() << LOG_ENDL;
    return 1;
  }
}
