#pragma once

namespace state {

struct State;

class UiManager {
public:
  void update(int dt, State& state);
};

} // namespace state
