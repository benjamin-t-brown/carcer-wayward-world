#pragma once

namespace state {

class State;
class StateManager;

class UiManager {
public:
  void update(int dt, State& state, StateManager& stateManager);
};

} // namespace state
