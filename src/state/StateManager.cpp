#include "state/StateManager.h"
#include "model/UtilityTypes.h"
#include "state/AbstractAction.h"

namespace state {

StateManager::StateManager() {
  // Constructor implementation
}

state::State& StateManager::getState() { return state; }

void StateManager::enqueueAction(state::State& state,
                                 state::AbstractAction* action,
                                 int ms) {
  action->setDatabase(getDatabase());
  auto actionPtr = new state::AsyncAction{std::unique_ptr<state::AbstractAction>(action),
                                          model::TimerStruct(ms)};
  sequentialActionsNext.push_back(std::unique_ptr<state::AsyncAction>(actionPtr));
  // if (actionPtr->action) {
  //   LOG(INFO) << "Enqueued action: " << actionPtr->action->getName() << " for
  //   "
  //             << ms << "ms" << LOG_ENDL;
  // }
}

void StateManager::addParallelAction(State& state,
                                     std::unique_ptr<state::AbstractAction> action,
                                     int ms) {
  action->setDatabase(getDatabase());
  parallelActions.push_back(std::unique_ptr<state::AsyncAction>(
      new state::AsyncAction{std::move(action), model::TimerStruct(ms)}));
}

void StateManager::moveSequentialActions(State& state) {
  sequentialActions.insert(sequentialActions.end(),
                           std::make_move_iterator(sequentialActionsNext.begin()),
                           std::make_move_iterator(sequentialActionsNext.end()));
  sequentialActionsNext.clear();
}

void StateManager::update(int dt) {
  moveSequentialActions(state);
  while (!sequentialActions.empty()) {
    auto& delayedActionPtr = sequentialActions.front();
    state::AsyncAction& delayedAction = *delayedActionPtr;
    if (delayedAction.action.get() != nullptr) {
      delayedAction.action->execute(&state);
      delayedAction.action = nullptr;
    }

    delayedAction.timer.update(dt);
    if (delayedAction.timer.isComplete()) {
      bool shouldLoop = delayedAction.timer.duration == 0;
      sequentialActions.erase(sequentialActions.begin());
      if (shouldLoop) {
        moveSequentialActions(state);
        continue;
      } else {
        break;
      }
    } else {
      break;
    }
  }
  for (unsigned int i = 0; i < parallelActions.size(); i++) {
    auto& delayedActionPtr = parallelActions[i];
    state::AsyncAction& delayedAction = *delayedActionPtr;
    delayedAction.timer.update(dt);
    if (delayedAction.timer.isComplete()) {
      if (delayedAction.action != nullptr) {
        delayedAction.action->execute(&state);
      }
      parallelActions.erase(parallelActions.begin() + i);
      i--;
    }
  }
}

} // namespace state
