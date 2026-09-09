// Characterization test for the one-shot sound request queue.
//
// Documents current behavior that later phases (Phase 6: separate world
// updates from platform output) preserve:
//   - PlaySound de-duplicates identical pending sound names;
//   - worldUpdate drains (clears) the queue every tick even with no window,
//     so a missing/disabled audio device does not retain an ever-growing queue.
#include "actions/general/PlaySound.hpp"
#include "state/WorldUpdater.h"
#include "state/State.hpp"
#include "state/StateManager.h"
#include <iostream>

namespace {

bool expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
  }
  return condition;
}

} // namespace

int main() {
  bool ok = true;

  // PlaySound de-duplicates identical pending names.
  {
    state::StateManager stateManager;
    auto& state = stateManager.getState();

    state::actions::PlaySound("step").execute(&state);
    state::actions::PlaySound("step").execute(&state);
    state::actions::PlaySound("clang").execute(&state);

    ok = expect(state.soundsToPlay.size() == 2,
                "duplicate sound name is not queued twice") && ok;
    ok = expect(state.soundsToPlay.contains("step"), "first sound queued") && ok;
    ok = expect(state.soundsToPlay.contains("clang"), "second sound queued") && ok;
  }

  // Empty sound names are ignored.
  {
    state::StateManager stateManager;
    auto& state = stateManager.getState();
    state::actions::PlaySound("").execute(&state);
    ok = expect(state.soundsToPlay.empty(), "empty sound name is ignored") && ok;
  }

  // worldUpdate drains the queue even when no window is present.
  {
    state::StateManager stateManager;
    auto& state = stateManager.getState();
    state::actions::PlaySound("step").execute(&state);
    state::actions::PlaySound("clang").execute(&state);
    ok = expect(state.soundsToPlay.size() == 2, "sounds pending before update") && ok;

    state::worldUpdate(nullptr, stateManager, 16);
    ok = expect(state.soundsToPlay.empty(),
                "headless worldUpdate drains the sound queue") && ok;

    // A second headless drain over repeated requests does not accumulate.
    state::actions::PlaySound("step").execute(&state);
    state::worldUpdate(nullptr, stateManager, 16);
    state::actions::PlaySound("step").execute(&state);
    state::worldUpdate(nullptr, stateManager, 16);
    ok = expect(state.soundsToPlay.empty(),
                "queue does not grow without an audio device") && ok;
  }

  return ok ? 0 : 1;
}
