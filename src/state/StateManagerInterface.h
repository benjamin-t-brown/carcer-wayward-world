#pragma once

#include "state/StateManager.h"

namespace state {

class StateManagerInterface {
private:
  static state::StateManager* stateManager;

protected:
  static state::StateManager* getStateManager(bool throwIfNotSet = false);
  bool hasStateManager() const { return stateManager != nullptr; }

public:
  static void setStateManager(state::StateManager* _stateManager);
};

} // namespace state