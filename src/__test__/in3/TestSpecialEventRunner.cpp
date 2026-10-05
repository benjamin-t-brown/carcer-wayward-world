#include "bmin/Map.h"
#include "bmin/String.h"
#include "db/Database.h"
#include "in3/EventRunnerHelpers.h"
#include "in3/QuestProgress.h"
#include "in3/SpecialEventRunner.h"
#include "model/instances/CharacterPlayer.h"
#include "model/instances/Player.h"
#include "model/templates/Quests.hpp"
#include "model/templates/SpecialEvents.hpp"
#include "sdl2w/Logger.h"


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
    in3::SpecialEventRunner runner(initialStorage, testEvent, gameEvents);

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
    in3::ConditionResult condResult = runner.evalCondition("IS(test_key)");
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

      in3::SpecialEventRunner talkRunner({}, talkEvent, {});
      in3::SpecialEventRunnerInterface talkIface(talkRunner);
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

    // MODAL: auto-advanced EXEC text must carry into the following CHOICE (same as TALK).
    {
      model::GameEvent modalEvent;
      modalEvent.id = "modal_auto_advance";
      modalEvent.eventType = model::GameEventType::MODAL;

      model::GameEventChildExec execNode;
      execNode.eventChildType = model::GameEventChildType::EXEC;
      execNode.id = "root";
      execNode.paragraphs = {"The chest creaks open."};
      execNode.next = "choice_node";
      execNode.autoAdvance = true;
      modalEvent.children.pushBack(execNode);

      model::GameEventChildChoice choiceNode;
      choiceNode.eventChildType = model::GameEventChildType::CHOICE;
      choiceNode.id = "choice_node";
      choiceNode.text = "";
      model::Choice takeIt;
      takeIt.text = "Take the coin.";
      takeIt.next = "end_node";
      choiceNode.choices.pushBack(takeIt);
      model::Choice leaveIt;
      leaveIt.text = "Leave it.";
      leaveIt.next = "end_node";
      choiceNode.choices.pushBack(leaveIt);
      modalEvent.children.pushBack(choiceNode);

      model::GameEventChildEnd endNode;
      endNode.eventChildType = model::GameEventChildType::END;
      endNode.id = "end_node";
      modalEvent.children.pushBack(endNode);

      in3::SpecialEventRunner modalRunner({}, modalEvent, {});
      in3::SpecialEventRunnerInterface modalIface(modalRunner);
      modalIface.startEvent();

      if (modalRunner.displayText != "The chest creaks open.") {
        LOG(ERROR) << "MODAL auto-advance should keep EXEC text. Got: '"
                   << modalRunner.displayText << "'" << LOG_ENDL;
        return 1;
      }
      if (modalRunner.displayTextChoices.size() != 2) {
        LOG(ERROR) << "Expected 2 choices after auto-advanced MODAL EXEC" << LOG_ENDL;
        return 1;
      }
      if (modalIface.getState() !=
          in3::SpecialEventRunnerInterfaceState::WAITING_TO_SELECT_CHOICE) {
        LOG(ERROR) << "MODAL auto-advance to CHOICE should wait for selection, got: "
                   << in3::SpecialEventRunnerInterface::stateToString(modalIface.getState())
                   << LOG_ENDL;
        return 1;
      }
      LOG(INFO) << "MODAL auto-advance text accumulation test passed" << LOG_ENDL;
    }

    // MODAL: autoAdvance into END still pauses so Okay can dismiss the EXEC text.
    {
      model::GameEvent modalEvent;
      modalEvent.id = "modal_auto_advance_end";
      modalEvent.eventType = model::GameEventType::MODAL;

      model::GameEventChildExec execNode;
      execNode.eventChildType = model::GameEventChildType::EXEC;
      execNode.id = "root";
      execNode.paragraphs = {"You pick up the coin."};
      execNode.next = "end_node";
      execNode.autoAdvance = true;
      modalEvent.children.pushBack(execNode);

      model::GameEventChildEnd endNode;
      endNode.eventChildType = model::GameEventChildType::END;
      endNode.id = "end_node";
      modalEvent.children.pushBack(endNode);

      in3::SpecialEventRunner modalRunner({}, modalEvent, {});
      in3::SpecialEventRunnerInterface modalIface(modalRunner);
      modalIface.startEvent();

      if (modalRunner.displayText != "You pick up the coin.") {
        LOG(ERROR) << "MODAL autoAdvance→END should show EXEC text, got: '"
                   << modalRunner.displayText << "'" << LOG_ENDL;
        return 1;
      }
      if (modalIface.getState() !=
          in3::SpecialEventRunnerInterfaceState::WAITING_TO_CONTINUE) {
        LOG(ERROR) << "MODAL autoAdvance→END should wait for Okay, got: "
                   << in3::SpecialEventRunnerInterface::stateToString(modalIface.getState())
                   << LOG_ENDL;
        return 1;
      }
      modalIface.continueEvent();
      if (modalRunner.displayText != "End.") {
        LOG(ERROR) << "continue after MODAL autoAdvance→END should show End., got: '"
                   << modalRunner.displayText << "'" << LOG_ENDL;
        return 1;
      }
      if (modalIface.getState() != in3::SpecialEventRunnerInterfaceState::FINISHED) {
        LOG(ERROR) << "MODAL END after Okay should be FINISHED, got: "
                   << in3::SpecialEventRunnerInterface::stateToString(modalIface.getState())
                   << LOG_ENDL;
        return 1;
      }
      LOG(INFO) << "MODAL auto-advance to END keeps Okay test passed" << LOG_ENDL;
    }

    // TALK: non-auto-advance EXEC pauses with empty choices (WAITING_TO_CONTINUE).
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

      in3::SpecialEventRunner talkRunner({}, talkEvent, {});
      in3::SpecialEventRunnerInterface talkIface(talkRunner);
      talkIface.startEvent();

      if (talkRunner.displayText != "Ugh, I can't deal with this right now.") {
        LOG(ERROR) << "Expected EXEC display text, got: '" << talkRunner.displayText << "'"
                   << LOG_ENDL;
        return 1;
      }
      if (!talkRunner.displayTextChoices.empty()) {
        LOG(ERROR) << "TALK EXEC pause should leave displayTextChoices empty" << LOG_ENDL;
        return 1;
      }
      if (talkIface.getState() !=
          in3::SpecialEventRunnerInterfaceState::WAITING_TO_CONTINUE) {
        LOG(ERROR) << "TALK EXEC pause should wait to continue, got: "
                   << in3::SpecialEventRunnerInterface::stateToString(talkIface.getState())
                   << LOG_ENDL;
        return 1;
      }
      talkIface.continueEvent();
      if (!talkRunner.isAtEndNode() || !talkRunner.displayText.empty()) {
        LOG(ERROR) << "continueEvent should land on END and clear TALK display text"
                   << LOG_ENDL;
        return 1;
      }
      if (talkIface.getState() != in3::SpecialEventRunnerInterfaceState::FINISHED ||
          !talkIface.isFinished()) {
        LOG(ERROR) << "TALK at END should report FINISHED, got: "
                   << in3::SpecialEventRunnerInterface::stateToString(talkIface.getState())
                   << LOG_ENDL;
        return 1;
      }
      LOG(INFO) << "TALK EXEC pause continueEvent test passed" << LOG_ENDL;
    }

    // Session policy: before startEvent is WAITING_TO_START; MODAL empty-next is FINISHED.
    {
      if (in3::SpecialEventRunnerInterface::stateToString(
              in3::SpecialEventRunnerInterfaceState::FINISHED) != "FINISHED") {
        LOG(ERROR) << "stateToString(FINISHED) should be FINISHED" << LOG_ENDL;
        return 1;
      }

      model::GameEvent modalEvent;
      modalEvent.id = "modal_end";
      modalEvent.eventType = model::GameEventType::MODAL;

      model::GameEventChildExec execNode;
      execNode.eventChildType = model::GameEventChildType::EXEC;
      execNode.id = "root";
      execNode.paragraphs = {"Body."};
      execNode.next = "end_node";
      execNode.autoAdvance = false;
      modalEvent.children.pushBack(execNode);

      model::GameEventChildEnd endNode;
      endNode.eventChildType = model::GameEventChildType::END;
      endNode.id = "end_node";
      modalEvent.children.pushBack(endNode);

      in3::SpecialEventRunner modalRunner({}, modalEvent, {});
      in3::SpecialEventRunnerInterface modalIface(modalRunner);
      if (modalIface.getState() != in3::SpecialEventRunnerInterfaceState::WAITING_TO_START ||
          modalIface.isFinished()) {
        LOG(ERROR) << "Interface should wait to start before startEvent, got: "
                   << in3::SpecialEventRunnerInterface::stateToString(modalIface.getState())
                   << LOG_ENDL;
        return 1;
      }

      modalIface.startEvent();
      if (modalIface.getState() != in3::SpecialEventRunnerInterfaceState::WAITING_TO_CONTINUE ||
          modalIface.isFinished()) {
        LOG(ERROR) << "MODAL with text and next should wait to continue, got: "
                   << in3::SpecialEventRunnerInterface::stateToString(modalIface.getState())
                   << LOG_ENDL;
        return 1;
      }

      modalIface.continueEvent();
      if (modalRunner.displayText != "End.") {
        LOG(ERROR) << "MODAL last advance should keep End. display, got: '"
                   << modalRunner.displayText << "'" << LOG_ENDL;
        return 1;
      }
      if (modalIface.getState() != in3::SpecialEventRunnerInterfaceState::FINISHED ||
          !modalIface.isFinished()) {
        LOG(ERROR) << "MODAL empty-next session should report FINISHED, got: "
                   << in3::SpecialEventRunnerInterface::stateToString(modalIface.getState())
                   << LOG_ENDL;
        return 1;
      }
      LOG(INFO) << "Session policy WAITING_TO_START / FINISHED test passed" << LOG_ENDL;
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

      in3::SpecialEventRunner talkRunner({}, talkEvent, {});
      in3::SpecialEventRunnerInterface talkIface(talkRunner);
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

      in3::clearTmpStorageKeys(storage);

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

      in3::SpecialEventRunner talkRunner(storage, talkEvent, {});
      in3::SpecialEventRunnerInterface talkIface(talkRunner);
      talkIface.startEvent();
      if (talkRunner.displayTextChoices.size() != 1 ||
          talkRunner.displayTextChoices[0].text != "About the realm.") {
        LOG(ERROR) << "Hydrated storage should show realmKnown choice only, not tmp"
                   << LOG_ENDL;
        return 1;
      }
      LOG(INFO) << "Dialogue storage persist / tmp clear test passed" << LOG_ENDL;
    }

    // Several quest execs on one node should raise a single journal notice.
    {
      model::QuestTemplate rock;
      rock.id = "alineaBartoRock";
      model::QuestStep getRock;
      getRock.id = "get-rock";
      model::QuestStep throwRock;
      throwRock.id = "throw-rock";
      rock.steps.pushBack(getRock);
      rock.steps.pushBack(throwRock);
      bmin::Map<bmin::String, model::QuestTemplate> quests;
      quests[rock.id] = rock;
      in3::setQuestTemplates(&quests);

      model::GameEvent talkEvent;
      talkEvent.id = "talk_journal_once";
      talkEvent.eventType = model::GameEventType::TALK;

      model::GameEventChildExec execNode;
      execNode.eventChildType = model::GameEventChildType::EXEC;
      execNode.id = "root";
      execNode.paragraphs = {"Several quest updates."};
      execNode.execStr = "START_QUEST(alineaBartoRock)\n"
                         "SET_QUEST_STEP_EQ(alineaBartoRock, throw-rock)\n"
                         "COMPLETE_QUEST_STEP(alineaBartoRock, get-rock)\n"
                         "COMPLETE_QUEST(alineaBartoRock)";
      execNode.next = "end_node";
      execNode.autoAdvance = false;
      talkEvent.children.pushBack(execNode);

      model::GameEventChildEnd endNode;
      endNode.eventChildType = model::GameEventChildType::END;
      endNode.id = "end_node";
      talkEvent.children.pushBack(endNode);

      in3::SpecialEventRunner talkRunner({}, talkEvent, {});
      in3::SpecialEventRunnerInterface talkIface(talkRunner);
      talkIface.startEvent();
      if (!talkRunner.pendingJournalNotice) {
        LOG(ERROR) << "Multiple quest calls on one EXEC should set pendingJournalNotice"
                   << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      auto notices = talkRunner.consumePendingNotices();
      if (!notices.journalUpdated || !notices.receivedItemNames.empty() ||
          notices.modifiedCoins != 0 || notices.modifiedExperience != 0) {
        LOG(ERROR) << "consumePendingNotices should return the journal flag"
                   << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      if (talkRunner.pendingJournalNotice ||
          !talkRunner.pendingReceivedItemNames.empty() ||
          talkRunner.pendingCoinDelta != 0 ||
          talkRunner.pendingExperienceDelta != 0) {
        LOG(ERROR) << "consumePendingNotices should clear pending journal fields"
                   << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      in3::setQuestTemplates(nullptr);
      LOG(INFO) << "Same-node quest journal notice is a single pending flag" << LOG_ENDL;
    }

    {
      db::Database database;
      database.load();

      model::Player player;
      player.party.pushBack(
          model::CharacterPlayer(database.getCharacterTemplate("testPartyMember1")));

      model::GameEvent talkEvent;
      talkEvent.id = "talk_item_once";
      talkEvent.eventType = model::GameEventType::TALK;

      model::GameEventChildExec execNode;
      execNode.eventChildType = model::GameEventChildType::EXEC;
      execNode.id = "root";
      execNode.paragraphs = {"Here, take these."};
      execNode.execStr = "ADD_ITEM_TO_PLAYER(BeerPappysLager)\n"
                         "ADD_ITEM_TO_PLAYER(BeerPappysLager)\n"
                         "ADD_ITEM_TO_PLAYER(AlineaCorrespondence1)";
      execNode.next = "end_node";
      execNode.autoAdvance = false;
      talkEvent.children.pushBack(execNode);

      model::GameEventChildEnd endNode;
      endNode.eventChildType = model::GameEventChildType::END;
      endNode.id = "end_node";
      talkEvent.children.pushBack(endNode);

      in3::SpecialEventRunner talkRunner({}, talkEvent, {});
      talkRunner.setExecContext(&player, &database);
      in3::SpecialEventRunnerInterface talkIface(talkRunner);
      talkIface.startEvent();
      if (talkRunner.pendingReceivedItemNames.size() != 2 ||
          talkRunner.pendingReceivedItemNames[0] != "BeerPappysLager" ||
          talkRunner.pendingReceivedItemNames[1] != "AlineaCorrespondence1") {
        LOG(ERROR) << "Same-node ADD_ITEM_TO_PLAYER should queue one notice per distinct item"
                   << LOG_ENDL;
        return 1;
      }
      auto notices = talkRunner.consumePendingNotices();
      if (notices.journalUpdated || notices.receivedItemNames.size() != 2 ||
          notices.receivedItemNames[0] != "BeerPappysLager" ||
          notices.receivedItemNames[1] != "AlineaCorrespondence1" ||
          notices.modifiedCoins != 0 || notices.modifiedExperience != 0) {
        LOG(ERROR) << "consumePendingNotices should return distinct item names"
                   << LOG_ENDL;
        return 1;
      }
      if (talkRunner.pendingJournalNotice ||
          !talkRunner.pendingReceivedItemNames.empty() ||
          talkRunner.pendingCoinDelta != 0 ||
          talkRunner.pendingExperienceDelta != 0) {
        LOG(ERROR) << "consumePendingNotices should clear pending item fields"
                   << LOG_ENDL;
        return 1;
      }
      // Destructible beer goes to party inventory; indestructable correspondence stays
      // in vars.items.
      auto lagerInStorage =
          in3::getStorage(talkRunner.storage, "vars.items.BeerPappysLager");
      if (lagerInStorage) {
        LOG(ERROR) << "destructible ADD_ITEM_TO_PLAYER should not use vars.items"
                   << LOG_ENDL;
        return 1;
      }
      // BeerPappysLager is not stackable: two grants are two inventory rows.
      int beerQty = 0;
      for (const auto& invItem : player.party[0].inventory) {
        if (invItem.itemName == "BeerPappysLager") {
          beerQty += invItem.quantity;
        }
      }
      if (beerQty != 2) {
        LOG(ERROR) << "destructible ADD_ITEM_TO_PLAYER should land in party inventory"
                   << LOG_ENDL;
        return 1;
      }
      auto letterCount =
          in3::getStorage(talkRunner.storage, "vars.items.AlineaCorrespondence1");
      if (!letterCount || *letterCount != "1") {
        LOG(ERROR) << "indestructable ADD_ITEM_TO_PLAYER should use vars.items"
                   << LOG_ENDL;
        return 1;
      }
      LOG(INFO) << "Same-node item receive notices are one per distinct item" << LOG_ENDL;
    }

    {
      model::GameEvent talkEvent;
      talkEvent.id = "talk_coins";
      talkEvent.eventType = model::GameEventType::TALK;

      model::GameEventChildExec execNode;
      execNode.eventChildType = model::GameEventChildType::EXEC;
      execNode.id = "root";
      execNode.paragraphs = {"Here's some coin."};
      execNode.execStr = "MODIFY_COINS(10)\nMODIFY_COINS(-3)\nMODIFY_COINS(-100)";
      execNode.next = "end_node";
      execNode.autoAdvance = false;
      talkEvent.children.pushBack(execNode);

      model::GameEventChildEnd endNode;
      endNode.eventChildType = model::GameEventChildType::END;
      endNode.id = "end_node";
      talkEvent.children.pushBack(endNode);

      in3::SpecialEventRunner talkRunner({}, talkEvent, {});
      in3::SpecialEventRunnerInterface talkIface(talkRunner);
      talkIface.startEvent();
      if (talkRunner.pendingCoinDelta != 0) {
        LOG(ERROR) << "MODIFY_COINS clamp should net to 0 pending coins" << LOG_ENDL;
        return 1;
      }
      auto coins = in3::getStorage(talkRunner.storage, in3::kPlayerCoinsStorageKey);
      if (!coins || *coins != "0") {
        LOG(ERROR) << "MODIFY_COINS should leave vars.player.coins at 0 after clamp"
                   << LOG_ENDL;
        return 1;
      }

      model::GameEvent gainEvent;
      gainEvent.id = "talk_coins_gain";
      gainEvent.eventType = model::GameEventType::TALK;
      model::GameEventChildExec gainExec;
      gainExec.eventChildType = model::GameEventChildType::EXEC;
      gainExec.id = "root";
      gainExec.paragraphs = {"Payment."};
      gainExec.execStr = "MODIFY_COINS(4)\nMODIFY_COINS(6)";
      gainExec.next = "end_node";
      gainExec.autoAdvance = false;
      gainEvent.children.pushBack(gainExec);
      gainEvent.children.pushBack(endNode);

      in3::SpecialEventRunner gainRunner({}, gainEvent, {});
      in3::SpecialEventRunnerInterface gainIface(gainRunner);
      gainIface.startEvent();
      if (gainRunner.pendingCoinDelta != 10) {
        LOG(ERROR) << "MODIFY_COINS should net pending coins across calls" << LOG_ENDL;
        return 1;
      }
      auto notices = gainRunner.consumePendingNotices();
      if (notices.modifiedCoins != 10 || gainRunner.pendingCoinDelta != 0) {
        LOG(ERROR) << "consumePendingNotices should return and clear modified coins"
                   << LOG_ENDL;
        return 1;
      }
      auto gained = in3::getStorage(gainRunner.storage, in3::kPlayerCoinsStorageKey);
      if (!gained || *gained != "10") {
        LOG(ERROR) << "MODIFY_COINS should set vars.player.coins" << LOG_ENDL;
        return 1;
      }
      LOG(INFO) << "MODIFY_COINS nets applied deltas and clamps at 0" << LOG_ENDL;
    }

    {
      model::QuestTemplate rewardQuest;
      rewardQuest.id = "rewardQuest";
      rewardQuest.rewards.coins = 12;
      rewardQuest.rewards.experience = 8;
      model::QuestRewardItem lager;
      lager.name = "BeerPappysLager";
      lager.amount = 15;
      rewardQuest.rewards.items.pushBack(lager);
      bmin::Map<bmin::String, model::QuestTemplate> quests;
      quests[rewardQuest.id] = rewardQuest;
      in3::setQuestTemplates(&quests);

      model::GameEvent talkEvent;
      talkEvent.id = "talk_quest_rewards";
      talkEvent.eventType = model::GameEventType::TALK;

      model::GameEventChildExec execNode;
      execNode.eventChildType = model::GameEventChildType::EXEC;
      execNode.id = "root";
      execNode.paragraphs = {"Quest complete."};
      execNode.execStr = "COMPLETE_QUEST(rewardQuest)";
      execNode.next = "end_node";
      execNode.autoAdvance = false;
      talkEvent.children.pushBack(execNode);

      model::GameEventChildEnd endNode;
      endNode.eventChildType = model::GameEventChildType::END;
      endNode.id = "end_node";
      talkEvent.children.pushBack(endNode);

      in3::SpecialEventRunner talkRunner({}, talkEvent, {});
      in3::SpecialEventRunnerInterface talkIface(talkRunner);
      talkIface.startEvent();
      if (!talkRunner.pendingJournalNotice ||
          talkRunner.pendingReceivedItemNames.size() != 1 ||
          talkRunner.pendingReceivedItemNames[0] != "BeerPappysLager" ||
          talkRunner.pendingCoinDelta != 12 ||
          talkRunner.pendingExperienceDelta != 8) {
        LOG(ERROR) << "COMPLETE_QUEST should queue journal, item, coin, and experience notices"
                   << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      auto notices = talkRunner.consumePendingNotices();
      if (!notices.journalUpdated || notices.receivedItemNames.size() != 1 ||
          notices.modifiedCoins != 12 || notices.modifiedExperience != 8) {
        LOG(ERROR) << "consumePendingNotices should return quest reward notices"
                   << LOG_ENDL;
        in3::setQuestTemplates(nullptr);
        return 1;
      }
      in3::setQuestTemplates(nullptr);
      LOG(INFO) << "COMPLETE_QUEST grants rewards into pending notices" << LOG_ENDL;
    }

    LOG(INFO) << "TestSpecialEventRunner completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error in test: " << e.what() << LOG_ENDL;
    return 1;
  }
}
