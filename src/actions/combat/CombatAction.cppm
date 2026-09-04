module;
#include <utility>
#include <cstddef>

export module carcer.actions.combat:CombatAction;
export import carcer.state;
import sdl2w;
#include "macros.h"

export {

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

} // export
