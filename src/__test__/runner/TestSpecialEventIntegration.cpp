#include "db/loaders/LoadSpecialEvents.h"
#include "lib/bmin/Map.h"
#include "sdl2w/Logger.h"
#include "model/templates/SpecialEvents.h"
#include "runner/SpecialEventRunner.h"
#include "bmin/String.h"
#include "bmin/DynArray.h"
#include "bmin/Map.h"

#include <vector>

#define TEST_NAME "TestSpecialEventIntegration"

int main(int argc, char** argv) {
  LOG(INFO) << "Starting " << TEST_NAME << LOG_ENDL;

  bmin::Map<bmin::String, model::GameEvent> specialEvents;
  try {
    // files are relative to executable in src dir
    db::loadSpecialEvents("__test__/runner/data/TestEvent.json", specialEvents);
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error loading test event: " << e.what() << LOG_ENDL;
    return 1;
  }

  model::GameEvent& testEvent = specialEvents["TestEvent"];

  // -1 is advance next node, other numbers are choice indexes.
  // MODAL events wait on non-empty EXEC text (autoAdvance is ignored).
  bmin::DynArray<std::pair<int, bmin::String>> inputSteps = {
      // clang-format off
      {-1, "This should display once."},
      {-1, "This should display once."}, // continue → choice menu
      {0, "You chose choice 1."},
      {-1, "You chose choice 1."}, // continue → choice menu
      {1, "You selected choice 2."},
      {-1, "You selected choice 2."},
      {1, "You selected choice 3."},
      {-1, "You selected choice 3."},
      {2, "You selected choice 4.  This should wait to continue."},
      {-1, "You have waited for choice 4."},
      {-1, "You have waited for choice 4."}, // continue → choice menu
      {3, "This should display by default."}, // Restart
      {-1, "This should display by default."}, // continue → choice menu
      {4, "End."}
      //
      // clang-format on
  };

  try {
    runner::SpecialEventRunner runner({}, testEvent, specialEvents);
    runner::SpecialEventRunnerInterface runnerInterface(runner);

    for (int stepIndex = 0; static_cast<size_t>(stepIndex) < inputSteps.size();
         stepIndex++) {
      const auto& p = inputSteps[stepIndex];
      int choiceIndex = p.first;
      if (stepIndex == 0) {
        LOG(INFO) << "Starting event at node: " << runner.currentNodeId << LOG_ENDL;
        runnerInterface.startEvent();
      } else if (choiceIndex == -1) {
        auto state = runnerInterface.getState();
        if (state == runner::SpecialEventRunnerInterfaceState::WAITING_TO_CONTINUE) {
          LOG(INFO) << "Continuing event." << LOG_ENDL;
          runnerInterface.continueEvent();
        } else {
          LOG(ERROR) << "Error, test indicated to continue event, but state is not "
                        "WAITING_TO_CONTINUE.  State: "
                     << runner::SpecialEventRunnerInterface::stateToString(state)
                     << LOG_ENDL;
          return 1;
        }
      } else {
        auto state = runnerInterface.getState();
        if (state == runner::SpecialEventRunnerInterfaceState::WAITING_TO_SELECT_CHOICE) {
          LOG(INFO) << "Selecting choice: " << choiceIndex << " - "
                    << runner.displayTextChoices[choiceIndex].text << LOG_ENDL;
          runnerInterface.selectChoice(choiceIndex);
        } else {
          LOG(ERROR) << "Error, test indicated to select choice, but state is not "
                        "WAITING_TO_SELECT_CHOICE.  State: "
                     << runner::SpecialEventRunnerInterface::stateToString(state)
                     << LOG_ENDL;
          return 1;
        }
      }

      LOG(INFO) << " Display text: " << runner.displayText << LOG_ENDL;
      int choiceIndexCtr = 0;
      for (const auto& choice : runner.displayTextChoices) {
        LOG(INFO) << "  " << choiceIndexCtr << ". " << choice.text << LOG_ENDL;
        choiceIndexCtr++;
      }

      for (const auto& error : runner.errors) {
        LOG(ERROR) << " Error: " << error.nodeId << " - " << error.message << LOG_ENDL;
      }
      if (runner.errors.size() > 0) {
        LOG(INFO) << "Errors: " << runner.errors.size() << LOG_ENDL;
        return 1;
      }
      bmin::String expectedDisplayText = p.second;
      if (runner.displayText != expectedDisplayText) {
        LOG(ERROR) << "Display text mismatch (found/expected): '" << runner.displayText
                   << "' != '" << expectedDisplayText << "'" << LOG_ENDL;
        return 1;
      }
    }

    LOG(INFO) << "Storage: \n" << runner.storageToString() << LOG_ENDL;

    LOG(INFO) << TEST_NAME << " completed successfully" << LOG_ENDL;
    return 0;
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error in test: " << e.what() << LOG_ENDL;
    return 1;
  }
}
