#pragma once

#include "state/AbstractAction.h"
#include "state/StateManagerInterface.h"

namespace state {

namespace actions {

// Combat actions enqueue follow-up work through StateManager while executing.
class CombatAction : public AbstractAction, public StateManagerInterface {
protected:
  void insertCombatAction(AbstractAction* action, int ms = 0) {
    auto* stateManager = getStateManager();
    if (stateManager == nullptr || action == nullptr) {
      return;
    }
    stateManager->insertAction(stateManager->getActionData(), action, ms);
  }

  void enqueueCombatAction(AbstractAction* action, int ms = 0) {
    auto* stateManager = getStateManager();
    if (stateManager == nullptr || action == nullptr) {
      return;
    }
    stateManager->enqueueAction(stateManager->getActionData(), action, ms);
  }
};

} // namespace actions

} // namespace state
