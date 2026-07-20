#pragma once

#include "state/WorldActions.h"

namespace state {
class StateManager;
}

namespace ui {

void setHeldMoveActive(state::StateManager& stateManager, bool isActive);
void cancelCurrentWorldActionMode(state::StateManager& stateManager);
void activateWorldAction(state::StateManager& stateManager,
                         state::WorldActionType worldActionType);

} // namespace ui
