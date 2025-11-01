#include "StateManagerInterface.h"

namespace state {

state::StateManager* StateManagerInterface::stateManager = nullptr;

void StateManagerInterface::setStateManager(state::StateManager* _stateManager) {
  stateManager = _stateManager;
}

state::StateManager* StateManagerInterface::getStateManager() {
  if (stateManager == nullptr) {
    throw std::runtime_error("StateManager not set for StateManagerInterface.");
  }
  return stateManager;
}

} // namespace state