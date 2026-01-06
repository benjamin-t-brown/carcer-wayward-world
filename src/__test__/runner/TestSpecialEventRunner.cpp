#include "lib/sdl2w/Logger.h"
#include "model/SpecialEvents.h"
#include "runner/SpecialEventRunner.h"
#include <unordered_map>
#include <vector>

int main(int argc, char** argv) {
  LOG(INFO) << "Starting TestSpecialEventRunner" << LOG_ENDL;

  try {
    // Create a simple test GameEvent
    model::GameEvent testEvent;
    testEvent.id = "test_event";
    testEvent.title = "Test Event";
    testEvent.eventType = model::GameEventType::MODAL;
    testEvent.icon = "test_icon";

    // Add a variable
    model::Variable var;
    var.id = "var1";
    var.key = "TEST_VAR";
    var.value = "Hello";
    var.importFrom = "";
    testEvent.vars.push_back(var);

    // Create an EXEC node (root)
    model::GameEventChildExec execNode;
    execNode.eventChildType = model::GameEventChildType::EXEC;
    execNode.id = "root";
    execNode.paragraphs = {"This is a test paragraph with @TEST_VAR variable."};
    execNode.execStr = "SET_STR(test_key, test_value)";
    execNode.next = "choice_node";
    execNode.autoAdvance = false;
    testEvent.children.push_back(execNode);

    // Create a CHOICE node
    model::GameEventChildChoice choiceNode;
    choiceNode.eventChildType = model::GameEventChildType::CHOICE;
    choiceNode.id = "choice_node";
    choiceNode.text = "Choose an option:";
    
    model::Choice choice1;
    choice1.text = "Option 1";
    choice1.prefixText = "";
    choice1.conditionStr = "";
    choice1.evalStr = "SET_BOOL(choice1_selected, true)";
    choice1.next = "end_node";
    choiceNode.choices.push_back(choice1);

    model::Choice choice2;
    choice2.text = "Option 2 (requires test_key)";
    choice2.prefixText = "";
    choice2.conditionStr = "IS(test_key)";
    choice2.evalStr = "SET_BOOL(choice2_selected, true)";
    choice2.next = "end_node";
    choiceNode.choices.push_back(choice2);

    testEvent.children.push_back(choiceNode);

    // Create an END node
    model::GameEventChildEnd endNode;
    endNode.eventChildType = model::GameEventChildType::END;
    endNode.id = "end_node";
    endNode.next = "";
    testEvent.children.push_back(endNode);

    // Create empty gameEvents vector
    std::vector<model::GameEvent> gameEvents;

    // Initialize storage
    std::unordered_map<std::string, std::string> initialStorage;
    initialStorage["initial_value"] = "100";

    // Create the runner
    runner::SpecialEventRunner runner(initialStorage, testEvent, gameEvents);

    LOG(INFO) << "Created runner with currentNodeId: " << runner.currentNodeId
              << LOG_ENDL;

    // Test: Check initial node
    auto currentNode = runner.getCurrentNode();
    if (!currentNode.has_value()) {
      LOG(ERROR) << "Failed to get current node" << LOG_ENDL;
      return 1;
    }
    if (runner.currentNodeId != "root") {
      LOG(ERROR) << "Expected to start at root node, but got: "
                 << runner.currentNodeId << LOG_ENDL;
      return 1;
    }
    LOG(INFO) << "Successfully got current node: " << runner.currentNodeId
              << LOG_ENDL;

    // Test: Manually execute the root node's execStr to test SET_STR
    // (In a real scenario, you'd process the root node first, but for this test
    // we'll test the exec functionality directly)
    bool execResult = runner.evalExecStr("SET_STR(test_key, test_value)");
    if (!execResult) {
      LOG(ERROR) << "Failed to execute SET_STR" << LOG_ENDL;
      return 1;
    }

    // Check that storage was updated
    auto testValue = runner.storage.find("test_key");
    if (testValue == runner.storage.end() || testValue->second != "test_value") {
      LOG(ERROR) << "Storage was not updated correctly by SET_STR" << LOG_ENDL;
      return 1;
    }
    LOG(INFO) << "Storage correctly updated: test_key = " << testValue->second
              << LOG_ENDL;

    // Test: Advance to CHOICE node (this will process the choice node)
    runner.advance("choice_node", {}, "");

    // Check that displayTextChoices were populated
    if (runner.displayTextChoices.empty()) {
      LOG(ERROR) << "displayTextChoices should not be empty after CHOICE node"
                 << LOG_ENDL;
      return 1;
    }
    LOG(INFO) << "Found " << runner.displayTextChoices.size()
              << " choices" << LOG_ENDL;

    // Check that choice2 is available (because test_key exists)
    bool foundChoice2 = false;
    for (const auto& choice : runner.displayTextChoices) {
      if (choice.text.find("Option 2") != std::string::npos) {
        foundChoice2 = true;
        LOG(INFO) << "Choice 2 is available (condition passed)" << LOG_ENDL;
        break;
      }
    }
    if (!foundChoice2) {
      LOG(ERROR) << "Choice 2 should be available because test_key exists"
                 << LOG_ENDL;
      return 1;
    }

    // Test: Select choice1
    if (runner.displayTextChoices.size() > 0) {
      const auto& choice1 = runner.displayTextChoices[0];
      runner.advance(choice1.next, choice1.onceKeysToCommit, choice1.execStr);

      // Check that choice1_selected was set
      auto choice1Selected = runner.storage.find("choice1_selected");
      if (choice1Selected == runner.storage.end() ||
          choice1Selected->second != "true") {
        LOG(ERROR) << "choice1_selected was not set correctly" << LOG_ENDL;
        return 1;
      }
      LOG(INFO) << "Choice 1 executed correctly" << LOG_ENDL;
    }

    // Test: Check for errors
    if (!runner.errors.empty()) {
      LOG(ERROR) << "Found " << runner.errors.size() << " errors:" << LOG_ENDL;
      for (const auto& error : runner.errors) {
        LOG(ERROR) << "  Node: " << error.nodeId
                   << ", Message: " << error.message << LOG_ENDL;
      }
      return 1;
    }

    // Test: Variable replacement
    std::string testText = "Hello @TEST_VAR world";
    std::string replaced = runner.replaceVariables(testText);
    if (replaced.find("Hello") == std::string::npos ||
        replaced.find("world") == std::string::npos) {
      LOG(ERROR) << "Variable replacement failed" << LOG_ENDL;
      return 1;
    }
    LOG(INFO) << "Variable replacement test: '" << testText << "' -> '"
              << replaced << "'" << LOG_ENDL;

    // Test: Condition evaluation
    runner::ConditionResult condResult = runner.evalCondition("IS(test_key)");
    if (!condResult.result) {
      LOG(ERROR) << "Condition IS(test_key) should be true" << LOG_ENDL;
      return 1;
    }
    LOG(INFO) << "Condition evaluation test passed" << LOG_ENDL;

    // Test: Exec string evaluation
    bool execResult2 = runner.evalExecStr("SET_NUM(num_test, 42)");
    if (!execResult2) {
      LOG(ERROR) << "Exec string evaluation failed" << LOG_ENDL;
      return 1;
    }
    auto numTest = runner.storage.find("num_test");
    if (numTest == runner.storage.end() || numTest->second != "42.000000") {
      LOG(ERROR) << "SET_NUM did not work correctly" << LOG_ENDL;
      return 1;
    }
    LOG(INFO) << "Exec string evaluation test passed" << LOG_ENDL;

    LOG(INFO) << "TestSpecialEventRunner completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error in test: " << e.what() << LOG_ENDL;
    return 1;
  }
}

