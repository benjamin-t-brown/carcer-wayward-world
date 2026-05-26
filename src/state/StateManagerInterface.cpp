#include "StateManagerInterface.h"
#include "state/StateManager.h"
#include <stdexcept>

namespace state {

state::StateManager* StateManagerInterface::stateManager = nullptr;

void StateManagerInterface::setStateManager(state::StateManager* _stateManager) {
  stateManager = _stateManager;
}

state::StateManager* StateManagerInterface::getStateManager(bool throwIfNotSet) {
  if (stateManager == nullptr) {
    if (throwIfNotSet) {
      throw std::runtime_error("StateManager not set for StateManagerInterface.");
    }
    return nullptr;
  }
  return stateManager;
}

} // namespace state