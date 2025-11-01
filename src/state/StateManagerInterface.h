#pragma once

#include "state/StateManager.h"

namespace state {

class StateManagerInterface {
private:
  static state::StateManager* stateManager;

protected:
  static state::StateManager* getStateManager();

public:
  static void setStateManager(state::StateManager* _stateManager);
};

} // namespace state