#pragma once

#include "state/WorldActions.h"

namespace sdl2w {
class Window;
}

namespace state {
class StateManager;
}

namespace ui {

void setHeldMoveActive(state::StateManager& stateManager, bool isActive);
void cancelCurrentWorldActionMode(state::StateManager& stateManager);
void activateWorldAction(state::StateManager& stateManager,
                         state::WorldActionType worldActionType,
                         sdl2w::Window* window = nullptr);

} // namespace ui
