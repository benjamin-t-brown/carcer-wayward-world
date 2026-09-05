module;
#include <cstddef>

export module carcer.world_updater;
export import carcer.state;
import sdl2w;

export namespace state {

void worldUpdate(sdl2w::Window* window, StateManager& stateManager, int dt);

inline void worldUpdate(StateManager& stateManager, int dt) {
  worldUpdate(nullptr, stateManager, dt);
}

void worldProcessPendingTriggers(sdl2w::Window* window,
                                 StateManager& stateManager);

} // namespace state
