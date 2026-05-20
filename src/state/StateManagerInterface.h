#pragma once

#include "state/StateManager.h"

namespace state {

class StateManagerInterface {
private:
  static state::StateManager* stateManager;

protected:
  static state::StateManager* getStateManager(bool throwIfNotSet = false);

public:
  static void setStateManager(state::StateManager* _stateManager);
};

} // namespace state