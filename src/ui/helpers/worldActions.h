#pragma once

#include "state/State.h"
#include "state/WorldActions.h"

namespace state {
class StateManager;
}

namespace ui {

std::optional<state::WorldActionType>
getWorldActionFromKeyboardShortcut(std::string_view key);
void setHeldMoveActive(state::StateManager& stateManager, bool isActive);
void cancelCurrentWorldActionMode(state::StateManager& stateManager);
void activateWorldAction(state::StateManager& stateManager,
                         state::WorldActionType worldActionType);

} // namespace ui
