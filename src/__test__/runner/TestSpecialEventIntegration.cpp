#include "db/loaders/LoadSpecialEvents.h"
#include "lib/sdl2w/Logger.h"
#include "model/SpecialEvents.h"
#include "runner/SpecialEventRunner.h"
#include <unordered_map>

#define TEST_NAME "TestSpecialEventIntegration"

int main(int argc, char** argv) {
  LOG(INFO) << "Starting " << TEST_NAME << LOG_ENDL;

  std::unordered_map<std::string, model::GameEvent> specialEvents;
  try {
    // files are relative to executable in src dir
    db::loadSpecialEvents("__test__/runner/data/TestEvent.json", specialEvents);
  } catch (const std::exception& e) {
    LOG(ERROR) << "Error loading test event: " << e.what() << LOG_ENDL;
    return 1;
  }

  model::GameEvent& testEvent = specialEvents["TestEvent"];

  // -1 is advance next node, other numbers are choice indexes
  std::vector<std::pair<int, std::string>> inputSteps = {
      // clang-format off
      {-1, "This should display once."},
      {0, "You chose choice 1."},
      {1, "You selected choice 2."},
      {1, "You selected choice 3."},
      {2, "You selected choice 4.  This should wait to continue."},
      {-1, "You have waited for choice 4."},
      {3, "This should display by default."}, // the Restart choice
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
      std::string expectedDisplayText = p.second;
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
