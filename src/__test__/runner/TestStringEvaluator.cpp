#include "lib/bmin/Map.h"
#include "lib/sdl2w/Logger.h"
#include "runner/StringEvaluator.h"

#include <vector>

#define TEST_NAME "TestStringEvaluator"

int main(int argc, char** argv) {
  LOG(INFO) << "Starting " << TEST_NAME << LOG_ENDL;
  bmin::Map<String, String> initialStorage;
  // clang-format off
  initialStorage.insert(String("a"), String("0"));
  initialStorage.insert(String("b"), String("1"));
  initialStorage.insert(String("c"), String("2"));
  initialStorage.insert(String("d"), String("3"));
  // z is undefined
  // clang-format on

  const DynArray<std::pair<String, std::pair<String, String>>>
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

  const DynArray<std::pair<String, bool>> invalidSyntax = {
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

    DynArray<std::pair<String, String>> failedTests;
    for (int i = 0; i < static_cast<int>(basicTestCases.size()); i++) {
      const auto& [expression, expectedPair] = basicTestCases[i];
      if (i == runOnlyIndex || runOnlyIndex == -1) {
        runner::StringEvaluator evaluator(initialStorage, expression);
        evaluator.evalStr(expression);
        String result =
            runner::getStorage(initialStorage, expectedPair.first).value_or("");
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
        bmin::Map<String, String> storage = initialStorage;
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
