#pragma once

#include "model/instances/World.h"
#include "state/WorldActions.h"
#include <optional>
#include <string_view>

namespace ui {

std::optional<state::WorldActionType>
getWorldActionFromKeyboardShortcut(std::string_view key, model::TurnMode turnMode);

struct MoveDelta {
  int dx = 0;
  int dy = 0;
};

std::optional<MoveDelta> getMoveDeltaForKey(std::string_view key);

bool isCancelActionKey(std::string_view key);

bool isConfirmActionKey(std::string_view key);

bool isCombatWaitKey(std::string_view key);

/** Keys "1"-"6" → party index 0-5. */
std::optional<int> getPartyMemberIndexFromKey(std::string_view key);

/** Keys "a"-"z" / "A"-"Z" → pick-up list index 0-25. */
std::optional<int> getPickUpItemIndexFromKey(std::string_view key);

} // namespace ui
