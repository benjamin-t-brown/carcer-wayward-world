#include "bmin/UniquePtr.h"
#include "state/StateManager.h"
#include "state/AbstractAction.h"
#include "state/WorldUpdater.h"

namespace state {

StateManager::StateManager() {}

state::State& StateManager::getState() { return state; }

ActionData& StateManager::getActionData() { return actionData; }

ActionBus& StateManager::getActionBus() { return actionBus; }

const ActionBus& StateManager::getActionBus() const { return actionBus; }

void StateManager::enqueueAction(ActionData& actions, AbstractAction* action, int ms) {
  actions.sequentialActionsNext.pushBack(
      bmin::makeUnique<AsyncAction>(AsyncAction{bmin::UniquePtr<AbstractAction>(action), model::TimerStruct(ms)}));
}

void StateManager::insertAction(ActionData& actions, AbstractAction* action, int ms) {
  actions.insertActions.pushBack(
      bmin::makeUnique<AsyncAction>(AsyncAction{bmin::UniquePtr<AbstractAction>(action), model::TimerStruct(ms)}));
}

void StateManager::addParallelAction(ActionData& actions,
                                     AbstractAction* action,
                                     int ms) {
  actions.parallelActions.pushBack(bmin::makeUnique<AsyncAction>(
      AsyncAction{bmin::UniquePtr<AbstractAction>(action), model::TimerStruct(ms)}));
}

void StateManager::moveSequentialActions(ActionData& actions) {
  actions.sequentialActions.splice(actions.sequentialActions.end(),
                                   actions.sequentialActionsNext);
}

void StateManager::update(int dt) {
  moveSequentialActions(actionData);
  while (!actionData.sequentialActions.empty()) {
    auto& delayedActionPtr = actionData.sequentialActions.front();
    AsyncAction& delayedAction = *delayedActionPtr;
    if (delayedAction.action.get() != nullptr) {
      AbstractAction& executedAction = *delayedAction.action;
      executedAction.execute(&state);
      actionBus.notify(executedAction, state);
      delayedAction.action.reset();
    }

    model::timerStructUpdate(delayedAction.timer, dt);
    if (model::timerStructIsComplete(delayedAction.timer)) {
      bool shouldLoop = delayedAction.timer.duration == 0;
      actionData.sequentialActions.erase(actionData.sequentialActions.begin());
      if (shouldLoop) {
        moveSequentialActions(actionData);
        continue;
      } else {
        break;
      }
    } else {
      break;
    }
  }
  for (unsigned int i = 0; i < actionData.parallelActions.size(); i++) {
    auto& delayedActionPtr = actionData.parallelActions[i];
    AsyncAction& delayedAction = *delayedActionPtr;
    model::timerStructUpdate(delayedAction.timer, dt);
    if (model::timerStructIsComplete(delayedAction.timer)) {
      if (delayedAction.action.get() != nullptr) {
        AbstractAction& executedAction = *delayedAction.action;
        executedAction.execute(&state);
        actionBus.notify(executedAction, state);
      }
      actionData.parallelActions.erase(static_cast<size_t>(i));
      i--;
    }
  }
  worldUpdate(state, dt);
  uiManager.update(dt, state, *this);
}

} // namespace state
