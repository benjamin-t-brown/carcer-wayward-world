#pragma once

#include "state/AbstractAction.h"
#include "state/StateManagerInterface.h"

namespace state {

namespace actions {

// Combat actions enqueue follow-up work through StateManager while executing.
class CombatAction : public AbstractAction {
protected:
  // void insertAction(AbstractAction* action, int ms = 0) {
  //   auto* stateManager = getStateManager();
  //   if (stateManager == nullptr) {
  //     return;
  //   }
  //   stateManager->insertAction(stateManager->getActionData(), action, ms);
  // }

  // void enqueueAction(AbstractAction* action, int ms = 0) {
  //   auto* stateManager = getStateManager();
  //   if (stateManager == nullptr) {
  //     return;
  //   }
  //   stateManager->enqueueAction(stateManager->getActionData(), action, ms);
  // }
};

} // namespace actions

} // namespace state
