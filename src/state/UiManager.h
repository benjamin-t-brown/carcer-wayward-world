#pragma once

namespace state {

struct State;
class StateManager;

class UiManager {
public:
  void update(int dt, State& state, StateManager& stateManager);
};

} // namespace state
