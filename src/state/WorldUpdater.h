#pragma once

namespace state {

struct State;

// Simulation seam invoked from StateManager::update after action processing.
// When cameraMode is Follow, snaps camera to cameraFollowCharacterId (or the
// current party member avatar if that id is empty) when viewW/viewH > 0.
void worldUpdate(State& state, int dt);

} // namespace state
