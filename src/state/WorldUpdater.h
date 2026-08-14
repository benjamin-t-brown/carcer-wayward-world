#pragma once

namespace sdl2w {
class Window;
}

namespace state {

class StateManager;

void worldUpdate(sdl2w::Window* window, StateManager& stateManager, int dt);
void worldProcessPendingTriggers(sdl2w::Window* window, StateManager& stateManager);
} // namespace state
