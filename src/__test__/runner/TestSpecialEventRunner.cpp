#include "lib/bmin/Map.h"
#include "sdl2w/Logger.h"
#include "model/templates/SpecialEvents.h"
#include "runner/EventRunnerHelpers.h"
#include "runner/SpecialEventRunner.h"
#include "bmin/String.h"
#include "bmin/Map.h"


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
    testEvent.vars.pushBack(var);

    // Create an EXEC node (root)
    model::GameEventChildExec execNode;
    execNode.eventChildType = model::GameEventChildType::EXEC;
    execNode.id = "root";
    execNode.paragraphs = {"This is a test paragraph with @TEST_VAR variable."};
    execNode.execStr = "SET_STR(test_key, test_value)";
    execNode.next = "choice_node";
    execNode.autoAdvance = false;
    testEvent.children.pushBack(execNode);

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
    choiceNode.choices.pushBack(choice1);

    model::Choice choice2;
    choice2.text = "Option 2 (requires test_key)";
    choice2.prefixText = "";
    choice2.conditionStr = "IS(test_key)";
    choice2.evalStr = "SET_BOOL(choice2_selected, true)";
    choice2.next = "end_node";
    choiceNode.choices.pushBack(choice2);

    testEvent.children.pushBack(choiceNode);

    // Create an END node
    model::GameEventChildEnd endNode;
    endNode.eventChildType = model::GameEventChildType::END;
    endNode.id = "end_node";
    endNode.next = "";
    testEvent.children.pushBack(endNode);

    bmin::Map<bmin::String, model::GameEvent> gameEvents;

    bmin::Map<bmin::String, bmin::String> initialStorage;
    initialStorage.insert(bmin::String("initial_value"), bmin::String("100"));

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
      LOG(ERROR) << "Expected to start at root node, but got: " << runner.currentNodeId
                 << LOG_ENDL;
      return 1;
    }
    LOG(INFO) << "Successfully got current node: " << runner.currentNodeId << LOG_ENDL;

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
    if (testValue == runner.storage.end() || testValue->value != "test_value") {
      LOG(ERROR) << "Storage was not updated correctly by SET_STR" << LOG_ENDL;
      return 1;
    }
    LOG(INFO) << "Storage correctly updated: test_key = " << testValue->value
              << LOG_ENDL;

    // Test: Advance to CHOICE node (this will process the choice node)
    runner.advance("choice_node", {}, "");

    // Check that displayTextChoices were populated
    if (runner.displayTextChoices.empty()) {
      LOG(ERROR) << "displayTextChoices should not be empty after CHOICE node"
                 << LOG_ENDL;
      return 1;
    }
    LOG(INFO) << "Found " << runner.displayTextChoices.size() << " choices" << LOG_ENDL;

    // Check that choice2 is available (because test_key exists)
    bool foundChoice2 = false;
    for (const auto& choice : runner.displayTextChoices) {
      if (choice.text.find("Option 2") != bmin::String::npos) {
        foundChoice2 = true;
        LOG(INFO) << "Choice 2 is available (condition passed)" << LOG_ENDL;
        break;
      }
    }
    if (!foundChoice2) {
      LOG(ERROR) << "Choice 2 should be available because test_key exists" << LOG_ENDL;
      return 1;
    }

    // Test: Select choice1
    if (runner.displayTextChoices.size() > 0) {
      const auto& choice1 = runner.displayTextChoices[0];
      runner.advance(choice1.next, choice1.onceKeysToCommit, choice1.execStr);

      // Check that choice1_selected was set
      auto choice1Selected = runner.storage.find("choice1_selected");
      if (choice1Selected == runner.storage.end() || choice1Selected->value != "true") {
        LOG(ERROR) << "choice1_selected was not set correctly" << LOG_ENDL;
        return 1;
      }
      LOG(INFO) << "Choice 1 executed correctly" << LOG_ENDL;
    }

    // Test: Check for errors
    if (!runner.errors.empty()) {
      LOG(ERROR) << "Found " << runner.errors.size() << " errors:" << LOG_ENDL;
      for (const auto& error : runner.errors) {
        LOG(ERROR) << "  Node: " << error.nodeId << ", Message: " << error.message
                   << LOG_ENDL;
      }
      return 1;
    }

    // Test: Variable replacement
    bmin::String testText = "Hello @TEST_VAR world";
    bmin::String replaced = runner.replaceVariables(testText);
    if (replaced.find("Hello") == bmin::String::npos ||
        replaced.find("world") == bmin::String::npos) {
      LOG(ERROR) << "Variable replacement failed" << LOG_ENDL;
      return 1;
    }
    LOG(INFO) << "Variable replacement test: '" << testText << "' -> '" << replaced << "'"
              << LOG_ENDL;

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
    if (numTest == runner.storage.end() || numTest->value != "42") {
      LOG(ERROR) << "SET_NUM did not work correctly" << LOG_ENDL;
      return 1;
    }
    LOG(INFO) << "Exec string evaluation test passed" << LOG_ENDL;

    // TALK: auto-advanced EXEC text must carry into the following CHOICE prompt
    // (Claire Remain Silent: znt1b71szsf → uj1jkcvhv0p).
    {
      model::GameEvent talkEvent;
      talkEvent.id = "talk_auto_advance";
      talkEvent.eventType = model::GameEventType::TALK;

      model::GameEventChildExec execNode;
      execNode.eventChildType = model::GameEventChildType::EXEC;
      execNode.id = "root";
      execNode.paragraphs = {"Fee for using the docks."};
      execNode.next = "choice_node";
      execNode.autoAdvance = true;
      talkEvent.children.pushBack(execNode);

      model::GameEventChildChoice choiceNode;
      choiceNode.eventChildType = model::GameEventChildType::CHOICE;
      choiceNode.id = "choice_node";
      choiceNode.text = "A shiver passes across your body.";
      model::Choice staySilent;
      staySilent.text = "(Remain silent.)";
      staySilent.next = "end_node";
      choiceNode.choices.pushBack(staySilent);
      talkEvent.children.pushBack(choiceNode);

      model::GameEventChildEnd endNode;
      endNode.eventChildType = model::GameEventChildType::END;
      endNode.id = "end_node";
      talkEvent.children.pushBack(endNode);

      runner::SpecialEventRunner talkRunner({}, talkEvent, {});
      runner::SpecialEventRunnerInterface talkIface(talkRunner);
      talkIface.startEvent();

      const bmin::String expected =
          "Fee for using the docks.\n\nA shiver passes across your body.";
      if (talkRunner.displayText != expected) {
        LOG(ERROR) << "TALK auto-advance should keep EXEC text with CHOICE text. Got: '"
                   << talkRunner.displayText << "'" << LOG_ENDL;
        return 1;
      }
      if (talkRunner.displayTextChoices.size() != 1) {
        LOG(ERROR) << "Expected 1 choice after auto-advanced EXEC" << LOG_ENDL;
        return 1;
      }
      LOG(INFO) << "TALK auto-advance text accumulation test passed" << LOG_ENDL;
    }

    // TALK: non-auto-advance EXEC should expose a synthetic "(Continue.)" choice.
    {
      model::GameEvent talkEvent;
      talkEvent.id = "talk_continue_choice";
      talkEvent.eventType = model::GameEventType::TALK;

      model::GameEventChildExec execNode;
      execNode.eventChildType = model::GameEventChildType::EXEC;
      execNode.id = "root";
      execNode.paragraphs = {"Ugh, I can't deal with this right now."};
      execNode.next = "end_node";
      execNode.autoAdvance = false;
      talkEvent.children.pushBack(execNode);

      model::GameEventChildEnd endNode;
      endNode.eventChildType = model::GameEventChildType::END;
      endNode.id = "end_node";
      talkEvent.children.pushBack(endNode);

      runner::SpecialEventRunner talkRunner({}, talkEvent, {});
      runner::SpecialEventRunnerInterface talkIface(talkRunner);
      talkIface.startEvent();

      if (talkRunner.displayText != "Ugh, I can't deal with this right now.") {
        LOG(ERROR) << "Expected EXEC display text, got: '" << talkRunner.displayText << "'"
                   << LOG_ENDL;
        return 1;
      }
      if (talkRunner.displayTextChoices.size() != 1 ||
          talkRunner.displayTextChoices[0].text != "(Continue.)" ||
          talkRunner.displayTextChoices[0].next != "end_node") {
        LOG(ERROR) << "Expected synthetic (Continue.) choice to end_node" << LOG_ENDL;
        return 1;
      }
      talkIface.selectChoice(0);
      if (!talkRunner.isAtEndNode() || !talkRunner.displayText.empty()) {
        LOG(ERROR) << "Continue choice should land on END and clear TALK display text"
                   << LOG_ENDL;
        return 1;
      }
      LOG(INFO) << "TALK synthetic Continue choice test passed" << LOG_ENDL;
    }

    // Chosen choices are tracked and reported as previously chosen on revisit.
    {
      model::GameEvent talkEvent;
      talkEvent.id = "talk_dim_choice";
      talkEvent.eventType = model::GameEventType::TALK;

      model::GameEventChildChoice choiceNode;
      choiceNode.eventChildType = model::GameEventChildType::CHOICE;
      choiceNode.id = "root";
      choiceNode.text = "Pick one.";
      model::Choice a;
      a.text = "Option A";
      a.next = "back";
      choiceNode.choices.pushBack(a);
      model::Choice b;
      b.text = "Option B";
      b.next = "end_node";
      choiceNode.choices.pushBack(b);
      talkEvent.children.pushBack(choiceNode);

      model::GameEventChildChoice backNode;
      backNode.eventChildType = model::GameEventChildType::CHOICE;
      backNode.id = "back";
      backNode.text = "Back to root.";
      model::Choice back;
      back.text = "(Go back.)";
      back.next = "root";
      backNode.choices.pushBack(back);
      talkEvent.children.pushBack(backNode);

      model::GameEventChildEnd endNode;
      endNode.eventChildType = model::GameEventChildType::END;
      endNode.id = "end_node";
      talkEvent.children.pushBack(endNode);

      runner::SpecialEventRunner talkRunner({}, talkEvent, {});
      runner::SpecialEventRunnerInterface talkIface(talkRunner);
      talkIface.startEvent();
      if (talkRunner.displayTextChoices.size() != 2 ||
          talkRunner.wasChoiceChosen(talkRunner.displayTextChoices[0].choiceKey)) {
        LOG(ERROR) << "Expected two fresh choices at root" << LOG_ENDL;
        return 1;
      }
      const auto firstKey = talkRunner.displayTextChoices[0].choiceKey;
      talkIface.selectChoice(0); // Option A -> back
      talkIface.selectChoice(0); // Go back -> root
      if (!talkRunner.wasChoiceChosen(firstKey)) {
        LOG(ERROR) << "Option A should be marked chosen after selecting it" << LOG_ENDL;
        return 1;
      }
      if (talkRunner.displayTextChoices.size() != 2 ||
          !talkRunner.wasChoiceChosen(talkRunner.displayTextChoices[0].choiceKey) ||
          talkRunner.wasChoiceChosen(talkRunner.displayTextChoices[1].choiceKey)) {
        LOG(ERROR) << "On revisit, only Option A should be previously chosen" << LOG_ENDL;
        return 1;
      }
      LOG(INFO) << "TALK previously-chosen choice tracking test passed" << LOG_ENDL;
    }

    // Persisted dialogue storage keeps vars.*/once.* and drops tmp.*.
    {
      bmin::Map<bmin::String, bmin::String> storage;
      storage.insert(bmin::String("vars.exposition.alinea.realmKnown"),
                     bmin::String("true"));
      storage.insert(bmin::String("once.vars.hasSpokenToalinea_claire"),
                     bmin::String("true"));
      storage.insert(bmin::String("once.tmp.askedPriestess"), bmin::String("true"));
      storage.insert(bmin::String("tmp.calledLark"), bmin::String("true"));
      storage.insert(bmin::String("tmp.otherScratch"), bmin::String("1"));

      runner::clearTmpStorageKeys(storage);

      if (!storage.contains("vars.exposition.alinea.realmKnown") ||
          !storage.contains("once.vars.hasSpokenToalinea_claire") ||
          !storage.contains("once.tmp.askedPriestess")) {
        LOG(ERROR) << "clearTmpStorageKeys should keep vars.* and once.*" << LOG_ENDL;
        return 1;
      }
      if (storage.contains("tmp.calledLark") || storage.contains("tmp.otherScratch")) {
        LOG(ERROR) << "clearTmpStorageKeys should erase tmp.* keys" << LOG_ENDL;
        return 1;
      }
      if (storage.size() != 3) {
        LOG(ERROR) << "clearTmpStorageKeys left unexpected key count: " << storage.size()
                   << LOG_ENDL;
        return 1;
      }

      // Second conversation should hydrate non-tmp keys from prior storage.
      model::GameEvent talkEvent;
      talkEvent.id = "talk_persist_hydrate";
      talkEvent.eventType = model::GameEventType::TALK;

      model::GameEventChildChoice choiceNode;
      choiceNode.eventChildType = model::GameEventChildType::CHOICE;
      choiceNode.id = "root";
      choiceNode.text = "Hello again.";
      model::Choice realmKnown;
      realmKnown.text = "About the realm.";
      realmKnown.conditionStr = "IS(vars.exposition.alinea.realmKnown)";
      realmKnown.next = "end_node";
      choiceNode.choices.pushBack(realmKnown);
      model::Choice calledLark;
      calledLark.text = "About the lark.";
      calledLark.conditionStr = "IS(tmp.calledLark)";
      calledLark.next = "end_node";
      choiceNode.choices.pushBack(calledLark);
      talkEvent.children.pushBack(choiceNode);

      model::GameEventChildEnd endNode;
      endNode.eventChildType = model::GameEventChildType::END;
      endNode.id = "end_node";
      talkEvent.children.pushBack(endNode);

      runner::SpecialEventRunner talkRunner(storage, talkEvent, {});
      runner::SpecialEventRunnerInterface talkIface(talkRunner);
      talkIface.startEvent();
      if (talkRunner.displayTextChoices.size() != 1 ||
          talkRunner.displayTextChoices[0].text != "About the realm.") {
        LOG(ERROR) << "Hydrated storage should show realmKnown choice only, not tmp"
                   << LOG_ENDL;
        return 1;
      }
      LOG(INFO) << "Dialogue storage persist / tmp clear test passed" << LOG_ENDL;
    }

    LOG(INFO) << "TestSpecialEventRunner completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error in test: " << e.what() << LOG_ENDL;
    return 1;
  }
}
