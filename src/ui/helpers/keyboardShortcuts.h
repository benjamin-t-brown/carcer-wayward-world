#pragma once

#include "state/WorldActions.h"
#include <optional>
#include <string_view>

namespace ui {

std::optional<state::WorldActionType>
getWorldActionFromKeyboardShortcut(std::string_view key);

struct MoveDelta {
  int dx = 0;
  int dy = 0;
};

std::optional<MoveDelta> getMoveDeltaForKey(std::string_view key);

bool isCancelActionKey(std::string_view key);

bool isConfirmActionKey(std::string_view key);

} // namespace ui
