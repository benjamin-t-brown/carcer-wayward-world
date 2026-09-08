#pragma once

namespace state {

struct State;

// Advances time-based UI state each tick (currently floating-notification
// timers). Named for what it does; it does not own or manage UI elements.
class UiStateUpdater {
public:
  void update(int dt, State& state);
};

} // namespace state
