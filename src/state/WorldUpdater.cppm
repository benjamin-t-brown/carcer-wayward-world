module;
#include <cstddef>
#include <cstdint>
#include <utility>

export module carcer.state.WorldUpdater;
export import carcer.state;
import sdl2w;
#include "macros.h"

export {

namespace state {

void worldUpdate(sdl2w::Window* window, StateManager& stateManager, int dt);
void worldProcessPendingTriggers(sdl2w::Window* window, StateManager& stateManager);

} // namespace state

} // export
